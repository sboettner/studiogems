/*
 * Studio Gems DISTRHO Plugins
 * Copyright (C) 2022 Stefan T. Boettner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the LICENSE file.
 */

#ifndef DISTRHO_PLUGIN_SAPPHIRE_H_INCLUDED
#define DISTRHO_PLUGIN_SAPPHIRE_H_INCLUDED

#include <vector>
#include <memory>
#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

struct ExcitationParameters {
    float   attack=1.0f;
    float   decay=100.0f;
    float   sustain=1.0f;
    float   release=10.0f;
};


struct FilterParameters {
    float   cutoff=1000.0f;
    float   envelope=1.0f;
    float   lfo=0.0f;
    float   modulation=0.0f;
    float   feedback=0.0f;
};


class Downsampler {
public:
    Downsampler();
    ~Downsampler();

    float* get_input_buffer(uint& size);
    void write_output(float* dst, uint size);

private:
    const static uint bufsize=8192;
    const static uint bufmask=bufsize-1;

    float*  buffer;
    uint    fillptr=0;
};


class DistrhoPluginSapphire : public Plugin
{
public:
    enum parameter_t {
        PARAM_BRIGHTNESS,
        PARAM_FALLOFF,
        PARAM_TWO_FACTOR,
        PARAM_THREE_FACTOR,
        PARAM_FIVE_FACTOR,
        PARAM_SEVEN_FACTOR,
        PARAM_HIGHER_FACTOR,
        PARAM_BANDWIDTH,
        PARAM_BANDWIDTH_EXPONENT,
        PARAM_EXCITATION_ATTACK,
        PARAM_EXCITATION_DECAY,
        PARAM_EXCITATION_SUSTAIN,
        PARAM_EXCITATION_RELEASE,
        PARAM_FILTER_CUTOFF,
        PARAM_FILTER_ENVELOPE,
        PARAM_FILTER_LFO,
        PARAM_FILTER_MODULATION,
        PARAM_FILTER_FEEDBACK,
        NUM_PARAMETERS
    };

    DistrhoPluginSapphire();
    ~DistrhoPluginSapphire() override;

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override
    {
        return "SapphirePadSynth";
    }

    const char* getDescription() const override
    {
        return "PAD Synth Plugin";
    }

    const char* getMaker() const noexcept override
    {
        return "Stefan T. Boettner";
    }

    const char* getHomePage() const override
    {
        return "https://...";
    }

    const char* getLicense() const noexcept override
    {
        return "GPL v3+";
    }

    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('S', 'G', 'S', 'p');
    }

    // -------------------------------------------------------------------
    // Init

    void initAudioPort(bool input, uint32_t index, AudioPort& port) override;
    void initParameter(uint32_t index, Parameter& parameter) override;

    // -------------------------------------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void  setParameterValue(uint32_t index, float value) override;

    // -------------------------------------------------------------------
    // Process

    void activate() override;
    void deactivate() override;
    void run(const float**, float** outputs, uint32_t frames, const MidiEvent* midievents, uint32_t nummidievents) override;

    // -------------------------------------------------------------------

private:
    void invalidate_waveform();

    struct Waveform {
        double* sample;
        int     length;
        int     period;     // length of one quasi-period in number of samples

        Waveform(int length, int period);
        ~Waveform();
    };

    struct Voice;

    double      step=0.0;
    double      phase=0.0;

    float       brightness=0.0f;
    float       falloff=1.0f;

    float       two_factor=1.0f;
    float       three_factor=1.0f;
    float       five_factor=1.0f;
    float       seven_factor=1.0f;
    float       higher_factor=1.0f;

    float       bandwidth=10.0f;            // bandwidth in cents for the fundamental
    float       bandwidth_exponent=1.0f;

    ExcitationParameters    excitation;
    FilterParameters        filter;

    Downsampler downsamplers[2];

    Waveform*   waveform=nullptr;

    std::vector<std::unique_ptr<Voice>> voices;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistrhoPluginSapphire)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // DISTRHO_PLUGIN_SAPPHIRE_H_INCLUDED
