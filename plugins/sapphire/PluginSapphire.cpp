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

#include "PluginSapphire.h"

START_NAMESPACE_DISTRHO


DistrhoPluginSapphire::DistrhoPluginSapphire():Plugin(NUM_PARAMETERS, 0, 0)
{
    deactivate();
}


DistrhoPluginSapphire::~DistrhoPluginSapphire()
{
}


void DistrhoPluginSapphire::initAudioPort(bool input, uint32_t index, AudioPort& port)
{
    port.groupId = kPortGroupMono;

    Plugin::initAudioPort(input, index, port);
}


void DistrhoPluginSapphire::initParameter(uint32_t index, Parameter& parameter)
{
    switch (index) {
    case PARAM_BRIGHTNESS:
        parameter.hints      = 0;
        parameter.name       = "Brightness";
        parameter.symbol     = "brightness";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -0.25f;
        parameter.ranges.max = 4.0f;
        break;        
    case PARAM_FALLOFF:
        parameter.hints      = 0;
        parameter.name       = "Fall-off";
        parameter.symbol     = "falloff";
        parameter.ranges.def = 1.0;
        parameter.ranges.min = 0.5f;
        parameter.ranges.max = 4.0f;
        break;        
    case PARAM_TWO_FACTOR:
        parameter.hints      = 0;
        parameter.name       = "Two";
        parameter.symbol     = "two";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_THREE_FACTOR:
        parameter.hints      = 0;
        parameter.name       = "Three";
        parameter.symbol     = "three";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_FIVE_FACTOR:
        parameter.hints      = 0;
        parameter.name       = "Five";
        parameter.symbol     = "five";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_SEVEN_FACTOR:
        parameter.hints      = 0;
        parameter.name       = "Seven";
        parameter.symbol     = "seven";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_HIGHER_FACTOR:
        parameter.hints      = 0;
        parameter.name       = "Higher";
        parameter.symbol     = "higher";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    }
}


float DistrhoPluginSapphire::getParameterValue(uint32_t index) const
{
    return 0.0;
}


void DistrhoPluginSapphire::setParameterValue(uint32_t index, float value)
{
}


void DistrhoPluginSapphire::activate()
{
}


void DistrhoPluginSapphire::deactivate()
{
}


void DistrhoPluginSapphire::run(const float**, float** outputs, uint32_t frames, const MidiEvent* midievents, uint32_t nummidievents)
{
    for (uint32_t i=0;i<frames;i++) {
        outputs[0][i]=0.0f;
        outputs[1][i]=0.0f;
    }
}


Plugin* createPlugin()
{
    return new DistrhoPluginSapphire();
}


END_NAMESPACE_DISTRHO
