# Soca Chords

A chord-progression browser for modern soca. Cycle through 123 progressions, hear them, then drag the MIDI straight onto a track.

- **Formats:** VST3 (Mac + Windows), AU (Mac), standalone app
- **Styles:** Power Soca, Groovy Soca, Bouyon, Dennery Segment, Afro-Soca, Jab Soca, Chutney Soca, Zess / Steam
- **Controls:** key, major/minor filter, 9 rhythm patterns, per-chord inversions, Guitar / Keys / Horns voicing, one-key chord triggering, optional bass note, built-in preview sound

## Getting the plugin (no coding needed)

1. Create a free account at github.com and make a new empty repository.
2. Upload everything in this folder to it: `CMakeLists.txt`, `README.md`, `Source`, `Tests`, `Assets` and the hidden `.github` folder.
3. Open the repository's **Actions** tab. A build called "Build Soca Chords" runs by itself and takes about 10 minutes.
4. Open the finished run and download **SocaChords-macOS** and/or **SocaChords-Windows** from the Artifacts section.

## Installing

**Mac**
- Copy `Soca Chords.vst3` to `~/Library/Audio/Plug-Ins/VST3/`
- Copy `Soca Chords.component` to `~/Library/Audio/Plug-Ins/Components/`
- The build is not signed by Apple, so macOS will block it the first time. Open Terminal and run:
  `xattr -cr ~/Library/Audio/Plug-Ins/VST3/"Soca Chords.vst3" ~/Library/Audio/Plug-Ins/Components/"Soca Chords.component"`

**Windows**
- Copy the `Soca Chords.vst3` folder to `C:\Program Files\Common Files\VST3\`

Then in Ableton: Settings > Plug-Ins > turn on VST3 (and Audio Units on Mac) > Rescan. Soca Chords appears under Plug-Ins as an instrument.

## Using it

1. Drop Soca Chords on a MIDI track.
2. Pick a style, scale and key. Use the arrows (or the arrow keys) to cycle; **Random** jumps anywhere in the filtered list.
3. Press **Play** to hear it. Tempo follows the DAW, and when the DAW is playing the chords lock to its bars.
4. **Voicing** changes how every chord is played: Guitar (five strummed strings), Keys (close position) or Horns (open section voicing).
5. **Inversions** are per chord: click a chord tile to select it, then choose Root, 1st or 2nd (and 3rd for seventh chords). Each tile shows its own inversion.
6. Choose a rhythm pattern and whether you want a bass note.
7. Drag from the **DRAG MIDI** tile onto any MIDI track. The clip contains exactly what you heard.

### One-key chords

Play any key on your MIDI keyboard (or click the keys in the plugin) and it plays a whole chord in the chosen key and voicing:

- White keys play the chords of the key: C plays I, D plays ii, E plays iii, F plays IV, G plays V, A plays vi, B plays vii.
- Black keys play borrowed chords (bII, bIII, iv, bVI, bVII in major).
- Higher and lower octaves on your keyboard move the chord up and down.
- Keys light up in the colour of the matching chord tile when that chord is in the current progression.

Record the MIDI track's output on a second track (see below) to capture what you play.

**Hear it through your own instrument:** the chords also leave the plugin as live MIDI. In Ableton, on a second MIDI track set *MIDI From* to the Soca Chords track, pick "Soca Chords" in the chooser below it, set Monitor to *In*, and turn the plugin's Volume down.

## Adding your own progressions

Open `Source/Library.h`. Each progression is one line, e.g.

    { "My Riddim", Min, Bouyon | Dennery, "i VII VI@2 VII@2" },

The rules are at the top of that file. Push the change to GitHub and a new build is made automatically.

## Notes

- The library is a curated set of the progressions that dominate soca from 2016 on, named by feel rather than by song. It is not a transcription of every release.
- Building locally instead: install CMake and a compiler (Xcode or Visual Studio), then `cmake -B build` and `cmake --build build --config Release`.
- The interface uses the Pacifico and Poppins fonts, both under the SIL Open Font License (licence files are in `Assets/`).
- Built with JUCE 8 (free for personal use under its AGPL/Starter terms).
