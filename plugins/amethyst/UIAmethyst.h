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

#ifndef DISTRHO_UI_AMETHYST_H_INCLUDED
#define DISTRHO_UI_AMETHYST_H_INCLUDED

#include "DistrhoUI.hpp"
#include <knob.h>
#include <textlabel.h>


START_NAMESPACE_DISTRHO

using namespace StudioGemsUI;

// -----------------------------------------------------------------------

class DistrhoUIAmethyst : public UI {
public:
    DistrhoUIAmethyst();
    ~DistrhoUIAmethyst() override;

protected:
    // -------------------------------------------------------------------
    // DSP Callbacks

    void parameterChanged(uint32_t index, float value) override;

    void onCairoDisplay(const CairoGraphicsContext&) override;

private:
    class ExcitationPanel;

    ScopedPointer<TextLabel>        title;
    ScopedPointer<ExcitationPanel>  excitationpanel;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistrhoUIAmethyst)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif // DISTRHO_UI_AMETHYST_H_INCLUDED
