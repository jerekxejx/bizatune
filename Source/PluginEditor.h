#pragma once
#include "PluginProcessor.h"

class BizatuneEditor : public juce::AudioProcessorEditor
{
public:
    explicit BizatuneEditor (BizatuneProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    BizatuneProcessor& proc;
    juce::ComboBox key, scale;
    juce::Slider speed { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider mix { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Label lKey, lScale, lSpeed, lMix, hint;
    std::unique_ptr<CA> aKey, aScale;
    std::unique_ptr<SA> aSpeed, aMix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BizatuneEditor)
};
