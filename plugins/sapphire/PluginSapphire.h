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

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

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

    int         curnote=-1;

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

    Waveform*   waveform=nullptr;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistrhoPluginSapphire)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // DISTRHO_PLUGIN_SAPPHIRE_H_INCLUDED
