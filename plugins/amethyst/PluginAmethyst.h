/*
 * Studio Gems DISTRHO Plugins
 * Copyright (C) 2023 Stefan T. Boettner
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

#ifndef DISTRHO_PLUGIN_AMETHYST_H_INCLUDED
#define DISTRHO_PLUGIN_AMETHYST_H_INCLUDED

#include <vector>
#include <memory>
#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class DistrhoPluginAmethyst : public Plugin
{
public:
    enum parameter_t {
        PARAM_ATTACK,
        PARAM_DECAY,
        PARAM_COMB1_TUNING,
        PARAM_COMB1_FEEDFORWARD,
        PARAM_COMB1_FEEDBACK,
        PARAM_COMB1_ENVELOPE,
        PARAM_COMB2_TUNING,
        PARAM_COMB2_FEEDFORWARD,
        PARAM_COMB2_FEEDBACK,
        PARAM_COMB2_ENVELOPE,
        NUM_PARAMETERS
    };

    DistrhoPluginAmethyst();
    ~DistrhoPluginAmethyst() override;

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override
    {
        return "AmethystPercussion";
    }

    const char* getDescription() const override
    {
        return "Percussive Synth Plugin";
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
        return d_cconst('S', 'G', 'A', 'm');
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
    class PinkNoise {
        // Voss-McCartney algorithm

        float   noise[8] {};
        uint    index=0;

        static float white()
        {
            return ldexpf((rand()&0xfffff)-0x80000, -21);
        }

    public:
        float operator()()
        {
            if (index&1)
                noise[0]=white();
            else if (index&2)
                noise[1]=white();
            else if (index&4)
                noise[2]=white();
            else if (index&8)
                noise[3]=white();
            else if (index&16)
                noise[4]=white();
            else if (index&32)
                noise[5]=white();
            else if (index&64)
                noise[6]=white();
            else
                noise[7]=white();
            
            index++;

            float v=0.0f;
            for (int i=0;i<8;i++)
                v+=noise[i];

            return v;
        }
    };


    class BrownNoise {
        // integrated white noise
        float   sum=0.0f;

    public:
        float operator()()
        {
            sum*=0.999f;
            sum+=ldexpf((rand()&0xfffff)-0x80000, -22);
            return sum;
        }
    };

    struct CombParameters {
        float   tuning=0.0f;
        float   feedforward=0.0f;
        float   feedback=0.0f;
        float   envelope=0.0f;
    };

    class CombFilter;

    float       attack=0.0f;
    float       decay=1000.0f;

    float       latent_energy=0.0f;
    float       energy=0.0f;

    CombParameters  combparams1;
    CombParameters  combparams2;

    PinkNoise   pinknoisesrc[2];
    BrownNoise  brownnoisesrc[2];

    CombFilter* comb1[2] {};
    CombFilter* comb2[2] {};

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistrhoPluginAmethyst)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif  // DISTRHO_PLUGIN_AMETHYST_H_INCLUDED
