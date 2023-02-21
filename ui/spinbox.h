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

#ifndef INCLUDE_STUDIOGEMS_SPINBOX_H
#define INCLUDE_STUDIOGEMS_SPINBOX_H

#include <cairohelper.h>
#include <Cairo.hpp>

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO

class SpinBox:public CairoSubWidget {
public:
    class Callback {
    public:
        virtual void value_changed(SubWidget*, int) = 0;
        virtual void value_change_begin(SubWidget*, int) = 0;
        virtual void value_change_end(SubWidget*, int) = 0;
    };

    SpinBox(Widget* parent, int x0, int y0, int width, int height);

    void set_value(int);
    void set_bounds(int minval, int maxval);
    void set_label(const char*);
    void set_display_func(std::function<const char*(int)> fn);

    void set_callback(Callback*);

protected:
    bool onScroll(const ScrollEvent&) override;
    void onCairoDisplay(const CairoGraphicsContext&) override;

private:
    cairo_surface_t*    surface;
    cairo_t*            context;

    Callback*           callback=nullptr;

    GlowSurface         glow;
    TextLayout          label_layout;
    TextLayout          value_layout;

    int         value=0;
    int         minvalue=INT_MIN;
    int         maxvalue=INT_MAX;

    std::function<const char*(int)> displayfn;

    static const char* default_display_func(int);
};

}

#endif
