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

#ifndef DISTRHO_UI_ONYX_H_INCLUDED
#define DISTRHO_UI_ONYX_H_INCLUDED

#include <DistrhoUI.hpp>
#include <knob.h>
#include <textlabel.h>
#include <raisedpanel.h>

START_NAMESPACE_DISTRHO

using namespace StudioGemsUI;

class OscillatorPanel;
class ExcitationPanel;

// -----------------------------------------------------------------------

class DistrhoUIOnyx : public UI, public KnobEventHandler::Callback {
public:
    DistrhoUIOnyx();
    ~DistrhoUIOnyx() override;

protected:
    // -------------------------------------------------------------------
    // DSP Callbacks

    void parameterChanged(uint32_t index, float value) override;

    // -------------------------------------------------------------------
    // Widget Callbacks
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

    //void onNanoDisplay() override;
    void onCairoDisplay(const CairoGraphicsContext&) override;

private:
    ScopedPointer<OscillatorPanel>  oscapanel;
    ScopedPointer<OscillatorPanel>  oscbpanel;
    ScopedPointer<OscillatorPanel>  osccpanel;
    ScopedPointer<OscillatorPanel>  oscdpanel;
    ScopedPointer<ExcitationPanel>  excpanel;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistrhoUIOnyx)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif // DISTRHO_UI_ONYX_H_INCLUDED
