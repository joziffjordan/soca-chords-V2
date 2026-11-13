#include "PluginProcessor.h"
#include "PluginEditor.h"

// ------------------------------------------------------------------ preview synth
namespace
{
struct PreviewSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** A simple bright, plucky poly synth: two detuned saws through a low-pass filter. */
struct PreviewVoice : juce::SynthesiserVoice
{
    explicit PreviewVoice (const std::atomic<int>& v) : voicing (v) {}
    const std::atomic<int>& voicing;

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<PreviewSound*> (s) != nullptr; }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int) override
    {
        const double hz = juce::MidiMessage::getMidiNoteInHertz (midiNote);
        const double sr = getSampleRate();
        inc1 = hz * 0.997 / sr;
        inc2 = hz * 1.003 / sr;
        const int style = voicing.load();
        isBass = midiNote < 48 && style != soca::Guitar;
        level = velocity * (isBass ? 0.22f : style == soca::Horns ? 0.11f : 0.13f);
        double cutoff = isBass ? 500.0 : juce::jlimit (900.0, 6000.0, hz * 6.0);
        juce::ADSR::Parameters shape { 0.004f, 0.35f, 0.55f, 0.12f };            // keys
        if (style == soca::Guitar && ! isBass) { shape = { 0.002f, 0.55f, 0.12f, 0.18f }; cutoff = juce::jlimit (1200.0, 5000.0, hz * 7.0); }
        if (style == soca::Horns && ! isBass)  { shape = { 0.022f, 0.18f, 0.82f, 0.09f }; cutoff = juce::jlimit (1500.0, 7500.0, hz * 10.0); }
        coeff = (float) (1.0 - std::exp (-juce::MathConstants<double>::twoPi * cutoff / sr));
        adsr.setSampleRate (sr);
        adsr.setParameters (shape);
        adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff) adsr.noteOff();
        else { adsr.reset(); clearCurrentNote(); }
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    using juce::SynthesiserVoice::renderNextBlock;
    void renderNextBlock (juce::AudioBuffer<float>& buffer, int start, int num) override
    {
        if (! adsr.isActive()) return;
        for (int i = 0; i < num; ++i)
        {
            phase1 += inc1; if (phase1 >= 1.0) phase1 -= 1.0;
            phase2 += inc2; if (phase2 >= 1.0) phase2 -= 1.0;
            const float raw = (float) (phase1 + phase2 - 1.0);
            lp1 += coeff * (raw - lp1);
            lp2 += coeff * (lp1 - lp2);
            const float s = lp2 * level * adsr.getNextSample();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.addSample (ch, start + i, s);
        }
        if (! adsr.isActive()) clearCurrentNote();
    }

    double phase1 = 0.0, phase2 = 0.5, inc1 = 0.0, inc2 = 0.0;
    float lp1 = 0.0f, lp2 = 0.0f, coeff = 0.5f, level = 0.1f;
    bool isBass = false;
    juce::ADSR adsr;
};
} // namespace

// ------------------------------------------------------------------ processor
SocaChordsProcessor::SocaChordsProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    for (int i = 0; i < 32; ++i) synth.addVoice (new PreviewVoice (voicing));
    synth.addSound (new PreviewSound());
    notes.reserve (4096);
    outgoing.ensureSize (8192);
    soca::library();   // parse once, up front
}

void SocaChordsProcessor::prepareToPlay (double sampleRate, int)
{
    rate = sampleRate;
    synth.setCurrentPlaybackSampleRate (sampleRate);
    std::fill (std::begin (activeCount), std::end (activeCount), 0);
    for (auto& h : held) h.count = 0;
    heldKeys = 0;
    triggerKeyDown = -1;
    builtVersion = 0;
    expected = -1.0;
}

bool SocaChordsProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void SocaChordsProcessor::flushNotes (juce::MidiBuffer& out, int samplePos)
{
    for (int p = 0; p < 128; ++p)
    {
        if (activeCount[p] > 0)
            out.addEvent (juce::MidiMessage::noteOff (1, p), samplePos);
        activeCount[p] = 0;
    }
}

void SocaChordsProcessor::copyInversions (int progressionIndex, signed char* dest) const
{
    const int p = juce::jlimit (0, soca::kLibrarySize - 1, progressionIndex);
    for (int i = 0; i < soca::kMaxChords; ++i) dest[i] = inversions[p][i].load();
}

void SocaChordsProcessor::triggerOn (int note, int velocity, int samplePos, int numSamples)
{
    if (note < 0 || note > 127) return;
    if (held[note].count > 0) triggerOff (note, samplePos);

    const auto mode = soca::kLibrary[juce::jlimit (0, soca::kLibrarySize - 1, progression.load())].mode;
    const auto& chord = soca::triggerChords (mode).chords[note % 12];
    const int keyOctave = juce::jlimit (-2, 2, note / 12 - 5);          // middle C octave = normal register
    const int style = voicing.load();
    auto& h = held[note];
    h.count = soca::voiceChord (chord, key.load(), keyOctave, style, 0, h.pitches);
    if (bass.load())
        h.pitches[h.count++] = 36 + (key.load() + chord.degree) % 12 + 12 * juce::jlimit (-1, 1, keyOctave);

    const int strum = style == soca::Guitar ? (int) (rate * 0.012) : 0;
    for (int i = 0; i < h.count; ++i)
        outgoing.addEvent (juce::MidiMessage::noteOn (1, juce::jlimit (0, 127, h.pitches[i]), (juce::uint8) juce::jlimit (1, 127, velocity)),
                           juce::jmin (numSamples - 1, samplePos + strum * i));
    ++heldKeys;
    triggerKeyDown = note % 12;
}

void SocaChordsProcessor::triggerOff (int note, int samplePos)
{
    if (note < 0 || note > 127 || held[note].count == 0) return;
    for (int i = 0; i < held[note].count; ++i)
        outgoing.addEvent (juce::MidiMessage::noteOff (1, juce::jlimit (0, 127, held[note].pitches[i])), samplePos);
    held[note].count = 0;
    if (--heldKeys <= 0) { heldKeys = 0; triggerKeyDown = -1; }
}

void SocaChordsProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    buffer.clear();
    outgoing.clear();

    // One-key chords: every incoming key plays a whole chord
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.isNoteOn())        triggerOn (m.getNoteNumber(), m.getVelocity(), meta.samplePosition, numSamples);
        else if (m.isNoteOff())  triggerOff (m.getNoteNumber(), meta.samplePosition);
        else if (m.isAllNotesOff() || m.isAllSoundOff())
            for (int n = 0; n < 128; ++n) triggerOff (n, meta.samplePosition);
    }
    if (const int on = uiTriggerOn.exchange (-1); on >= 0)   triggerOn (on, 100, 0, numSamples);
    if (const int off = uiTriggerOff.exchange (-1); off >= 0) triggerOff (off, numSamples - 1);

    // Tempo and transport from the host
    double bpm = currentBpm.load();
    bool hostPlaying = false;
    double hostPpq = 0.0;
    if (auto* ph = getPlayHead())
        if (auto info = ph->getPosition())
        {
            if (auto b = info->getBpm()) bpm = *b;
            if (auto q = info->getPpqPosition()) { hostPpq = *q; hostPlaying = info->getIsPlaying(); }
        }
    bpm = juce::jlimit (30.0, 300.0, bpm);
    currentBpm = bpm;

    // Rebuild the note list if the selection changed
    const int v = version.load();
    if (v != builtVersion)
    {
        flushNotes (outgoing, 0);
        signed char inv[soca::kMaxChords];
        copyInversions (progression.load(), inv);
        soca::buildNotes (progression.load(), key.load(), rhythm.load(), octave.load(), bass.load(), voicing.load(), inv, notes);
        const auto& lib = soca::library();
        const int idx = juce::jlimit (0, (int) lib.size() - 1, progression.load());
        loopBeats = juce::jmax (1.0, (double) lib[(size_t) idx].totalBeats);
        builtVersion = v;
        expected = -1.0;
    }

    const bool isPlaying = playing.load();
    if (! isPlaying)
    {
        if (wasPlaying) flushNotes (outgoing, 0);
        playheadBeat = -1.0f;
        expected = -1.0;
    }
    else
    {
        const double beatsPerSample = bpm / 60.0 / rate;

        // Follow the host's bar position when it is running, otherwise free-run.
        double start = position;
        if (hostPlaying)
        {
            start = std::fmod (hostPpq, loopBeats);
            if (start < 0.0) start += loopBeats;
        }
        else if (! wasPlaying || expected < 0.0)
        {
            start = 0.0;
        }

        if (expected < 0.0 || std::abs (start - expected) > 0.02)
            flushNotes (outgoing, 0);     // jumped: don't leave notes hanging
        else
            start = expected;             // continuous: carry on exactly where the last block ended

        double segStart = start;
        double remaining = numSamples * beatsPerSample;
        double sampleOffset = 0.0;
        const double lastMoment = loopBeats - 0.001;

        while (remaining > 1.0e-9)
        {
            const double segLen = juce::jmin (remaining, loopBeats - segStart);
            const double segEnd = segStart + segLen;

            auto toSample = [&] (double beat)
            {
                return juce::jlimit (0, numSamples - 1, (int) (sampleOffset + (beat - segStart) / beatsPerSample));
            };

            for (const auto& n : notes)        // note-offs first so repeated notes retrigger cleanly
            {
                const double off = juce::jmin ((double) n.start + n.length, lastMoment);
                if (off >= segStart && off < segEnd && activeCount[n.pitch] > 0)
                {
                    outgoing.addEvent (juce::MidiMessage::noteOff (1, n.pitch), toSample (off));
                    --activeCount[n.pitch];
                }
            }
            for (const auto& n : notes)
            {
                if (n.start >= segStart && n.start < segEnd)
                {
                    outgoing.addEvent (juce::MidiMessage::noteOn (1, n.pitch, (juce::uint8) n.velocity), toSample (n.start));
                    ++activeCount[n.pitch];
                }
            }

            remaining -= segLen;
            sampleOffset += segLen / beatsPerSample;
            segStart = segEnd >= loopBeats - 1.0e-9 ? 0.0 : segEnd;
        }

        position = expected = segStart;
        playheadBeat = (float) segStart;
    }
    wasPlaying = isPlaying;

    // Built-in preview sound
    if (soundOn.load())
    {
        synth.renderNextBlock (buffer, outgoing, 0, numSamples);
        buffer.applyGain (volume.load());
    }
    else
    {
        synth.allNotesOff (0, false);
    }

    midi.swapWith (outgoing);   // chords also leave as MIDI for any instrument that listens
}

juce::AudioProcessorEditor* SocaChordsProcessor::createEditor() { return new SocaChordsEditor (*this); }

void SocaChordsProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml ("SocaChords");
    xml.setAttribute ("progressionName", soca::kLibrary[juce::jlimit (0, soca::kLibrarySize - 1, progression.load())].name);
    xml.setAttribute ("key", key.load());
    xml.setAttribute ("rhythm", rhythm.load());
    xml.setAttribute ("octave", octave.load());
    xml.setAttribute ("style", styleFilter.load());
    xml.setAttribute ("mode", modeFilter.load());
    xml.setAttribute ("bass", bass.load());
    xml.setAttribute ("sound", soundOn.load());
    xml.setAttribute ("volume", (double) volume.load());
    xml.setAttribute ("voicing", voicing.load());
    for (int p = 0; p < soca::kLibrarySize; ++p)
    {
        juce::String values;
        bool any = false;
        for (int c = 0; c < soca::kMaxChords; ++c)
        {
            const int v = inversions[p][c].load();
            any = any || v != 0;
            values << v;
        }
        if (any)
        {
            auto* e = xml.createNewChildElement ("inversions");
            e->setAttribute ("progression", soca::kLibrary[p].name);
            e->setAttribute ("values", values);
        }
    }
    copyXmlToBinary (xml, destData);
}

void SocaChordsProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (! xml->hasTagName ("SocaChords")) return;
        const auto name = xml->getStringAttribute ("progressionName");
        for (int i = 0; i < soca::kLibrarySize; ++i)       // by name, so presets survive library edits
            if (name == soca::kLibrary[i].name) progression = i;
        key        = juce::jlimit (0, 11, xml->getIntAttribute ("key", 0));
        rhythm     = juce::jlimit (0, soca::kNumRhythms - 1, xml->getIntAttribute ("rhythm", 0));
        octave     = juce::jlimit (-1, 1, xml->getIntAttribute ("octave", 0));
        styleFilter = juce::jlimit (0, soca::kNumStyles, xml->getIntAttribute ("style", 0));
        modeFilter = juce::jlimit (0, 2, xml->getIntAttribute ("mode", 0));
        bass       = xml->getBoolAttribute ("bass", true);
        soundOn    = xml->getBoolAttribute ("sound", true);
        volume     = (float) juce::jlimit (0.0, 1.0, xml->getDoubleAttribute ("volume", 0.7));
        voicing    = juce::jlimit (0, 2, xml->getIntAttribute ("voicing", soca::Keys));
        for (auto& row : inversions) for (auto& v : row) v = 0;
        for (auto* e : xml->getChildWithTagNameIterator ("inversions"))
        {
            const auto progName = e->getStringAttribute ("progression");
            const auto values = e->getStringAttribute ("values");
            for (int p = 0; p < soca::kLibrarySize; ++p)
                if (progName == soca::kLibrary[p].name)
                    for (int c = 0; c < juce::jmin (soca::kMaxChords, values.length()); ++c)
                        inversions[p][c] = (signed char) juce::jlimit (0, 3, (int) (values[c] - '0'));
        }
        notesChanged();
        ++stateVersion;
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SocaChordsProcessor(); }
