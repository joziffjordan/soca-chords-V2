#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"

class SocaChordsProcessor : public juce::AudioProcessor
{
public:
    SocaChordsProcessor();
    ~SocaChordsProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Soca Chords"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.3; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- settings (written by the editor, read by the audio thread)
    std::atomic<int> progression { 0 }, key { 0 }, rhythm { 0 }, octave { 0 };
    std::atomic<int> styleFilter { 0 }, modeFilter { 0 };       // 0 = all
    std::atomic<int> voicing { soca::Keys };
    /** One inversion per chord of every progression. */
    std::atomic<signed char> inversions[soca::kLibrarySize][soca::kMaxChords] {};
    void copyInversions (int progressionIndex, signed char* dest) const;

    // ---- one-key chords: the editor's on-screen keys post notes here (60 = middle C)
    std::atomic<int> uiTriggerOn { -1 }, uiTriggerOff { -1 };
    std::atomic<int> triggerKeyDown { -1 };     // pitch class of the key currently held, for display
    std::atomic<bool> bass { true }, playing { false }, soundOn { true };
    std::atomic<float> volume { 0.7f };

    /** Call after changing anything that changes the notes. */
    void notesChanged() { ++version; }

    // ---- read by the editor
    std::atomic<float> playheadBeat { -1.0f };
    std::atomic<double> currentBpm { 120.0 };
    std::atomic<int> stateVersion { 0 };   // bumps when the host restores a preset

private:
    void flushNotes (juce::MidiBuffer& out, int samplePos);
    void triggerOn (int note, int velocity, int samplePos, int numSamples);
    void triggerOff (int note, int samplePos);

    struct HeldChord { int count = 0; int pitches[soca::kMaxVoices + 1] {}; };
    HeldChord held[128];
    int heldKeys = 0;

    juce::Synthesiser synth;
    std::vector<soca::Note> notes;
    juce::MidiBuffer outgoing;
    int activeCount[128] {};
    std::atomic<int> version { 1 };
    int builtVersion = 0;
    bool wasPlaying = false;
    double position = 0.0, expected = -1.0, loopBeats = 4.0;
    double rate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SocaChordsProcessor)
};
