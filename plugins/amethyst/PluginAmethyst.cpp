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
    for (int i=0;i<frames;i++) {
        outputs[0][i]=0.0f;
        outputs[1][i]=0.0f;
    }
}


Plugin* createPlugin()
{
    return new DistrhoPluginAmethyst();
}

END_NAMESPACE_DISTRHO
