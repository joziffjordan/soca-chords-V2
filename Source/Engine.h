// Soca Chords - turns the library into notes, chord names and MIDI files.
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include "Library.h"
#include <array>
#include <algorithm>

namespace soca
{
struct Chord
{
    int degree = 0;          // semitones above the tonic
    int intervals[6] {};     // semitones above the chord root
    int numIntervals = 0;
    float beats = 4.0f;
    bool minor = false;
    char roman[16] {};       // as written in the library, without @beats
    char suffix[12] {};      // m, 7, maj7 ...
};

struct Progression
{
    std::vector<Chord> chords;
    float totalBeats = 0.0f;
    bool valid = true;
};

struct Note
{
    float start = 0, length = 0;   // in beats
    int pitch = 60, velocity = 100;
};

inline bool parseChord (const juce::String& tokenIn, Mode mode, Chord& c)
{
    auto token = tokenIn.trim();
    c = Chord();

    if (token.contains ("@"))
    {
        c.beats = token.fromFirstOccurrenceOf ("@", false, false).getFloatValue();
        token = token.upToFirstOccurrenceOf ("@", false, false);
        if (c.beats <= 0.0f) return false;
    }
    token.copyToUTF8 (c.roman, sizeof (c.roman));

    int shift = 0, pos = 0;
    while (pos < token.length() && (token[pos] == 'b' || token[pos] == '#'))
        shift += (token[pos++] == 'b') ? -1 : 1;

    juce::String numeral;
    while (pos < token.length() && juce::String ("ivIV").containsChar (token[pos]))
        numeral += token[pos++];
    auto quality = token.substring (pos);

    static const char* names[] = { "I", "II", "III", "IV", "V", "VI", "VII" };
    static const int majorScale[] = { 0, 2, 4, 5, 7, 9, 11 };
    static const int minorScale[] = { 0, 2, 3, 5, 7, 8, 10 };
    int index = -1;
    for (int i = 0; i < 7; ++i)
        if (numeral.toUpperCase() == names[i]) index = i;
    if (index < 0) return false;

    c.minor = (numeral == numeral.toLowerCase());
    c.degree = (((mode == Maj ? majorScale : minorScale)[index] + shift) % 12 + 12) % 12;

    const int third = c.minor ? 3 : 4;
    auto set = [&c] (std::initializer_list<int> list)
    {
        c.numIntervals = 0;
        for (int v : list) c.intervals[c.numIntervals++] = v;
    };
    juce::String suffix;

    if (quality.isEmpty())        { set ({ 0, third, 7 });                              suffix = c.minor ? "m" : ""; }
    else if (quality == "7")      { set ({ 0, third, 7, 10 });                          suffix = c.minor ? "m7" : "7"; }
    else if (quality == "maj7")   { set ({ 0, third, 7, 11 });                          suffix = c.minor ? "m(maj7)" : "maj7"; }
    else if (quality == "9")      { set ({ 0, third, 7, 10, 14 });                      suffix = c.minor ? "m9" : "9"; }
    else if (quality == "sus4")   { set ({ 0, 5, 7 });                                  suffix = "sus4"; }
    else if (quality == "sus2")   { set ({ 0, 2, 7 });                                  suffix = "sus2"; }
    else if (quality == "add9")   { set ({ 0, third, 7, 14 });                          suffix = c.minor ? "m(add9)" : "add9"; }
    else if (quality == "6")      { set ({ 0, third, 7, 9 });                           suffix = c.minor ? "m6" : "6"; }
    else if (quality == "dim")    { set ({ 0, 3, 6 });                                  suffix = "dim"; }
    else return false;

    suffix.copyToUTF8 (c.suffix, sizeof (c.suffix));
    return true;
}

inline Progression parseProgression (const ProgressionDef& def)
{
    Progression p;
    juce::StringArray tokens;
    tokens.addTokens (def.chords, " ", "");
    tokens.removeEmptyStrings();
    for (auto& t : tokens)
    {
        Chord c;
        if (! parseChord (t, def.mode, c)) { p.valid = false; continue; }
        p.chords.push_back (c);
        p.totalBeats += c.beats;
    }
    if (p.chords.empty()) p.valid = false;
    return p;
}

/** The whole library, parsed once. Read-only afterwards, so safe on any thread. */
inline const std::vector<Progression>& library()
{
    static const std::vector<Progression> parsed = []
    {
        std::vector<Progression> v;
        for (int i = 0; i < kLibrarySize; ++i)
            v.push_back (parseProgression (kLibrary[i]));
        return v;
    }();
    return parsed;
}

// ---------------------------------------------------------------- naming
inline bool keyUsesSharps (int key, Mode mode)
{
    const int relativeMajor = (key + (mode == Min ? 3 : 0)) % 12;
    return relativeMajor == 7 || relativeMajor == 2 || relativeMajor == 9
        || relativeMajor == 4 || relativeMajor == 11 || relativeMajor == 6;
}

inline juce::String noteName (int pitchClass, bool sharps)
{
    static const char* flat[]  = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
    static const char* sharp[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return (sharps ? sharp : flat)[((pitchClass % 12) + 12) % 12];
}

inline juce::String keyName (int key)
{
    static const char* names[] = { "C", "C# / Db", "D", "Eb", "E", "F", "F# / Gb", "G", "Ab", "A", "Bb", "B" };
    return names[((key % 12) + 12) % 12];
}

inline juce::String chordName (const Chord& c, int key, Mode mode)
{
    return noteName (key + c.degree, keyUsesSharps (key, mode)) + c.suffix;
}

// ---------------------------------------------------------------- voicing
enum Voicing { Guitar = 0, Keys = 1, Horns = 2 };
static const char* const kVoicingNames[] = { "Guitar", "Keys", "Horns" };
constexpr int kMaxChords = 8;       // chords per progression that can carry their own inversion
constexpr int kMaxVoices = 6;

/** How many inversions a chord has (3 for a triad, 4 for a seventh chord ...). */
inline int numInversions (const Chord& c) { return juce::jlimit (1, 4, c.numIntervals); }

/** Works out the MIDI notes for one chord. Returns how many notes were written to `pitches`
    (at most kMaxVoices), lowest first.

    Keys   - close position; each inversion moves the lowest note up an octave.
    Horns  - the same shape a little higher, with the second voice from the top dropped an
             octave (a "drop 2" section voicing).
    Guitar - five strings, barre-chord style (1 5 1 3 5); the inversion picks the bass note. */
inline int voiceChord (const Chord& c, int key, int octave, int voicing, int inversion, int* pitches)
{
    const int n = juce::jmin (c.numIntervals, kMaxVoices);
    if (n <= 0) return 0;
    const int shift = 12 * juce::jlimit (-2, 2, octave);
    const int rootPc = (key + c.degree) % 12;
    const int inv = juce::jlimit (0, numInversions (c) - 1, inversion);
    auto place = [] (int pitchClass, int lowest) { return lowest + ((pitchClass - lowest) % 12 + 12) % 12; };

    if (voicing == Guitar)
    {
        static const int triadOrder[] = { 0, 2, 3, 4, 5 };
        static const int seventhOrder[] = { 0, 2, 3, 5, 6 };
        const int* order = n == 3 ? triadOrder : seventhOrder;
        int previous = 39 + shift;                                   // bass lands between E2 and Eb3
        for (int i = 0; i < 5; ++i)
        {
            const int tone = c.intervals[(inv + order[i]) % n] % 12;
            previous = place (rootPc + tone, previous + 1);
            pitches[i] = previous;
        }
        return 5;
    }

    const int base = (voicing == Horns ? 57 : 52) + shift;
    const int root = place (rootPc, base);
    for (int i = 0; i < n; ++i) pitches[i] = root + c.intervals[i];
    for (int i = 0; i < inv; ++i) pitches[i] += 12;
    std::sort (pitches, pitches + n);
    if (pitches[0] > base + 9)
        for (int i = 0; i < n; ++i) pitches[i] -= 12;

    if (voicing == Horns && n >= 3)
    {
        pitches[n - 2] -= 12;
        std::sort (pitches, pitches + n);
    }
    return n;
}

/** "1st inv", "Root" ... for display. */
inline juce::String inversionName (int inversion)
{
    static const char* names[] = { "Root", "1st", "2nd", "3rd" };
    return names[juce::jlimit (0, 3, inversion)];
}

// ---------------------------------------------------------------- one-key chords
/** The chord each key of the octave triggers: white keys play the chords of the key,
    black keys play borrowed chords. Index 0 = C ... 11 = B. */
struct TriggerSet { Chord chords[12]; };

inline const TriggerSet& triggerChords (Mode mode)
{
    static const std::array<TriggerSet, 2> sets = []
    {
        //                       C     C#     D        D#      E      F     F#    G    G#     A     A#      B
        const char* major[] = { "I",  "bII", "ii",    "bIII", "iii", "IV", "iv", "V", "bVI", "vi", "bVII", "viidim" };
        const char* minor[] = { "i",  "bII", "iidim", "IV",   "III", "iv", "V",  "v", "V7",  "VI", "i7",   "VII" };
        std::array<TriggerSet, 2> built {};
        for (int i = 0; i < 12; ++i)
        {
            parseChord (major[i], Maj, built[0].chords[(size_t) i]);
            parseChord (minor[i], Min, built[1].chords[(size_t) i]);
        }
        return built;
    }();
    return sets[mode == Min ? 1 : 0];
}

/** The numeral shown on each key, e.g. "vii°". */
inline juce::String triggerLabel (Mode mode, int pitchClass)
{
    return juce::String (triggerChords (mode).chords[((pitchClass % 12) + 12) % 12].roman).replace ("dim", juce::String::fromUTF8 ("\xc2\xb0"));
}

// ---------------------------------------------------------------- notes
/** Fills `out` with every note of one pass through the progression.
    `inversions` holds one value per chord (may be null for all root position).
    Does not allocate if `out` already has enough capacity. */
inline void buildNotes (int progressionIndex, int key, int rhythmIndex, int octave, bool bass,
                        int voicing, const signed char* inversions, std::vector<Note>& out)
{
    out.clear();
    const auto& lib = library();
    if (progressionIndex < 0 || progressionIndex >= (int) lib.size()) return;
    const auto& prog = lib[(size_t) progressionIndex];
    const auto& rhythm = kRhythms[juce::jlimit (0, kNumRhythms - 1, rhythmIndex)];

    const float gap = 0.03f;
    const float strum = voicing == Guitar ? 0.022f : 0.0f;      // beats between strings
    float t = 0.0f;
    int chordIndex = 0;

    for (const auto& c : prog.chords)
    {
        const float end = t + c.beats;
        const int root = (key + c.degree) % 12;
        const int inversion = (inversions != nullptr && chordIndex < kMaxChords) ? inversions[chordIndex] : 0;

        int pitches[kMaxVoices];
        const int count = voiceChord (c, key, octave, voicing, inversion, pitches);

        auto addChord = [&] (float start, float length, int velocity)
        {
            for (int i = 0; i < count; ++i)
            {
                const float delay = juce::jmin (strum * (float) i, length * 0.4f);
                out.push_back ({ start + delay, length - delay, pitches[i], velocity });
            }
        };

        if (rhythm.numHits == 0)
        {
            addChord (t, c.beats - gap, 96);
        }
        else
        {
            const int firstStep = juce::roundToInt (t * 4.0f);
            const int lastStep  = juce::roundToInt (end * 4.0f);
            for (int s = firstStep; s < lastStep; ++s)
                for (int h = 0; h < rhythm.numHits; ++h)
                    if (rhythm.hits[h].step == s % 16)
                    {
                        const float start = (float) s / 4.0f;
                        const float length = juce::jmin ((float) rhythm.hits[h].len / 4.0f * 0.9f, end - start - gap);
                        addChord (start, length, rhythm.hits[h].vel);
                    }
        }

        if (bass)
            out.push_back ({ t, c.beats - gap, 36 + root + 12 * juce::jlimit (-1, 1, octave), 100 });

        t = end;
        ++chordIndex;
    }
}

// ---------------------------------------------------------------- MIDI file
inline juce::String describe (int progressionIndex, int key)
{
    const auto& def = kLibrary[progressionIndex];
    return juce::String (def.name) + " - " + noteName (key, keyUsesSharps (key, def.mode))
         + (def.mode == Min ? " minor" : " major");
}

/** Writes the progression to a .mid file in the temp folder and returns it. */
inline juce::File writeMidiFile (int progressionIndex, int key, int rhythmIndex, int octave, bool bass,
                                 int voicing, const signed char* inversions, double bpm)
{
    std::vector<Note> notes;
    buildNotes (progressionIndex, key, rhythmIndex, octave, bass, voicing, inversions, notes);

    const int ppq = 480;
    juce::MidiMessageSequence seq;
    seq.addEvent (juce::MidiMessage::textMetaEvent (3, "Soca Chords"), 0.0);
    seq.addEvent (juce::MidiMessage::timeSignatureMetaEvent (4, 4), 0.0);
    seq.addEvent (juce::MidiMessage::tempoMetaEvent (juce::roundToInt (60000000.0 / juce::jmax (20.0, bpm))), 0.0);

    for (const auto& n : notes)
    {
        seq.addEvent (juce::MidiMessage::noteOn (1, n.pitch, (juce::uint8) n.velocity), n.start * ppq);
        seq.addEvent (juce::MidiMessage::noteOff (1, n.pitch), (n.start + n.length) * ppq);
    }
    const double endTicks = library()[(size_t) progressionIndex].totalBeats * ppq;
    seq.addEvent (juce::MidiMessage::endOfTrack(), endTicks);
    seq.sort();
    seq.updateMatchedPairs();

    juce::MidiFile file;
    file.setTicksPerQuarterNote (ppq);
    file.addTrack (seq);

    auto folder = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("SocaChords");
    folder.createDirectory();
    auto name = juce::File::createLegalFileName ("Soca Chords - " + describe (progressionIndex, key) + " - "
                                                 + kVoicingNames[juce::jlimit (0, 2, voicing)] + " - "
                                                 + juce::String (juce::roundToInt (bpm)) + "bpm.mid");
    auto target = folder.getChildFile (name);
    target.deleteFile();
    juce::FileOutputStream stream (target);
    if (stream.openedOk())
    {
        file.writeTo (stream);
        stream.flush();
    }
    return target;
}
} // namespace soca
