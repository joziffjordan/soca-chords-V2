// Runs the audio engine without a host: checks MIDI out is balanced and the preview synth makes sound.
#include "../Source/PluginProcessor.h"
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();
#include <iostream>

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    std::unique_ptr<juce::AudioProcessor> base (createPluginFilter());
    auto* p = dynamic_cast<SocaChordsProcessor*> (base.get());
    const double sr = 48000.0; const int block = 512;
    p->setPlayConfigDetails (0, 2, sr, block);
    p->prepareToPlay (sr, block);
    int failures = 0;

    for (int prog : { 0, 23, 70, 100 })
        for (int rhythm = 0; rhythm < soca::kNumRhythms; ++rhythm)
        {
            p->progression = prog; p->rhythm = rhythm; p->notesChanged(); p->playing = true;
            int ons = 0, offs = 0, held[128] = {}; float peak = 0; bool stuck = false;
            juce::AudioBuffer<float> buf (2, block); juce::MidiBuffer midi;
            const int blocks = (int) (sr * 60.0 / 120.0 * 40.0 / block);   // 40 beats at 120 bpm: > 2 loops
            for (int b = 0; b < blocks + 20; ++b)
            {
                if (b == blocks) p->playing = false;
                midi.clear(); p->processBlock (buf, midi);
                for (const auto meta : midi)
                {
                    auto m = meta.getMessage();
                    if (m.isNoteOn())  { ++ons;  if (++held[m.getNoteNumber()] > 1) stuck = true; }
                    if (m.isNoteOff()) { ++offs; --held[m.getNoteNumber()]; }
                }
                peak = juce::jmax (peak, buf.getMagnitude (0, block));
            }
            int hanging = 0; for (int h : held) hanging += h;
            const bool ok = ons > 0 && ons == offs && hanging == 0 && ! stuck && peak > 0.01f && peak < 1.0f;
            if (! ok) ++failures;
            std::cout << (ok ? "ok   " : "FAIL ") << soca::kLibrary[prog].name << " / " << soca::kRhythms[rhythm].name
                      << "  ons " << ons << " offs " << offs << " hanging " << hanging << " overlap " << stuck << " peak " << peak << "\n";
        }
    // one-key chords: every key should play a full chord and release it cleanly, in each voicing
    for (int voicing = 0; voicing < 3; ++voicing)
    {
        p->voicing = voicing; p->playing = false; p->notesChanged();
        int ons = 0, offs = 0; float peak = 0; bool keyShown = false;
        juce::AudioBuffer<float> buf (2, block); juce::MidiBuffer midi;
        for (int key = 48; key < 84; ++key)
            for (int b = 0; b < 12; ++b)
            {
                midi.clear();
                if (b == 0) midi.addEvent (juce::MidiMessage::noteOn (1, key, (juce::uint8) 100), 10);
                if (b == 8) midi.addEvent (juce::MidiMessage::noteOff (1, key), 10);
                p->processBlock (buf, midi);
                if (b == 4) keyShown = keyShown || p->triggerKeyDown.load() == key % 12;
                for (const auto meta : midi) { if (meta.getMessage().isNoteOn()) ++ons; if (meta.getMessage().isNoteOff()) ++offs; }
                peak = juce::jmax (peak, buf.getMagnitude (0, block));
            }
        const bool ok = ons >= 36 * 4 && ons == offs && peak > 0.01f && peak < 1.0f && keyShown && p->triggerKeyDown.load() == -1;
        if (! ok) ++failures;
        std::cout << (ok ? "ok   " : "FAIL ") << "one-key chords / " << soca::kVoicingNames[voicing] << "  ons " << ons << " offs " << offs << " peak " << peak << "\n";
    }

    // state round trip keeps voicing and per-chord inversions
    {
        p->voicing = soca::Horns; p->progression = 5; p->inversions[5][2] = 2;
        juce::MemoryBlock state; p->getStateInformation (state);
        p->voicing = soca::Keys; p->inversions[5][2] = 0; p->progression = 0;
        p->setStateInformation (state.getData(), (int) state.getSize());
        const bool ok = p->voicing.load() == soca::Horns && p->inversions[5][2].load() == 2 && p->progression.load() == 5;
        if (! ok) ++failures;
        std::cout << (ok ? "ok   " : "FAIL ") << "state round trip\n";
    }
    std::cout << failures << " failures\n";
    return failures;
}
