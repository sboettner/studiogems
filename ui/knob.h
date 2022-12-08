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

#ifndef INCLUDE_STUDIOGEMS_KNOB_H
#define INCLUDE_STUDIOGEMS_KNOB_H

#include <cairohelper.h>
#include <Cairo.hpp>

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO

class Knob:public CairoSubWidget, public KnobEventHandler {
public:
    enum class Size {
        TINY,
        SMALL,
        MEDIUM,
        LARGE,
        HUGE
    };

    Knob(Widget* parent, Size size, uint x0, uint y0, uint width, uint height);

    void set_name(const char*);
    void set_color(const Color&);

protected:
    bool onMouse(const MouseEvent& event) override;
    bool onMotion(const MotionEvent& event) override;
    bool onScroll(const ScrollEvent& event) override;

    void onCairoDisplay(const CairoGraphicsContext&) override;

private:
    double          scale;

    Color           color;

    ConicPattern    conepat1;
    ConicPattern    conepat2;

    GlowSurface     glow;
    TextLayout      header_layout;
    TextLayout      value_layout;
};


}

#endif
