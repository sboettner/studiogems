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

#include "PluginAmethyst.h"

START_NAMESPACE_DISTRHO

class DistrhoPluginAmethyst::CombFilter {
    const static uint bufsize=4096;
    const static uint bufmask=4095;

    const CombParameters& params;

    float*  ringbuffer;
    uint    wrptr=0;

public:
    CombFilter(const CombParameters&);
    ~CombFilter();

    float operator()(float sample, float env);
};


DistrhoPluginAmethyst::CombFilter::CombFilter(const CombParameters& params):params(params)
{
    ringbuffer=new float[bufsize];

    for (uint i=0;i<bufsize;i++)
        ringbuffer[i]=0.0f;
}


DistrhoPluginAmethyst::CombFilter::~CombFilter()
{
    delete[] ringbuffer;
}


float DistrhoPluginAmethyst::CombFilter::operator()(float v, float env)
{
    float freq=440.0f * expf(params.tuning*M_LN2/12.0f);
    uint delay=lrintf(48000 / freq);
    
    env=powf(env, params.envelope);
    if (env>1) env=1.0f;

    float w=ringbuffer[(wrptr-delay)&bufmask]*0.5f + ringbuffer[(wrptr-delay-1)&bufmask]*0.25f + ringbuffer[(wrptr-delay+1)&bufmask]*0.25f;
    v+=w*params.feedback*env;

    ringbuffer[wrptr++]=v;
    wrptr&=bufmask;

    v+=w*params.feedforward*env;

    return v;
}


DistrhoPluginAmethyst::DistrhoPluginAmethyst():Plugin(NUM_PARAMETERS, 0, 0)
{
}


DistrhoPluginAmethyst::~DistrhoPluginAmethyst()
{
    deactivate();
}


void DistrhoPluginAmethyst::initAudioPort(bool input, uint32_t index, AudioPort& port)
{
    port.groupId = kPortGroupMono;

    Plugin::initAudioPort(input, index, port);
}


void DistrhoPluginAmethyst::initParameter(uint32_t index, Parameter& parameter)
{
    switch (index) {
    case PARAM_ATTACK:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Attack";
        parameter.symbol     = "attack";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1000.0f;
        break;
    case PARAM_DECAY:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Decay";
        parameter.symbol     = "decay";
        parameter.ranges.def = 1000.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1000.0f;
        break;
    case PARAM_COMB1_TUNING:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Comb 1 Tuning";
        parameter.symbol     = "combtuning1";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -12.0f;
        parameter.ranges.max = 36.0f;
        break;
    case PARAM_COMB1_FEEDFORWARD:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Comb 1 Feedforward";
        parameter.symbol     = "combfwd1";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -1.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_COMB1_FEEDBACK:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Comb 1 Feedback";
        parameter.symbol     = "combback1";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -1.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_COMB1_ENVELOPE:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Comb 1 Envelope";
        parameter.symbol     = "combenv1";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 4.0f;
        break;
    case PARAM_COMB2_TUNING:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Comb 2 Tuning";
        parameter.symbol     = "combtuning2";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -12.0f;
        parameter.ranges.max = 36.0f;
        break;
    case PARAM_COMB2_FEEDFORWARD:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Comb 2 Feedforward";
        parameter.symbol     = "combfwd2";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -1.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_COMB2_FEEDBACK:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Comb 2 Feedback";
        parameter.symbol     = "combback2";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -1.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_COMB2_ENVELOPE:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Comb 2 Envelope";
        parameter.symbol     = "combenv2";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 4.0f;
        break;
    }
}


float DistrhoPluginAmethyst::getParameterValue(uint32_t index) const
{
    switch (index) {
    case PARAM_ATTACK:
        return attack;
    case PARAM_DECAY:
        return decay;
    case PARAM_COMB1_TUNING:
        return combparams1.tuning;
    case PARAM_COMB1_FEEDFORWARD:
        return combparams1.feedforward;
    case PARAM_COMB1_FEEDBACK:
        return combparams1.feedback;
    case PARAM_COMB1_ENVELOPE:
        return combparams1.envelope;
    case PARAM_COMB2_TUNING:
        return combparams2.tuning;
    case PARAM_COMB2_FEEDFORWARD:
        return combparams2.feedforward;
    case PARAM_COMB2_FEEDBACK:
        return combparams2.feedback;
    case PARAM_COMB2_ENVELOPE:
        return combparams2.envelope;
    default:
        return 0.0f;
    }
}


void DistrhoPluginAmethyst::setParameterValue(uint32_t index, float value)
{
    switch (index) {
    case PARAM_ATTACK:
        attack=value;
        break;
    case PARAM_DECAY:
        decay=value;
        break;
    case PARAM_COMB1_TUNING:
        combparams1.tuning=value;
        break;
    case PARAM_COMB1_FEEDFORWARD:
        combparams1.feedforward=value;
        break;
    case PARAM_COMB1_FEEDBACK:
        combparams1.feedback=value;
        break;
    case PARAM_COMB1_ENVELOPE:
        combparams1.envelope=value;
        break;
    case PARAM_COMB2_TUNING:
        combparams2.tuning=value;
        break;
    case PARAM_COMB2_FEEDFORWARD:
        combparams2.feedforward=value;
        break;
    case PARAM_COMB2_FEEDBACK:
        combparams2.feedback=value;
        break;
    case PARAM_COMB2_ENVELOPE:
        combparams2.envelope=value;
        break;
    }
}


void DistrhoPluginAmethyst::activate()
{
    comb1[0]=new CombFilter(combparams1);
    comb1[1]=new CombFilter(combparams1);
    comb2[0]=new CombFilter(combparams2);
    comb2[1]=new CombFilter(combparams2);
}


void DistrhoPluginAmethyst::deactivate()
{
    delete comb1[0];
    delete comb1[1];
    delete comb2[0];
    delete comb2[1];

    comb1[0]=comb1[1]=comb2[0]=comb2[1]=nullptr;
}


void DistrhoPluginAmethyst::run(const float**, float** outputs, uint32_t frames, const MidiEvent* midievents, uint32_t nummidievents)
{
    for (uint32_t i=0;i<nummidievents;i++) {
        const auto& ev=midievents[i];

        if ((ev.data[0]&0xf0)==0x90)
            latent_energy+=ev.data[2]/127.0f;
    }

    float attack_rate=1000.0f / attack / getSampleRate();
    float decay_rate =1000.0f / decay  / getSampleRate();

    if (attack_rate>1) attack_rate=1.0f;
    if (decay_rate >1) decay_rate =1.0f;

    for (uint32_t i=0;i<frames;i++) {
        energy-=energy*decay_rate;

        float v=latent_energy*attack_rate;
        latent_energy-=v;
        energy+=v;

        outputs[0][i]=(*comb2[0])((*comb1[0])(pinknoisesrc[0]() * energy, energy), energy);
        outputs[1][i]=(*comb2[1])((*comb1[1])(pinknoisesrc[1]() * energy, energy), energy);
    }
}


Plugin* createPlugin()
{
    return new DistrhoPluginAmethyst();
}

END_NAMESPACE_DISTRHO
