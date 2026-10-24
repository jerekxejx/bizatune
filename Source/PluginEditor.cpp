#include "PluginEditor.h"

#define U8(s) juce::String (juce::CharPointer_UTF8 (s))

static const juce::Colour kRed (0xffe0143c), kPink (0xffff4d76), kDark (0xff1d0a10), kLine (0xff5a1424);

BizatuneEditor::BizatuneEditor (BizatuneProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setSize (520, 340);

    auto fillCombo = [&] (juce::ComboBox& cb, const char* id)
    {
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
            cb.addItemList (c->choices, 1);
        cb.setColour (juce::ComboBox::backgroundColourId, kDark);
        cb.setColour (juce::ComboBox::outlineColourId, kLine);
        cb.setColour (juce::ComboBox::textColourId, juce::Colours::white);
        cb.setColour (juce::ComboBox::arrowColourId, kPink);
        addAndMakeVisible (cb);
    };
    fillCombo (key, "key");
    fillCombo (scale, "scale");

    auto setupLabel = [&] (juce::Label& l, const juce::String& t)
    {
        l.setText (t, juce::dontSendNotification);
        l.setColour (juce::Label::textColourId, juce::Colour (0xffa47983));
        addAndMakeVisible (l);
    };
    setupLabel (lKey, "Tono");
    setupLabel (lScale, "Escala");
    setupLabel (lSpeed, U8 ("Velocidad de correcci\xc3\xb3n"));
    setupLabel (lMix, "Mezcla");

    for (auto* s : { &speed, &mix })
    {
        s->setColour (juce::Slider::thumbColourId, kPink);
        s->setColour (juce::Slider::trackColourId, kRed);
        s->setColour (juce::Slider::backgroundColourId, kDark);
        s->setColour (juce::Slider::rotarySliderFillColourId, kRed);
        s->setColour (juce::Slider::rotarySliderOutlineColourId, kLine);
        s->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        s->setColour (juce::Slider::textBoxOutlineColourId, kLine);
        addAndMakeVisible (*s);
    }

    hint.setText (U8 ("\xe2\x88\x92" "100 Robot          0 Trap          100 Natural"), juce::dontSendNotification);
    hint.setJustificationType (juce::Justification::centred);
    hint.setColour (juce::Label::textColourId, juce::Colour (0xff7a5560));
    addAndMakeVisible (hint);

    aKey = std::make_unique<CA> (proc.apvts, "key", key);
    aScale = std::make_unique<CA> (proc.apvts, "scale", scale);
    aSpeed = std::make_unique<SA> (proc.apvts, "speed", speed);
    aMix = std::make_unique<SA> (proc.apvts, "mix", mix);
}

void BizatuneEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a0716), 0.0f, (float) getHeight(),
                                             juce::Colour (0xff0a0507), (float) getWidth() * 0.6f, 0.0f, false));
    g.fillAll();
    g.setColour (kLine);
    g.drawRect (getLocalBounds(), 1);

    g.setColour (kPink);
    g.setFont (juce::FontOptions (46.0f, juce::Font::bold | juce::Font::italic));
    g.drawText ("Bizatune", 0, 8, getWidth(), 54, juce::Justification::centred);

    g.setColour (juce::Colour (0xffa47983));
    g.setFont (juce::FontOptions (12.0f));
    g.drawText (U8 ("AFINACI\xc3\x93N BAJO TIERRA"), 0, 60, getWidth(), 16, juce::Justification::centred);
}

void BizatuneEditor::resized()
{
    auto r = getLocalBounds().reduced (22);
    r.removeFromTop (70);

    auto row = r.removeFromTop (52);
    auto a = row.removeFromLeft (row.getWidth() / 2 - 8);
    row.removeFromLeft (16);
    lKey.setBounds (a.removeFromTop (20));
    key.setBounds (a.removeFromTop (28));
    lScale.setBounds (row.removeFromTop (20));
    scale.setBounds (row.removeFromTop (28));

    r.removeFromTop (14);
    auto bottom = r;
    auto mixArea = bottom.removeFromRight (100);
    lMix.setBounds (mixArea.removeFromTop (20));
    mix.setBounds (mixArea);

    bottom.removeFromRight (10);
    lSpeed.setBounds (bottom.removeFromTop (20));
    speed.setBounds (bottom.removeFromTop (40));
    hint.setBounds (bottom.removeFromTop (22));
}
