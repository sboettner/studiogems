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

#ifndef DISTRHO_PLUGIN_ONYX_H_INCLUDED
#define DISTRHO_PLUGIN_ONYX_H_INCLUDED

#include <vector>
#include <memory>
#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class DistrhoPluginOnyx : public Plugin
{
public:
    enum parameter_t {
        PARAM_OSCA_WAVEFORM,
        PARAM_OSCA_SHAPE,
        PARAM_OSCA_FREQUENCY,
        PARAM_OSCA_AMPLITUDE,
        PARAM_OSCA_EXCITATION,
        PARAM_OSCA_LFO,
        PARAM_OSCB_WAVEFORM,
        PARAM_OSCB_SHAPE,
        PARAM_OSCB_FREQUENCY,
        PARAM_OSCB_AMPLITUDE,
        PARAM_OSCB_EXCITATION,
        PARAM_OSCB_LFO,
        PARAM_OSCC_WAVEFORM,
        PARAM_OSCC_SHAPE,
        PARAM_OSCC_FREQUENCY,
        PARAM_OSCC_AMPLITUDE,
        PARAM_OSCC_EXCITATION,
        PARAM_OSCC_LFO,
        PARAM_OSCD_WAVEFORM,
        PARAM_OSCD_SHAPE,
        PARAM_OSCD_FREQUENCY,
        PARAM_OSCD_AMPLITUDE,
        PARAM_OSCD_EXCITATION,
        PARAM_OSCD_LFO,
        PARAM_LFO_TYPE,
        PARAM_LFO_FREQUENCY,
        PARAM_EXCITATION_ATTACK,
        PARAM_EXCITATION_SUSTAIN,
        PARAM_EXCITATION_DECAY,
        PARAM_EXCITATION_RELEASE,
        PARAM_UNISON_VOICES,
        PARAM_UNISON_DETUNE,
        PARAM_UNISON_WIDTH,
        NUM_PARAMETERS
    };

    enum waveform_t {
        WAVEFORM_SINE,
        WAVEFORM_TRIANGLE,
        WAVEFORM_PULSE
    };

    enum lfo_type_t {
        LFO_TYPE_SINE,
        LFO_TYPE_FALLING,
        LFO_TYPE_RISING,
        LFO_TYPE_ONESHOT
    };

    struct OscillatorParameters {
        waveform_t  waveform=WAVEFORM_SINE;
        float       shape=0.0f;
        float       frequency=1.0f;
        float       amplitude=1.0f;
        float       excitation=1.0f;
        float       lfo=0.0f;
    };

    struct ExcitationParameters {
        float   attack=1.0f;
        float   sustain=1.0f;
        float   decay=1.0f;
        float   release=1.0f;
    };

    DistrhoPluginOnyx();
    ~DistrhoPluginOnyx() override;

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override
    {
        return "OnyxFM";
    }

    const char* getDescription() const override
    {
        return "Two operator FM synthesizer";
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
        return d_cconst('S', 'G', 'O', 'x');
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
    class Excitation;
    class OscillatorExcitation;
    class Voice;
    class Downsampler;

    OscillatorParameters    oscparams[4];
    ExcitationParameters    excparams;

    int                     lfo_type=0;
    float                   lfo_frequency=1.0f;

    int                     unison_voices=1;
    float                   unison_detune=10.0f;
    float                   unison_width=1.0f;

    std::vector<std::unique_ptr<Voice>>     voices;
    Downsampler*            downsamplers=nullptr;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistrhoPluginOnyx)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // DISTRHO_PLUGIN_ONYX_H_INCLUDED
