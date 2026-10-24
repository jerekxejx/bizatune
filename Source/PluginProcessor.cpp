#include "PluginProcessor.h"
#include "PluginEditor.h"

#define U8(s) juce::String (juce::CharPointer_UTF8 (s))

static const int kMasks[5] = { 0xFFF, 2741, 1453, 1193, 661 };

juce::AudioProcessorValueTreeState::ParameterLayout BizatuneProcessor::layout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "key", 1 }, "Tono",
        StringArray { "Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa#", "Sol", "Sol#", "La", "La#", "Si" }, 9));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "scale", 1 }, "Escala",
        StringArray { U8 ("Crom\xc3\xa1tica"), "Mayor", "Menor",
                      U8 ("Pentat\xc3\xb3nica menor"), U8 ("Pentat\xc3\xb3nica mayor") }, 2));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "speed", 1 }, "Velocidad",
        NormalisableRange<float> (-100.0f, 100.0f, 1.0f), 0.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mezcla",
        NormalisableRange<float> (0.0f, 100.0f, 1.0f), 100.0f));
    return l;
}

BizatuneProcessor::BizatuneProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "BIZATUNE", layout())
{
    keyP = apvts.getRawParameterValue ("key");
    scaleP = apvts.getRawParameterValue ("scale");
    speedP = apvts.getRawParameterValue ("speed");
    mixP = apvts.getRawParameterValue ("mix");
}

bool BizatuneProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto o = l.getMainOutputChannelSet();
    return (o == juce::AudioChannelSet::mono() || o == juce::AudioChannelSet::stereo())
           && o == l.getMainInputChannelSet();
}

void BizatuneProcessor::prepareToPlay (double rate, int)
{
    sr = rate;
    D = juce::jmax (1, (int) std::round (sr / 11025.0));
    L = (float) juce::jmin (0.025 * sr, 3000.0);
    Ld = (int) (L * 0.5f) + 2;
    setLatencySamples (Ld);
    for (auto& v : buf) v.assign ((size_t) RB, 0.0f);
    for (auto& v : outb) v.assign ((size_t) RB, 0.0f);
    std::fill (std::begin (dec), std::end (dec), 0.0f);
    dw = dcnt = hopCnt = wp = bitCnt = 0;
    dacc = 0.0f; pv = false; rT = rSm = 1.0f; outF = 0.0f; p = 0.5f;
    bh[0] = bh[1] = 0.0f;
}

int BizatuneProcessor::nearest (float m) const
{
    const int r = (int) std::round (m);
    int best = r;
    float bd = 99.0f;
    for (int c = r - 6; c <= r + 6; ++c)
    {
        const int pc = ((c - keyI) % 12 + 12) % 12;
        if (! ((mask >> pc) & 1)) continue;
        const float d = std::abs ((float) c - m);
        if (d < bd) { bd = d; best = c; }
    }
    return best;
}

// Deteccion de tono (YIN) sobre el audio diezmado. Devuelve Hz, o 0 si no hay voz.
float BizatuneProcessor::detect()
{
    float d[W], df[W];
    for (int j = 0; j < W; ++j) d[j] = dec[(dw + j) % W];

    float e = 0.0f;
    for (int j = 0; j < W; ++j) e += d[j] * d[j];
    if (std::sqrt (e / W) < 0.01f) return 0.0f;

    const float ds = (float) (sr / D);
    const int tmin = juce::jmax (2, (int) (ds / 900.0f));
    const int tmax = juce::jmin ((int) (ds / 70.0f), W - J - 1);

    float run = 0.0f;
    df[0] = 1.0f;
    for (int t = 1; t <= tmax; ++t)
    {
        float a = 0.0f;
        for (int j = 0; j < J; ++j) { const float q = d[j] - d[j + t]; a += q * q; }
        run += a;
        df[t] = run > 0.0f ? a * (float) t / run : 1.0f;
    }

    int f1 = 0, t = tmin;
    while (t < tmax)
    {
        if (df[t] < 0.15f)
        {
            while (t + 1 < tmax && df[t + 1] < df[t]) ++t;
            f1 = t;
            break;
        }
        ++t;
    }
    if (! f1)
    {
        int m = tmin;
        for (t = tmin; t <= tmax; ++t) if (df[t] < df[m]) m = t;
        if (df[m] < 0.3f) f1 = m;
    }
    if (! f1) return 0.0f;

    float tt = (float) f1;
    if (f1 > 1 && f1 < tmax)
    {
        const float a = df[f1 - 1], b = df[f1], c = df[f1 + 1], de = a - 2.0f * b + c;
        if (de != 0.0f) tt += 0.5f * (a - c) / de;
    }
    return ds / tt;
}

void BizatuneProcessor::analyse()
{
    const float f = detect();
    if (f <= 0.0f) { pv = false; rT = 1.0f; outF = 0.0f; return; }

    const float m = 69.0f + 12.0f * std::log2 (f / 440.0f);
    if (! pv) { om = m; cn = 0; }
    if (! pv || cn % hold == 0) T = nearest (m);
    ++cn;
    pv = true;
    om += al * ((float) T - om);
    rT = juce::jlimit (0.5f, 2.0f, std::pow (2.0f, (om - m) / 12.0f));
    outF = 440.0f * std::pow (2.0f, (om - 69.0f) / 12.0f);
}

void BizatuneProcessor::processBlock (juce::AudioBuffer<float>& b, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int nc = juce::jmin (2, b.getNumChannels());
    const int n = b.getNumSamples();
    if (nc == 0) return;

    const float v = speedP->load();
    const float mix = mixP->load() * 0.01f;
    keyI = (int) keyP->load();
    mask = kMasks[juce::jlimit (0, 4, (int) scaleP->load())];

    const float neg = v < 0.0f ? -v / 100.0f : 0.0f;
    const float tau = v > 0.0f ? v / 100.0f * 250.0f : 0.0f;
    const float hopMs = 1000.0f * (float) (HD * D) / (float) sr;
    al = tau > 0.0f ? 1.0f - std::exp (-hopMs / tau) : 1.0f;
    hold = 1 + (int) std::round (neg * 24.0f);
    const float g = 0.6f * neg;
    const float outGain = 1.0f / (1.0f + 0.8f * g);
    const int bitK = 1 + (int) (neg * 3.0f);

    float* io[2] = { nullptr, nullptr };
    for (int c = 0; c < nc; ++c) io[c] = b.getWritePointer (c);

    for (int i = 0; i < n; ++i)
    {
        float mono = 0.0f;
        for (int c = 0; c < nc; ++c) mono += io[c][i];
        mono /= (float) nc;

        dacc += mono;
        if (++dcnt >= D)
        {
            dec[dw] = dacc / (float) D;
            dw = (dw + 1) % W;
            dacc = 0.0f; dcnt = 0;
            if (++hopCnt >= HD) { hopCnt = 0; analyse(); }
        }

        rSm += 0.02f * (rT - rSm);
        float dr = (1.0f - rSm) / L;
        if (! pv || std::abs (1.0f - rSm) < 0.003f)
            dr += juce::jlimit (-1.0e-4f, 1.0e-4f, 0.5f - p);   // sin correccion: vuelve a un solo cabezal limpio
        p += dr;
        p -= std::floor (p);

        for (int c = 0; c < nc; ++c)
        {
            auto& bf = buf[(size_t) c];
            bf[(size_t) wp] = io[c][i];

            float s = 0.0f;
            for (int h = 0; h < 2; ++h)
            {
                float q = p + 0.5f * (float) h;
                q -= std::floor (q);
                float pos = (float) wp - (q * L + 2.0f);
                if (pos < 0.0f) pos += (float) RB;
                const int k = (int) pos;
                const float fr = pos - (float) k;
                s += (bf[(size_t) (k & M)] * (1.0f - fr) + bf[(size_t) ((k + 1) & M)] * fr)
                     * (0.5f - 0.5f * std::cos (6.2831853f * q));
            }

            if (g > 0.0f && outF > 0.0f)
            {
                const int P = juce::jlimit (2, RB - 2, (int) ((float) sr / outF));
                s += g * outb[(size_t) c][(size_t) ((wp - P) & M)];
            }
            outb[(size_t) c][(size_t) wp] = s;

            if (bitK > 1)
            {
                if (bitCnt % bitK == 0) bh[c] = s;
                s = bh[c];
            }

            const float dry = bf[(size_t) ((wp - Ld) & M)];
            io[c][i] = dry * (1.0f - mix) + s * outGain * mix;
        }
        ++bitCnt;
        wp = (wp + 1) & M;
    }
}

juce::AudioProcessorEditor* BizatuneProcessor::createEditor() { return new BizatuneEditor (*this); }

void BizatuneProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void BizatuneProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BizatuneProcessor(); }
