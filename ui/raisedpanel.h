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

#ifndef INCLUDE_STUDIOGEMS_RAISEDPANEL_H
#define INCLUDE_STUDIOGEMS_RAISEDPANEL_H

#include <cairohelper.h>
#include <Cairo.hpp>

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO

class RaisedPanel:public CairoSubWidget {
public:
    RaisedPanel(Widget* parent, uint x0, uint y0, uint width, uint height);
    ~RaisedPanel();

protected:
    void onCairoDisplay(const CairoGraphicsContext&) override;

private:
    cairo_surface_t*    surface;
};

}

#endif
