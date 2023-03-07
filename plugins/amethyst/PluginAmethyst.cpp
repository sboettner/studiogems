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
    }
}


float DistrhoPluginAmethyst::getParameterValue(uint32_t index) const
{
    switch (index) {
    case PARAM_ATTACK:
        return attack;
    case PARAM_DECAY:
        return decay;
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
    }
}


void DistrhoPluginAmethyst::activate()
{
}


void DistrhoPluginAmethyst::deactivate()
{
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

        outputs[0][i]=pinknoisesrc[0]() * energy;
        outputs[1][i]=pinknoisesrc[1]() * energy;
    }
}


Plugin* createPlugin()
{
    return new DistrhoPluginAmethyst();
}

END_NAMESPACE_DISTRHO
