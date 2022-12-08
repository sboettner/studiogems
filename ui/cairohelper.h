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

#ifndef INCLUDE_STUDIOGEMS_CAIROHELPER_H
#define INCLUDE_STUDIOGEMS_CAIROHELPER_H

#include <cairo/cairo.h>
#include <pango/pango.h>
#include <Color.hpp>


void cairo_rounded_rectangle(cairo_t* cr, double x0, double y0, double w, double h, double r);
void cairo_set_source_color(cairo_t* cr, const DGL_NAMESPACE::Color& color);


namespace StudioGemsUI {

class ConicPattern {
public:
    ConicPattern(float radius);
    ~ConicPattern();

    void add_stop(float phi, const DGL_NAMESPACE::Color& color);
    void set_center(double, double);

    operator cairo_pattern_t*() const
    {
        return pat;
    }

private:
    cairo_pattern_t*        pat=nullptr;

    float                   radius;
    float                   lastphi=0.0f;
    DGL_NAMESPACE::Color    lastcolor;
};


class GlowSurface {
public:
    GlowSurface(int, int);
    ~GlowSurface();

    void clear();
    void glow();

    cairo_surface_t* get_surface() const
    {
        return surface;
    }

    cairo_t* get_context() const
    {
        return ctx;
    }

private:
    static void blur_line(unsigned char* pixels, int length, int stride);

    int                 width, height;
    cairo_surface_t*    surface;
    cairo_t*            ctx;
};


class TextLayout {
public:
    TextLayout(cairo_t*);
    ~TextLayout();

    void add_attribute(const char*);

    void set_font(const char* family, int size, PangoWeight weight=PANGO_WEIGHT_NORMAL, PangoStyle style=PANGO_STYLE_NORMAL);
    void set_alignment(PangoAlignment);
    void set_width(int);
    void set_text(const char*);
    void set_textf(const char*, ...);
  
    void show(cairo_t*);

    static void register_font_file(const char*);

private:
    PangoFontDescription*   font;
    PangoAttrList*          attributes;
    PangoLayout*            layout;
};

}

#endif
