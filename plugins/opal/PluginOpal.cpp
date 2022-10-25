/*
 * Studio Gems DISTRHO Plugins
 * Copyright (C) 2022 Stefan T. Boettner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the LICENSE file.
 */

#include "PluginOpal.h"

START_NAMESPACE_DISTRHO

DistrhoPluginOpal::DistrhoPluginOpal():Plugin(3, 0, 0)
{
}


DistrhoPluginOpal::~DistrhoPluginOpal()
{
}


void DistrhoPluginOpal::initAudioPort(bool input, uint32_t index, AudioPort& port)
{
}


void DistrhoPluginOpal::initParameter(uint32_t index, Parameter& parameter)
{
}


float DistrhoPluginOpal::getParameterValue(uint32_t index) const
{
    return 0.0;
}


void  DistrhoPluginOpal::setParameterValue(uint32_t index, float value)
{
}


void DistrhoPluginOpal::activate()
{
}


void DistrhoPluginOpal::deactivate()
{
}


void DistrhoPluginOpal::run(const float** inputs, float** outputs, uint32_t frames)
{
    // TODO: implement proper processing
    if (outputs[0] != inputs[0])
        std::memcpy(outputs[0], inputs[0], sizeof(float)*frames);

    if (outputs[1] != inputs[1])
        std::memcpy(outputs[1], inputs[1], sizeof(float)*frames);
}


Plugin* createPlugin()
{
    return new DistrhoPluginOpal();
}



END_NAMESPACE_DISTRHO
