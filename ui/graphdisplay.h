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

#ifndef INCLUDE_STUDIOGEMS_GRAPHDISPLAY_H
#define INCLUDE_STUDIOGEMS_GRAPHDISPLAY_H

#include <cairohelper.h>
#include <Cairo.hpp>

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO

class GraphDisplay:public CairoSubWidget {
public:
    GraphDisplay(Widget* parent, uint x0, uint y0, uint width, uint height);
    ~GraphDisplay();

protected:
    virtual void draw_graph(cairo_t*)=0;

    void onCairoDisplay(const CairoGraphicsContext&) override;

    void plot(cairo_t* cr, float x0, float x1, float y0, float y1, std::function<std::pair<float, float>(float)> fn, int nsamples=16);

private:
    cairo_surface_t*    inset;
    GlowSurface         glow;
};

}

#endif
