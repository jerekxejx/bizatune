#pragma once
#include <JuceHeader.h>

class BizatuneProcessor : public juce::AudioProcessor
{
public:
    BizatuneProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Bizatune"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();

private:
    static constexpr int W = 512;     // ventana de analisis (muestras diezmadas)
    static constexpr int J = 256;     // ventana de integracion YIN
    static constexpr int HD = 64;     // salto de analisis (muestras diezmadas)
    static constexpr int RB = 8192;   // buffers circulares
    static constexpr int M = RB - 1;

    float detect();
    void analyse();
    int nearest (float midi) const;

    std::atomic<float>* keyP = nullptr;
    std::atomic<float>* scaleP = nullptr;
    std::atomic<float>* speedP = nullptr;
    std::atomic<float>* mixP = nullptr;

    double sr = 44100.0;
    int D = 4, Ld = 0;
    float L = 1024.0f;

    // analisis
    float dec[W] {};
    int dw = 0, dcnt = 0, hopCnt = 0;
    float dacc = 0.0f;

    // estado de afinacion
    int keyI = 9, mask = 0xFFF, hold = 1;
    float al = 1.0f;
    float om = 0.0f;
    int T = 0, cn = 0;
    bool pv = false;
    float rT = 1.0f, rSm = 1.0f, outF = 0.0f;

    // cambio de tono
    std::array<std::vector<float>, 2> buf, outb;
    float bh[2] {};
    int wp = 0, bitCnt = 0;
    float p = 0.5f;
};
