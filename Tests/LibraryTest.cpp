// Checks every progression parses and produces sane notes, then writes sample MIDI files.
#include "../Source/Engine.h"
#include <iostream>
#include <set>

int main (int argc, char** argv)
{
    int failures = 0;
    std::set<juce::String> names, shapes;
    const auto& lib = soca::library();

    for (int i = 0; i < soca::kLibrarySize; ++i)
    {
        const auto& def = soca::kLibrary[i];
        const auto& p = lib[(size_t) i];
        auto fail = [&] (const juce::String& why) { ++failures; std::cout << "FAIL " << def.name << ": " << why << "\n"; };

        if (! p.valid) { fail ("did not parse: " + juce::String (def.chords)); continue; }
        if (! names.insert (def.name).second) fail ("duplicate name");
        if (! shapes.insert (juce::String ((int) def.mode) + def.chords).second) fail ("duplicate progression");
        if (def.styles == 0) fail ("no style tag");
        if (std::fmod (p.totalBeats, 4.0f) != 0.0f) fail ("not a whole number of bars");

        for (int r = 0; r < soca::kNumRhythms; ++r)
            for (int key = 0; key < 12; ++key)
            {
              for (int voicing = 0; voicing < 3; ++voicing)
              {
                std::vector<soca::Note> notes;
                signed char inv[soca::kMaxChords];
                for (int c = 0; c < soca::kMaxChords; ++c) inv[c] = (signed char) ((c + key + r) % 4);
                soca::buildNotes (i, key, r, 0, true, voicing, inv, notes);
                if (notes.empty()) fail ("no notes");
                for (auto& n : notes)
                {
                    if (n.pitch < 24 || n.pitch > 96) fail ("pitch out of range");
                    if (n.length <= 0.0f) fail ("zero-length note in rhythm " + juce::String (soca::kRhythms[r].name));
                    if (n.start < 0.0f || n.start + n.length > p.totalBeats) fail ("note outside loop");
                }
              }
            }

        juce::StringArray chordNames;
        for (auto& c : p.chords) chordNames.add (soca::chordName (c, 0, def.mode));
        std::cout << juce::String (def.name).paddedRight (' ', 26) << (def.mode == soca::Min ? "Cm  " : "C   ")
                  << juce::String (def.chords).paddedRight (' ', 34) << chordNames.joinIntoString (" ") << "\n";
    }

    if (argc > 1)
    {
        auto out = juce::File (argv[1]);
        out.createDirectory();
        soca::writeMidiFile (0, 0, 0, 0, true, soca::Keys, nullptr, 160.0).copyFileTo (out.getChildFile ("anthem_sustained.mid"));
        soca::writeMidiFile (0, 7, 1, 0, false, soca::Guitar, nullptr, 160.0).copyFileTo (out.getChildFile ("anthem_G_stabs.mid"));
        soca::writeMidiFile (60, 9, 3, 0, true, soca::Horns, nullptr, 110.0).copyFileTo (out.getChildFile ("minor_A_332.mid"));
    }

    // voicing shapes for C major, each inversion
    for (int voicing = 0; voicing < 3; ++voicing)
        for (int inv = 0; inv < 3; ++inv)
        {
            int pitches[soca::kMaxVoices];
            const int n = soca::voiceChord (lib[0].chords[0], 0, 0, voicing, inv, pitches);
            std::cout << soca::kVoicingNames[voicing] << " " << soca::inversionName (inv) << ":";
            for (int k = 0; k < n; ++k) std::cout << " " << juce::MidiMessage::getMidiNoteName (pitches[k], true, true, 4);
            std::cout << "\n";
        }
    for (int m = 0; m < 2; ++m)
    {
        std::cout << (m == 0 ? "major keys:" : "minor keys:");
        for (int k = 0; k < 12; ++k) std::cout << " " << soca::triggerLabel ((soca::Mode) m, k);
        std::cout << "\n";
    }

    std::cout << soca::kLibrarySize << " progressions, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
