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

#include "UIOpal.h"

START_NAMESPACE_DISTRHO

DistrhoUIOpal::DistrhoUIOpal()
{
}


DistrhoUIOpal::~DistrhoUIOpal()
{
}


void DistrhoUIOpal::parameterChanged(uint32_t index, float value)
{
}


void DistrhoUIOpal::onDisplay()
{
}


UI* createUI()
{
    return new DistrhoUIOpal();
}

END_NAMESPACE_DISTRHO
