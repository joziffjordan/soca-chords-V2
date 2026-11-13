#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
// ---------------------------------------------------------------------- colour ramps
// Each control uses three shades of one colour: light on top, base on the face, dark on the edge.
struct Ramp { juce::Colour light, base, dark, text; };
enum RampId { rPink, rOrange, rYellow, rTeal, rPurple, rBlue, rGreen, rWhite, rNight, numRamps };

const Ramp kRamps[numRamps] = {
    { juce::Colour (0xffff8cc0), juce::Colour (0xffdb1f78), juce::Colour (0xff96104f), juce::Colours::white },
    { juce::Colour (0xffffc07a), juce::Colour (0xffff8a1f), juce::Colour (0xffb8540a), juce::Colour (0xff3d1800) },
    { juce::Colour (0xffffeb94), juce::Colour (0xffffc81f), juce::Colour (0xffc08f00), juce::Colour (0xff402800) },
    { juce::Colour (0xff8cf0e8), juce::Colour (0xff19c2c6), juce::Colour (0xff0b7f84), juce::Colour (0xff03383a) },
    { juce::Colour (0xffb49bff), juce::Colour (0xff6f42e0), juce::Colour (0xff43209f), juce::Colours::white },
    { juce::Colour (0xff8ccbff), juce::Colour (0xff1a6bcc), juce::Colour (0xff0e4690), juce::Colours::white },
    { juce::Colour (0xffa4eda0), juce::Colour (0xff35b955), juce::Colour (0xff1c7f33), juce::Colour (0xff062b10) },
    { juce::Colours::white,      juce::Colour (0xfff1ecfa), juce::Colour (0xffbfb2da), juce::Colour (0xff3a1456) },
    { juce::Colour (0xff4a3f8f), juce::Colour (0xff2b2366), juce::Colour (0xff150f3d), juce::Colours::white },
};
const int kTileRamps[] = { rPink, rYellow, rTeal, rPurple, rOrange, rGreen, rBlue, rPink };

const juce::Colour kInk { 0xff3a1456 };
const juce::Colour kPlum { 0xff3a1456 };

const Ramp& ramp (int id) { return kRamps[juce::jlimit (0, (int) numRamps - 1, id)]; }

// ---------------------------------------------------------------------- fonts (bundled, open licence)
juce::Typeface::Ptr scriptFace()
{
    static const auto t = juce::Typeface::createSystemTypefaceFor (BinaryData::PacificoRegular_ttf, BinaryData::PacificoRegular_ttfSize);
    return t;
}
juce::Font font (float size, bool extraBold = false)
{
    static const auto bold  = juce::Typeface::createSystemTypefaceFor (BinaryData::PoppinsBold_ttf, BinaryData::PoppinsBold_ttfSize);
    static const auto extra = juce::Typeface::createSystemTypefaceFor (BinaryData::PoppinsExtraBold_ttf, BinaryData::PoppinsExtraBold_ttfSize);
    return juce::Font (juce::FontOptions (extraBold ? extra : bold).withHeight (size));
}

// ---------------------------------------------------------------------- 3D shapes
/** A raised block: dark edge underneath, gradient face, soft highlight along the top. */
void draw3D (juce::Graphics& g, juce::Rectangle<float> r, const Ramp& c, float radius, float depth, bool pressed = false)
{
    g.setColour (juce::Colours::black.withAlpha (0.22f));
    g.fillRoundedRectangle (r.translated (0.0f, 4.0f).reduced (1.0f, 0.0f), radius);
    g.setColour (c.dark);
    g.fillRoundedRectangle (r.withTrimmedTop (depth * 0.5f), radius);

    auto face = r.withTrimmedBottom (depth).translated (0.0f, pressed ? depth - 1.0f : 0.0f);
    juce::ColourGradient grad (c.light, 0.0f, face.getY(), c.base, 0.0f, face.getBottom(), false);
    grad.addColour (0.58, c.base);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (face, radius);

    juce::Graphics::ScopedSaveState state (g);
    g.reduceClipRegion (face.withHeight (juce::jmin (radius, face.getHeight() * 0.5f)).toNearestInt());
    g.setColour (juce::Colours::white.withAlpha (0.6f));
    g.drawRoundedRectangle (face.reduced (1.0f).translated (0.0f, 0.5f), radius - 1.0f, 2.0f);
}

juce::Rectangle<float> faceOf (juce::Rectangle<float> r, float depth, bool pressed = false)
{
    return r.withTrimmedBottom (depth).translated (0.0f, pressed ? depth - 1.0f : 0.0f);
}

void softPanel (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour fill, juce::Colour edge)
{
    g.setColour (juce::Colours::black.withAlpha (0.2f));
    g.fillRoundedRectangle (r.translated (0.0f, 12.0f).reduced (4.0f, 0.0f), 22.0f);
    g.setColour (edge);
    g.fillRoundedRectangle (r.translated (0.0f, 7.0f), 22.0f);
    g.setColour (fill);
    g.fillRoundedRectangle (r, 22.0f);
}

// ---------------------------------------------------------------------- carnival feathers
const float kDeg = juce::MathConstants<float>::pi / 180.0f;

void drawPlume (juce::Graphics& g, juce::Point<float> origin, float degrees, float length, float width, juce::Colour lower, juce::Colour upper)
{
    juce::Graphics::ScopedSaveState state (g);
    g.addTransform (juce::AffineTransform::rotation (degrees * kDeg).translated (origin.x, origin.y));
    const float start = length * 0.14f;
    juce::Path top, bottom;
    top.startNewSubPath (start, 0.0f);
    top.cubicTo (length * 0.3f, -width, length * 0.8f, -width, length, 0.0f);
    top.closeSubPath();
    bottom.startNewSubPath (start, 0.0f);
    bottom.cubicTo (length * 0.3f, width, length * 0.8f, width, length, 0.0f);
    bottom.closeSubPath();
    g.setColour (upper);
    g.fillPath (top);
    g.setColour (lower);
    g.fillPath (bottom);

    g.setColour (juce::Colour (0xff3c0a50).withAlpha (0.2f));
    for (float x = length * 0.22f; x < length * 0.9f; x += 14.0f)
    {
        g.drawLine (x, 0.0f, x + length * 0.06f, -width * 0.6f, 1.3f);
        g.drawLine (x, 0.0f, x + length * 0.06f, width * 0.6f, 1.3f);
    }
    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.drawLine (0.0f, 0.0f, length, 0.0f, 2.2f);
}

void drawQuill (juce::Graphics& g, juce::Point<float> origin, float degrees, float length, juce::Colour tip)
{
    juce::Graphics::ScopedSaveState state (g);
    g.addTransform (juce::AffineTransform::rotation (degrees * kDeg).translated (origin.x, origin.y));
    g.setColour (juce::Colour (0xfff4b45a));
    g.drawLine (0.0f, 0.0f, length, 0.0f, 5.0f);
    g.setColour (juce::Colour (0xff5a2a0a));
    for (float x = 6.0f; x < length - 4.0f; x += 14.0f)
        g.fillRect (x, -2.5f, 5.0f, 5.0f);
    juce::Path drop;
    drop.startNewSubPath (length - 4.0f, 0.0f);
    drop.cubicTo (length + 10.0f, -11.0f, length + 26.0f, -8.0f, length + 34.0f, 0.0f);
    drop.cubicTo (length + 26.0f, 8.0f, length + 10.0f, 11.0f, length - 4.0f, 0.0f);
    g.setColour (tip);
    g.fillPath (drop);
}

struct PlumeLayer { int rampId; float scale, width; };

/** A mas-costume fan: striped quills behind, four layers of plumes, a jewel where they meet. */
void drawFan (juce::Graphics& g, juce::Point<float> origin, float fromDeg, float toDeg, int count, float length,
              const PlumeLayer* layers, juce::Colour tip)
{
    for (int i = 0; i <= count; ++i)
        drawQuill (g, origin, fromDeg + (toDeg - fromDeg) * (float) i / (float) count, length * 1.14f, tip);

    for (int layer = 0; layer < 4; ++layer)
    {
        const int n = count + layer;
        const auto& c = ramp (layers[layer].rampId);
        for (int i = 0; i <= n; ++i)
        {
            const float angle = fromDeg + (toDeg - fromDeg) * ((float) i + 0.5f * (float) (layer % 2)) / (float) n;
            const float wobble = 0.92f + 0.08f * (float) ((i * 7 + layer * 3) % 5) / 4.0f;
            drawPlume (g, origin, angle, length * layers[layer].scale * wobble, layers[layer].width, c.base, c.light);
        }
    }
    g.setColour (ramp (rYellow).base);
    g.fillEllipse (origin.x - 26.0f, origin.y - 26.0f, 52.0f, 52.0f);
    g.setColour (juce::Colours::white.withAlpha (0.8f));
    g.drawEllipse (origin.x - 26.0f, origin.y - 26.0f, 52.0f, 52.0f, 4.0f);
    g.setColour (ramp (rPink).base);
    g.fillEllipse (origin.x - 11.0f, origin.y - 11.0f, 22.0f, 22.0f);
}

void drawSequin (juce::Graphics& g, float x, float y, float size, juce::Colour c)
{
    g.setColour (juce::Colour (0xff3c0a50).withAlpha (0.35f));
    g.fillEllipse (x, y + 2.0f, size, size);
    g.setGradientFill ({ juce::Colours::white, x + size * 0.35f, y + size * 0.3f, c, x + size * 0.8f, y + size * 0.8f, true });
    g.fillEllipse (x, y, size, size);
}

/** Script lettering: white outline, 3D edge in darker shades of the same colour. */
float drawScriptWord (juce::Graphics& g, const juce::String& text, float x, float baseline, float size, const Ramp& c)
{
    const juce::Font f (juce::FontOptions (scriptFace()).withHeight (size));
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText (f, text, x, baseline);
    juce::Path face;
    glyphs.createPath (face);
    const juce::PathStrokeType outline (size * 0.085f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

    g.setColour (juce::Colour (0xff28003c).withAlpha (0.3f));
    g.strokePath (face, outline, juce::AffineTransform::translation (0.0f, 13.0f));
    g.fillPath (face, juce::AffineTransform::translation (0.0f, 13.0f));
    for (int i = 8; i >= 1; --i)
    {
        g.setColour (i > 4 ? c.dark : c.base);
        const auto t = juce::AffineTransform::translation (0.0f, (float) i);
        g.strokePath (face, outline, t);
        g.fillPath (face, t);
    }
    g.setColour (juce::Colours::white);
    g.strokePath (face, outline);
    const auto b = face.getBounds();
    g.setGradientFill ({ c.light, 0.0f, b.getY(), c.light.interpolatedWith (c.base, 0.55f), 0.0f, b.getBottom(), false });
    g.fillPath (face);
    return b.getRight();
}

void label (juce::Graphics& g, const juce::String& text, juce::Rectangle<int> area, juce::Colour colour, bool shadow)
{
    g.setFont (font (11.5f, true));
    if (shadow)
    {
        g.setColour (juce::Colour (0xff3c0a3c).withAlpha (0.6f));
        g.drawText (text, area.translated (0, 1), juce::Justification::centredLeft);
    }
    g.setColour (colour);
    g.drawText (text, area, juce::Justification::centredLeft);
}
}

// ---------------------------------------------------------------------- look and feel
SunsetLookAndFeel::SunsetLookAndFeel()
{
    setColour (juce::ComboBox::textColourId, kInk);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colours::white);
    setColour (juce::PopupMenu::textColourId, kInk);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, ramp (rPink).base);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::Label::textColourId, kInk);
}

static int rampOf (juce::Button& b)
{
    const bool on = b.getToggleState();
    return (int) b.getProperties().getWithDefault (on ? "rampOn" : "ramp", (int) rWhite);
}

void SunsetLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto c = ramp (rampOf (b));
    if (over && ! down) { c.light = c.light.brighter (0.25f); c.base = c.base.brighter (0.08f); }
    draw3D (g, b.getLocalBounds().toFloat().withTrimmedBottom (5.0f), c, 13.0f, 5.0f, down);
}

void SunsetLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool down)
{
    const auto face = faceOf (b.getLocalBounds().toFloat().withTrimmedBottom (5.0f), 5.0f, down);
    g.setColour (ramp (rampOf (b)).text);
    g.setFont (font ((float) b.getProperties().getWithDefault ("fontSize", 15.0), true));
    g.drawText (b.getButtonText(), face.toNearestInt(), juce::Justification::centred, false);
}

void SunsetLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    const bool on = b.getToggleState();
    auto track = juce::Rectangle<float> (0.0f, ((float) b.getHeight() - 28.0f) * 0.5f, 50.0f, 28.0f);
    const auto& c = ramp (on ? rPink : rNight);
    g.setGradientFill ({ c.dark, 0.0f, track.getY(), c.base, 0.0f, track.getBottom(), false });
    g.fillRoundedRectangle (track, 14.0f);
    g.setColour (juce::Colours::black.withAlpha (0.3f));
    g.drawRoundedRectangle (track.reduced (0.5f), 14.0f, 1.0f);
    auto knob = juce::Rectangle<float> (on ? track.getRight() - 25.0f : track.getX() + 3.0f, track.getY() + 3.0f, 22.0f, 22.0f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (knob.translated (0.0f, 2.0f));
    g.setGradientFill ({ juce::Colours::white, 0.0f, knob.getY(), juce::Colour (0xffd9d2ec), 0.0f, knob.getBottom(), false });
    g.fillEllipse (knob);
    g.setColour (juce::Colours::white);
    g.setFont (font (14.0f));
    g.drawText (b.getButtonText(), b.getLocalBounds().withTrimmedLeft (60), juce::Justification::centredLeft);
}

void SunsetLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool down, int, int, int, int, juce::ComboBox&)
{
    draw3D (g, juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height - 5.0f), ramp (rWhite), 13.0f, 5.0f, down);
    juce::Path arrow;
    const float cx = (float) width - 20.0f, cy = ((float) height - 10.0f) * 0.5f;
    arrow.startNewSubPath (cx - 5.0f, cy - 2.5f);
    arrow.lineTo (cx, cy + 3.0f);
    arrow.lineTo (cx + 5.0f, cy - 2.5f);
    g.setColour (kInk);
    g.strokePath (arrow, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void SunsetLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& l)
{
    l.setBounds (12, 0, box.getWidth() - 44, box.getHeight() - 10);
    l.setFont (getComboBoxFont (box));
}

juce::Font SunsetLookAndFeel::getComboBoxFont (juce::ComboBox&) { return font (15.0f, true); }
juce::Font SunsetLookAndFeel::getPopupMenuFont() { return font (15.0f); }

void SunsetLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float, float,
                                          juce::Slider::SliderStyle, juce::Slider&)
{
    auto track = juce::Rectangle<float> ((float) x, (float) y + (float) height * 0.5f - 6.0f, (float) width, 12.0f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillRoundedRectangle (track, 6.0f);
    const auto& c = ramp (rPink);
    g.setGradientFill ({ c.light, 0.0f, track.getY(), c.base, 0.0f, track.getBottom(), false });
    g.fillRoundedRectangle (track.withRight (sliderPos), 6.0f);
    auto knob = juce::Rectangle<float> (22.0f, 22.0f).withCentre ({ sliderPos, track.getCentreY() });
    const auto& k = ramp (rYellow);
    g.setColour (k.dark);
    g.fillEllipse (knob.translated (0.0f, 3.0f));
    g.setGradientFill ({ k.light, 0.0f, knob.getY(), k.base, 0.0f, knob.getBottom(), false });
    g.fillEllipse (knob);
}

// ---------------------------------------------------------------------- editor
static const int kWidth = 880, kHeight = 680;

SocaChordsEditor::SocaChordsEditor (SocaChordsProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lookAndFeel);

    for (auto* box : { &styleBox, &modeBox, &keyBox, &rhythmBox })
    {
        addAndMakeVisible (box);
        box->setWantsKeyboardFocus (false);
    }
    styleBox.addItem ("All styles", 1);
    for (int i = 0; i < soca::kNumStyles; ++i) styleBox.addItem (soca::kStyles[i].name, i + 2);
    styleBox.onChange = [this] { proc.styleFilter = styleBox.getSelectedId() - 1; rebuildFilter(); };

    modeBox.addItem ("Major + minor", 1);
    modeBox.addItem ("Major", 2);
    modeBox.addItem ("Minor", 3);
    modeBox.onChange = [this] { proc.modeFilter = modeBox.getSelectedId() - 1; rebuildFilter(); };

    for (int i = 0; i < 12; ++i) keyBox.addItem (soca::keyName (i), i + 1);
    keyBox.onChange = [this] { proc.key = keyBox.getSelectedId() - 1; proc.notesChanged(); refreshChordControls(); repaint(); };

    for (int i = 0; i < soca::kNumRhythms; ++i) rhythmBox.addItem (soca::kRhythms[i].name, i + 1);
    rhythmBox.onChange = [this] { proc.rhythm = rhythmBox.getSelectedId() - 1; proc.notesChanged(); };

    auto setUp = [this] (juce::TextButton& b, int rampId, int rampOn, double fontSize)
    {
        addAndMakeVisible (b);
        b.setWantsKeyboardFocus (false);
        b.getProperties().set ("ramp", rampId);
        b.getProperties().set ("rampOn", rampOn);
        b.getProperties().set ("fontSize", fontSize);
    };
    setUp (prevButton, rPink, rPink, 24.0);
    setUp (nextButton, rPink, rPink, 24.0);
    setUp (randomButton, rGreen, rGreen, 17.0);
    setUp (playButton, rOrange, rTeal, 19.0);
    prevButton.onClick = [this] { step (-1); };
    nextButton.onClick = [this] { step (1); };
    randomButton.onClick = [this]
    {
        if (filtered.size() > 1)
        {
            int pick = proc.progression.load();
            while (pick == proc.progression.load())
                pick = filtered[(size_t) juce::Random::getSystemRandom().nextInt ((int) filtered.size())];
            select (pick);
        }
    };
    playButton.setClickingTogglesState (true);
    playButton.onClick = [this]
    {
        proc.playing = playButton.getToggleState();
        playButton.setButtonText (playButton.getToggleState() ? "Stop" : "Play");
    };

    for (int i = 0; i < 4; ++i)
    {
        setUp (inversionButtons[i], rWhite, rPink, 14.0);
        inversionButtons[i].setButtonText (soca::inversionName (i));
        inversionButtons[i].onClick = [this, i]
        {
            if (selectedChord < soca::kMaxChords)
                proc.inversions[proc.progression.load()][selectedChord] = (signed char) i;
            proc.notesChanged();
            refreshChordControls();
            repaint();
        };
    }
    for (int i = 0; i < 3; ++i)
    {
        setUp (voicingButtons[i], rWhite, rPurple, 14.0);
        voicingButtons[i].setButtonText (soca::kVoicingNames[i]);
        voicingButtons[i].onClick = [this, i] { proc.voicing = i; proc.notesChanged(); refreshChordControls(); };
    }

    addAndMakeVisible (bassToggle);
    bassToggle.setWantsKeyboardFocus (false);
    bassToggle.onClick = [this] { proc.bass = bassToggle.getToggleState(); proc.notesChanged(); };

    addAndMakeVisible (volumeSlider);
    volumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setRange (0.0, 1.0);
    volumeSlider.setWantsKeyboardFocus (false);
    volumeSlider.onValueChange = [this] { proc.volume = (float) volumeSlider.getValue(); };

    addAndMakeVisible (dragTile);
    addAndMakeVisible (triggerKeys);

    setWantsKeyboardFocus (true);
    setSize (kWidth, kHeight);
    syncFromProcessor();
    startTimerHz (30);
}

SocaChordsEditor::~SocaChordsEditor() { stopTimer(); setLookAndFeel (nullptr); }

const soca::Progression& SocaChordsEditor::currentProgression() const
{
    return soca::library()[(size_t) juce::jlimit (0, soca::kLibrarySize - 1, proc.progression.load())];
}

void SocaChordsEditor::syncFromProcessor()
{
    seenStateVersion = proc.stateVersion.load();
    styleBox.setSelectedId (proc.styleFilter.load() + 1, juce::dontSendNotification);
    modeBox.setSelectedId (proc.modeFilter.load() + 1, juce::dontSendNotification);
    keyBox.setSelectedId (proc.key.load() + 1, juce::dontSendNotification);
    rhythmBox.setSelectedId (proc.rhythm.load() + 1, juce::dontSendNotification);
    bassToggle.setToggleState (proc.bass.load(), juce::dontSendNotification);
    volumeSlider.setValue (proc.volume.load(), juce::dontSendNotification);
    playButton.setToggleState (proc.playing.load(), juce::dontSendNotification);
    playButton.setButtonText (proc.playing.load() ? "Stop" : "Play");

    // Keep the saved progression even if the saved filters would hide it.
    const int current = proc.progression.load();
    rebuildFilter();
    if (std::find (filtered.begin(), filtered.end(), current) == filtered.end())
    {
        proc.styleFilter = 0;
        proc.modeFilter = 0;
        styleBox.setSelectedId (1, juce::dontSendNotification);
        modeBox.setSelectedId (1, juce::dontSendNotification);
        proc.progression = current;
        rebuildFilter();
    }
    select (current);
}

void SocaChordsEditor::rebuildFilter()
{
    filtered.clear();
    const int style = proc.styleFilter.load();
    const int mode = proc.modeFilter.load();
    const auto& lib = soca::library();
    for (int i = 0; i < soca::kLibrarySize; ++i)
    {
        const auto& def = soca::kLibrary[i];
        if (! lib[(size_t) i].valid) continue;
        if (style > 0 && (def.styles & soca::kStyles[style - 1].flag) == 0) continue;
        if (mode == 1 && def.mode != soca::Maj) continue;
        if (mode == 2 && def.mode != soca::Min) continue;
        filtered.push_back (i);
    }
    if (! filtered.empty()
        && std::find (filtered.begin(), filtered.end(), proc.progression.load()) == filtered.end())
        select (filtered.front());
    repaint();
}

void SocaChordsEditor::select (int libraryIndex)
{
    const int next = juce::jlimit (0, soca::kLibrarySize - 1, libraryIndex);
    if (next != proc.progression.load()) selectedChord = 0;
    proc.progression = next;
    proc.notesChanged();
    refreshChordControls();
    repaint();
}

void SocaChordsEditor::selectChord (int chordIndex)
{
    selectedChord = juce::jlimit (0, juce::jmax (0, (int) currentProgression().chords.size() - 1), chordIndex);
    refreshChordControls();
    repaint();
}

/** Shows the selected chord's inversion and the current voicing on the buttons. */
void SocaChordsEditor::refreshChordControls()
{
    const auto& prog = currentProgression();
    selectedChord = juce::jlimit (0, juce::jmax (0, (int) prog.chords.size() - 1), selectedChord);
    if (prog.chords.empty()) return;

    const auto& chord = prog.chords[(size_t) selectedChord];
    const int available = soca::numInversions (chord);
    const int current = selectedChord < soca::kMaxChords
                            ? juce::jlimit (0, available - 1, (int) proc.inversions[proc.progression.load()][selectedChord].load()) : 0;
    const int tileRamp = tileColour (selectedChord);
    for (int i = 0; i < 4; ++i)
    {
        inversionButtons[i].setVisible (i < available);
        inversionButtons[i].getProperties().set ("rampOn", tileRamp);
        inversionButtons[i].setToggleState (i == current, juce::dontSendNotification);
    }
    for (int i = 0; i < 3; ++i)
        voicingButtons[i].setToggleState (i == proc.voicing.load(), juce::dontSendNotification);
    resized();
}

void SocaChordsEditor::step (int delta)
{
    if (filtered.empty()) return;
    auto it = std::find (filtered.begin(), filtered.end(), proc.progression.load());
    int pos = it == filtered.end() ? 0 : (int) std::distance (filtered.begin(), it);
    const int n = (int) filtered.size();
    select (filtered[(size_t) (((pos + delta) % n + n) % n)]);
}

bool SocaChordsEditor::keyPressed (const juce::KeyPress& k)
{
    if (k == juce::KeyPress::rightKey || k == juce::KeyPress::downKey) { step (1);  return true; }
    if (k == juce::KeyPress::leftKey  || k == juce::KeyPress::upKey)   { step (-1); return true; }
    return false;
}

void SocaChordsEditor::mouseDown (const juce::MouseEvent& e)
{
    const auto& prog = currentProgression();
    for (int i = 0; i < (int) prog.chords.size(); ++i)
        if (tileBounds (i).contains (e.position))
        {
            selectChord (i);
            return;
        }
}

juce::File SocaChordsEditor::makeMidiFile() const
{
    signed char inv[soca::kMaxChords];
    proc.copyInversions (proc.progression.load(), inv);
    return soca::writeMidiFile (proc.progression.load(), proc.key.load(), proc.rhythm.load(), proc.octave.load(),
                                proc.bass.load(), proc.voicing.load(), inv, proc.currentBpm.load());
}

void SocaChordsEditor::timerCallback()
{
    if (proc.stateVersion.load() != seenStateVersion)
        syncFromProcessor();

    const float beat = proc.playheadBeat.load();
    if (std::abs (beat - lastBeat) > 0.001f)
    {
        lastBeat = beat;
        repaint (tilesArea.expanded (8));
    }
    if (const int key = proc.triggerKeyDown.load(); key != shownTriggerKey)
    {
        shownTriggerKey = key;
        triggerKeys.repaint();
    }
    if (const int bpm = juce::roundToInt (proc.currentBpm.load()); bpm != shownBpm)
    {
        shownBpm = bpm;
        repaint (chipArea);
    }
}

// ---------------------------------------------------------------------- layout
int SocaChordsEditor::tileColour (int chordIndex) const
{
    return kTileRamps[(chordIndex + proc.progression.load()) % 8];
}

juce::Rectangle<float> SocaChordsEditor::tileBounds (int chordIndex) const
{
    const auto& prog = currentProgression();
    const float total = juce::jmax (1.0f, prog.totalBeats);
    const auto area = tilesArea.toFloat();
    float x = area.getX(), beats = 0.0f;
    for (int i = 0; i < (int) prog.chords.size(); ++i)
    {
        const float w = area.getWidth() * prog.chords[(size_t) i].beats / total;
        if (i == chordIndex)
            return juce::Rectangle<float> (x, area.getY(), w, area.getHeight()).reduced (6.0f, 0.0f);
        x += w;
        beats += prog.chords[(size_t) i].beats;
    }
    return {};
}

void SocaChordsEditor::resized()
{
    const int left = 26, right = kWidth - 26;
    chipArea = { right - 230, 22, 230, 44 };

    // filters
    filterArea = { left, 100, right - left, 66 };
    const int gap = 12, w = (filterArea.getWidth() - gap * 3) / 4;
    juce::ComboBox* boxes[] = { &styleBox, &modeBox, &keyBox, &rhythmBox };
    for (int i = 0; i < 4; ++i)
        boxes[i]->setBounds (left + i * (w + gap), filterArea.getY() + 18, w, 49);

    // main panel
    panelArea = { left, 180, right - left, 268 };
    auto inner = panelArea.reduced (18, 14);
    auto top = inner.removeFromTop (49);
    prevButton.setBounds (top.removeFromLeft (46));
    nextButton.setBounds (top.removeFromRight (46));
    inner.removeFromTop (8);
    tilesArea = inner.removeFromTop (108);
    inner.removeFromTop (10);

    auto controls = inner;
    auto invArea = controls.removeFromLeft ((controls.getWidth() - 22) / 2);
    controls.removeFromLeft (22);
    inversionLabel = invArea.removeFromTop (16);
    voicingLabel = controls.removeFromTop (16);
    invArea.removeFromTop (3);
    controls.removeFromTop (3);
    int visible = 0;
    for (auto& b : inversionButtons) if (b.isVisible()) ++visible;
    visible = juce::jmax (1, visible);
    const int invW = (invArea.getWidth() - 8 * (visible - 1)) / visible;
    for (int i = 0, shown = 0; i < 4; ++i)
        if (inversionButtons[i].isVisible())
            inversionButtons[i].setBounds (invArea.getX() + (shown++) * (invW + 8), invArea.getY(), invW, 43);
    const int voiW = (controls.getWidth() - 16) / 3;
    for (int i = 0; i < 3; ++i)
        voicingButtons[i].setBounds (controls.getX() + i * (voiW + 8), controls.getY(), voiW, 43);

    // bottom: trigger keyboard on the left, transport on the right
    keysPanel = { left, 494, 392, 160 };
    triggerKeys.setBounds (keysPanel.getX() + 14, keysPanel.getY() + 38, 364, 108);

    const int colX = keysPanel.getRight() + 16, colW = right - colX;
    playButton.setBounds (colX, 464, (colW - 10) / 2, 55);
    randomButton.setBounds (colX + (colW - 10) / 2 + 10, 464, (colW - 10) / 2, 55);
    stripArea = { colX, 530, colW, 44 };
    bassToggle.setBounds (stripArea.getX() + 12, stripArea.getY() + 6, 112, 32);
    volumeSlider.setBounds (stripArea.getRight() - 150, stripArea.getY() + 6, 138, 32);
    dragTile.setBounds (colX, 586, colW, 72);

    if (backdrop.isNull())
        renderBackdrop();
}

/** Everything that never changes is painted once into an image. */
void SocaChordsEditor::renderBackdrop()
{
    backdrop = juce::Image (juce::Image::ARGB, kWidth, kHeight, true);
    juce::Graphics g (backdrop);
    const float W = (float) kWidth, H = (float) kHeight;

    // sunset: yellow through orange and pink to violet
    juce::ColourGradient sky (juce::Colour (0xffffd84a), W * 0.05f, 0.0f, juce::Colour (0xff5c82ff), W * 0.95f, H, false);
    sky.addColour (0.28, juce::Colour (0xffffa23f));
    sky.addColour (0.56, juce::Colour (0xffff5f8f));
    sky.addColour (0.80, juce::Colour (0xffb45cf0));
    g.setGradientFill (sky);
    g.fillAll();
    g.setColour (juce::Colour (0xffffe98a).withAlpha (0.5f));
    g.fillEllipse (-120.0f, -140.0f, 420.0f, 420.0f);
    g.setColour (juce::Colour (0xff8a5cf5).withAlpha (0.42f));
    g.fillEllipse (520.0f, -200.0f, 520.0f, 520.0f);
    g.setColour (juce::Colour (0xffff8a3d).withAlpha (0.42f));
    g.fillEllipse (300.0f, 420.0f, 520.0f, 420.0f);

    // feather fans in the corners
    const PlumeLayer warm[] = { { rPink, 1.0f, 30.0f }, { rTeal, 0.82f, 28.0f }, { rOrange, 0.62f, 28.0f }, { rYellow, 0.42f, 24.0f } };
    const PlumeLayer cool[] = { { rPurple, 1.0f, 30.0f }, { rPink, 0.82f, 28.0f }, { rTeal, 0.62f, 28.0f }, { rYellow, 0.42f, 24.0f } };
    drawFan (g, { W + 6.0f, -14.0f }, 96.0f, 186.0f, 8, 250.0f, cool, ramp (rPink).base);
    drawFan (g, { -8.0f, H + 10.0f }, -88.0f, -2.0f, 8, 250.0f, warm, ramp (rTeal).base);
    drawFan (g, { W + 10.0f, H + 14.0f }, 182.0f, 268.0f, 6, 210.0f, warm, ramp (rYellow).base);

    drawSequin (g, 548.0f, 16.0f, 12.0f, ramp (rYellow).base);
    drawSequin (g, 576.0f, 50.0f, 9.0f, ramp (rTeal).base);
    drawSequin (g, 520.0f, 44.0f, 10.0f, ramp (rPink).base);
    drawSequin (g, 10.0f, 330.0f, 9.0f, ramp (rPink).base);
    drawSequin (g, 862.0f, 300.0f, 10.0f, ramp (rYellow).base);
    drawSequin (g, 862.0f, 470.0f, 9.0f, ramp (rTeal).base);

    // title
    const float end = drawScriptWord (g, "Soca", 30.0f, 68.0f, 88.0f, ramp (rPink));
    const float titleEnd = drawScriptWord (g, "Chords", end + 20.0f, 68.0f, 88.0f, ramp (rTeal));
    auto badge = juce::Rectangle<float> (titleEnd + 20.0f, 50.0f, 128.0f, 22.0f);
    g.setColour (juce::Colour (0xff2a1270));
    g.fillRoundedRectangle (badge.translated (0.0f, 3.0f), 11.0f);
    g.setGradientFill ({ ramp (rPurple).base, 0.0f, badge.getY(), ramp (rPurple).dark, 0.0f, badge.getBottom(), false });
    g.fillRoundedRectangle (badge, 10.5f);
    g.setColour (juce::Colours::white);
    g.setFont (font (13.5f, true));
    g.drawText ("by Dev Griffin", badge, juce::Justification::centred);

    // panels
    softPanel (g, panelArea.toFloat(), juce::Colours::white.withAlpha (0.88f), juce::Colour (0xff78288c).withAlpha (0.4f));
    softPanel (g, keysPanel.toFloat(), kPlum.withAlpha (0.88f), juce::Colour (0xff280a3c).withAlpha (0.6f));
    g.setColour (kPlum.withAlpha (0.88f));
    g.fillRoundedRectangle (stripArea.toFloat(), 14.0f);

    // fixed captions
    const char* captions[] = { "STYLE", "SCALE", "KEY", "RHYTHM" };
    juce::ComboBox* boxes[] = { &styleBox, &modeBox, &keyBox, &rhythmBox };
    for (int i = 0; i < 4; ++i)
        label (g, captions[i], { boxes[i]->getX() + 2, filterArea.getY(), 200, 16 }, juce::Colours::white, true);
    label (g, "VOICING (ALL CHORDS)", voicingLabel, ramp (rPurple).dark, false);

    g.setColour (juce::Colours::white);
    g.setFont (font (14.5f, true));
    g.drawText ("One-key chords", keysPanel.getX() + 16, keysPanel.getY() + 10, 160, 20, juce::Justification::centredLeft);
    g.setColour (ramp (rYellow).light);
    g.setFont (font (11.0f));
    g.drawText ("white = in key, black = borrowed", keysPanel.getRight() - 216, keysPanel.getY() + 10, 200, 20, juce::Justification::centredRight);
    g.setColour (juce::Colours::white);
    g.setFont (font (14.0f));
    g.drawText ("Volume", stripArea.getRight() - 214, stripArea.getY(), 60, stripArea.getHeight(), juce::Justification::centredLeft);
}

// ---------------------------------------------------------------------- painting
void SocaChordsEditor::paint (juce::Graphics& g)
{
    g.drawImageAt (backdrop, 0, 0);

    const int index = proc.progression.load();
    const auto& def = soca::kLibrary[index];
    const auto& prog = currentProgression();
    const int key = proc.key.load();

    // tempo and position chips
    auto it = std::find (filtered.begin(), filtered.end(), index);
    const int pos = it == filtered.end() ? 0 : (int) std::distance (filtered.begin(), it);
    const juce::String chips[] = { juce::String (juce::roundToInt (proc.currentBpm.load())) + " BPM",
                                   juce::String (pos + 1) + " / " + juce::String ((int) filtered.size()) };
    auto chipRow = chipArea;
    for (int i = 1; i >= 0; --i)
    {
        auto r = chipRow.removeFromRight (108).toFloat().withTrimmedBottom (6.0f);
        chipRow.removeFromRight (10);
        draw3D (g, r, ramp (rWhite), 13.0f, 5.0f);
        g.setColour (kInk);
        g.setFont (font (14.0f, true));
        g.drawText (chips[i], faceOf (r, 5.0f).toNearestInt(), juce::Justification::centred);
    }

    if (filtered.empty() || prog.chords.empty())
    {
        g.setColour (kInk);
        g.setFont (font (16.0f, true));
        g.drawText ("No progressions match these filters", tilesArea, juce::Justification::centred);
        return;
    }

    // name and tags
    auto nameArea = juce::Rectangle<int> (prevButton.getRight() + 14, prevButton.getY() - 2, nextButton.getX() - prevButton.getRight() - 28, 28);
    g.setColour (kInk);
    g.setFont (font (24.0f, true));
    g.drawText (def.name, nameArea, juce::Justification::centredLeft);
    juce::StringArray tags;
    for (int i = 0; i < soca::kNumStyles; ++i)
        if (def.styles & soca::kStyles[i].flag) tags.add (soca::kStyles[i].name);
    g.setColour (ramp (rPink).dark);
    g.setFont (font (13.0f, true));
    g.drawText (soca::noteName (key, soca::keyUsesSharps (key, def.mode)) + (def.mode == soca::Min ? " minor" : " major")
                    + "   |   " + tags.joinIntoString (", "),
                nameArea.translated (0, 26).withHeight (18), juce::Justification::centredLeft);

    // chord tiles
    const float beat = proc.playheadBeat.load();
    const bool isPlaying = beat >= 0.0f;
    float t = 0.0f;
    for (int i = 0; i < (int) prog.chords.size(); ++i)
    {
        const auto& c = prog.chords[(size_t) i];
        const auto r = tileBounds (i);
        const bool active = isPlaying && beat >= t && beat < t + c.beats;
        const auto& col = ramp (tileColour (i));
        const float radius = juce::jmin (18.0f, r.getWidth() * 0.3f);

        draw3D (g, r, col, radius, 8.0f, active);
        const auto face = faceOf (r, 8.0f, active);
        if (isPlaying && ! active)
        {
            g.setColour (juce::Colour (0xff28003c).withAlpha (0.28f));
            g.fillRoundedRectangle (face, radius);
        }
        if (i == selectedChord)
        {
            g.setColour (juce::Colours::white);
            g.drawRoundedRectangle (r.expanded (3.0f), radius + 3.0f, 4.0f);
            g.setColour (col.dark);
            g.drawRoundedRectangle (r.expanded (5.5f), radius + 5.0f, 1.5f);
        }

        const float nameSize = juce::jlimit (15.0f, 38.0f, face.getWidth() / 3.0f);
        g.setColour (col.text);
        g.setFont (font (nameSize, true));
        g.drawText (soca::chordName (c, key, def.mode), face.withTrimmedTop (face.getHeight() * 0.16f).withHeight (face.getHeight() * 0.5f),
                    juce::Justification::centred, false);
        g.setColour (col.text.withAlpha (0.85f));
        g.setFont (font (juce::jmin (15.0f, nameSize * 0.6f)));
        g.drawText (juce::String (c.roman), face.withTrimmedTop (face.getHeight() * 0.64f), juce::Justification::centred, false);

        // inversion tag
        const int inv = i < soca::kMaxChords ? juce::jlimit (0, soca::numInversions (c) - 1, (int) proc.inversions[index][i].load()) : 0;
        const juce::String tag = inv == 0 ? "Root" : soca::inversionName (inv) + " inv";
        const float tagW = juce::jmin (face.getWidth() - 10.0f, inv == 0 ? 42.0f : 54.0f);
        if (face.getWidth() > 60.0f)
        {
            auto chip = juce::Rectangle<float> (face.getRight() - tagW - 7.0f, face.getY() + 7.0f, tagW, 17.0f);
            g.setColour (col.dark);
            g.fillRoundedRectangle (chip, 8.5f);
            g.setColour (juce::Colours::white);
            g.setFont (font (10.5f, true));
            g.drawText (tag, chip, juce::Justification::centred, false);
        }
        t += c.beats;
    }

    // caption for the inversion buttons names the selected chord
    const auto& sel = prog.chords[(size_t) juce::jlimit (0, (int) prog.chords.size() - 1, selectedChord)];
    label (g, "INVERSION FOR SELECTED CHORD: " + soca::chordName (sel, key, def.mode).toUpperCase().replace ("M", "m"),
           inversionLabel, ramp (rPink).dark, false);
}

// ---------------------------------------------------------------------- trigger keyboard
static const int kWhitePitch[] = { 0, 2, 4, 5, 7, 9, 11 };
static const int kBlackPitch[] = { 1, 3, 6, 8, 10 };
static const int kBlackAfter[] = { 0, 1, 3, 4, 5 };      // the white key each black key follows

juce::Rectangle<float> SocaChordsEditor::TriggerKeys::keyBounds (int pitchClass) const
{
    const float keyW = (float) getWidth() / 7.0f, h = (float) getHeight();
    for (int i = 0; i < 7; ++i)
        if (kWhitePitch[i] == pitchClass)
            return { (float) i * keyW + 2.0f, 0.0f, keyW - 4.0f, h };
    for (int i = 0; i < 5; ++i)
        if (kBlackPitch[i] == pitchClass)
            return { ((float) kBlackAfter[i] + 1.0f) * keyW - keyW * 0.3f, 0.0f, keyW * 0.6f, h * 0.58f };
    return {};
}

int SocaChordsEditor::TriggerKeys::keyAt (juce::Point<float> p) const
{
    for (int pc : kBlackPitch) if (keyBounds (pc).contains (p)) return pc;
    for (int pc : kWhitePitch) if (keyBounds (pc).contains (p)) return pc;
    return -1;
}

void SocaChordsEditor::TriggerKeys::mouseDown (const juce::MouseEvent& e)
{
    mouseKey = keyAt (e.position);
    if (mouseKey >= 0) owner.proc.uiTriggerOn = 60 + mouseKey;
}

void SocaChordsEditor::TriggerKeys::mouseUp (const juce::MouseEvent&)
{
    if (mouseKey >= 0) owner.proc.uiTriggerOff = 60 + mouseKey;
    mouseKey = -1;
}

void SocaChordsEditor::TriggerKeys::paint (juce::Graphics& g)
{
    const auto& def = soca::kLibrary[owner.proc.progression.load()];
    const auto& prog = owner.currentProgression();
    const auto& set = soca::triggerChords (def.mode);
    const int down = owner.proc.triggerKeyDown.load();

    // a key takes the colour of the matching chord tile when that chord is in the progression
    auto colourFor = [&] (int pitchClass)
    {
        for (int i = 0; i < (int) prog.chords.size(); ++i)
            if (prog.chords[(size_t) i].degree == set.chords[pitchClass].degree
                && prog.chords[(size_t) i].minor == set.chords[pitchClass].minor)
                return owner.tileColour (i);
        return -1;
    };

    for (int pc : kWhitePitch)
    {
        const auto r = keyBounds (pc);
        const int tile = colourFor (pc);
        const auto& c = ramp (tile >= 0 ? tile : (int) rWhite);
        const bool pressed = pc == down;
        g.setColour (c.dark);
        g.fillRoundedRectangle (r, 8.0f);
        auto face = r.withTrimmedBottom (6.0f).translated (0.0f, pressed ? 5.0f : 0.0f);
        g.setGradientFill ({ c.light, 0.0f, face.getY(), c.base, 0.0f, face.getBottom(), false });
        g.fillRoundedRectangle (face, 8.0f);
        g.fillRect (face.withHeight (10.0f));
        g.setColour (c.text);
        g.setFont (font (13.5f, true));
        g.drawText (soca::triggerLabel (def.mode, pc), face.withTrimmedTop (face.getHeight() - 26.0f), juce::Justification::centred, false);
    }
    for (int pc : kBlackPitch)
    {
        const auto r = keyBounds (pc);
        const int tile = colourFor (pc);
        const bool pressed = pc == down;
        g.setColour (juce::Colour (0xff0a0624));
        g.fillRoundedRectangle (r, 6.0f);
        auto face = r.withTrimmedBottom (5.0f).translated (0.0f, pressed ? 4.0f : 0.0f);
        g.setGradientFill ({ juce::Colour (0xff4a3f8f), 0.0f, face.getY(), juce::Colour (0xff1b1440), 0.0f, face.getBottom(), false });
        g.fillRoundedRectangle (face, 6.0f);
        g.fillRect (face.withHeight (8.0f));
        g.setColour (tile >= 0 ? ramp (tile).light : ramp (rYellow).light);
        g.setFont (font (10.0f, true));
        g.drawText (soca::triggerLabel (def.mode, pc), face.withTrimmedTop (face.getHeight() - 20.0f), juce::Justification::centred, false);
    }
}

// ---------------------------------------------------------------------- drag tile
void SocaChordsEditor::DragTile::paint (juce::Graphics& g)
{
    auto c = ramp (rYellow);
    if (hover) c.light = c.light.brighter (0.3f);
    const auto r = getLocalBounds().toFloat().withTrimmedBottom (6.0f);
    draw3D (g, r, c, 18.0f, 6.0f);
    const auto face = faceOf (r, 6.0f).toNearestInt();
    g.setColour (c.text);
    g.setFont (font (22.0f, true));
    g.drawText ("DRAG MIDI", face.withTrimmedBottom (20), juce::Justification::centred);
    g.setColour (c.text.withAlpha (0.85f));
    g.setFont (font (12.0f));
    g.drawText ("drag onto a track", face.withTrimmedTop (34), juce::Justification::centred);
}

void SocaChordsEditor::DragTile::mouseDrag (const juce::MouseEvent& e)
{
    if (dragging || e.getDistanceFromDragStart() < 6) return;
    dragging = true;
    const auto file = owner.makeMidiFile();
    if (file.existsAsFile())
        juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this);
}
