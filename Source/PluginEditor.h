#pragma once
#include "PluginProcessor.h"

/** Chunky 3D controls: every control is drawn from a light, mid and dark shade of one colour. */
struct SunsetLookAndFeel : juce::LookAndFeel_V4
{
    SunsetLookAndFeel();
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool, bool) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos, float minPos,
                           float maxPos, juce::Slider::SliderStyle, juce::Slider&) override;
};

class SocaChordsEditor : public juce::AudioProcessorEditor,
                         public juce::DragAndDropContainer,
                         private juce::Timer
{
public:
    explicit SocaChordsEditor (SocaChordsProcessor&);
    ~SocaChordsEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    /** The tile you drag from. Writes a .mid file and hands it to the DAW. */
    struct DragTile : juce::Component
    {
        explicit DragTile (SocaChordsEditor& e) : owner (e) { setMouseCursor (juce::MouseCursor::DraggingHandCursor); }
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override { dragging = false; }
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
        void mouseExit (const juce::MouseEvent&) override { hover = false; repaint(); }
        SocaChordsEditor& owner;
        bool dragging = false, hover = false;
    };

    /** One octave of keys: each key plays a whole chord. */
    struct TriggerKeys : juce::Component
    {
        explicit TriggerKeys (SocaChordsEditor& e) : owner (e) {}
        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        int keyAt (juce::Point<float>) const;
        juce::Rectangle<float> keyBounds (int pitchClass) const;
        SocaChordsEditor& owner;
        int mouseKey = -1;
    };

    void timerCallback() override;
    void syncFromProcessor();
    void rebuildFilter();
    void step (int delta);
    void select (int libraryIndex);
    void selectChord (int chordIndex);
    void refreshChordControls();
    juce::File makeMidiFile() const;
    void renderBackdrop();
    juce::Rectangle<float> tileBounds (int chordIndex) const;
    int tileColour (int chordIndex) const;
    const soca::Progression& currentProgression() const;

    SocaChordsProcessor& proc;
    SunsetLookAndFeel lookAndFeel;
    std::vector<int> filtered;          // library indices that pass the filters
    int selectedChord = 0;
    int seenStateVersion = -1, shownTriggerKey = -2, shownBpm = 0;
    float lastBeat = -2.0f;
    juce::Rectangle<int> panelArea, tilesArea, keysPanel, stripArea, filterArea, chipArea, inversionLabel, voicingLabel;
    juce::Image backdrop;               // gradient, feathers and panels, painted once

    juce::ComboBox styleBox, modeBox, keyBox, rhythmBox;
    juce::TextButton prevButton { "<" }, nextButton { ">" }, randomButton { "Random" }, playButton { "Play" };
    juce::TextButton inversionButtons[4], voicingButtons[3];
    juce::ToggleButton bassToggle { "Bass" };
    juce::Slider volumeSlider;
    DragTile dragTile { *this };
    TriggerKeys triggerKeys { *this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SocaChordsEditor)
};
