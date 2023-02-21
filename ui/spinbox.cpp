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

#include "spinbox.h"

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO


SpinBox::SpinBox(Widget* parent, int x0, int y0, int width, int height):
    CairoSubWidget(parent),
    surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height)),
    context(cairo_create(surface)),
    glow(width/2, height),
    label_layout(context),
    value_layout(context)
{
    setAbsolutePos(x0, y0);
    setSize(width, height);

    label_layout.set_font("orbitron", 16, PANGO_WEIGHT_HEAVY);
    label_layout.set_width(width/2-16);
    label_layout.set_alignment(PANGO_ALIGN_LEFT);

    value_layout.set_font("orbitron", 16, PANGO_WEIGHT_MEDIUM);
    value_layout.set_width(width/2-16);
    value_layout.set_alignment(PANGO_ALIGN_RIGHT);

    displayfn=default_display_func;
}


void SpinBox::set_value(int val)
{
    value=val;
    repaint();
}


void SpinBox::set_bounds(int minval, int maxval)
{
    minvalue=minval;
    maxvalue=maxval;
    repaint();
}


void SpinBox::set_label(const char* label)
{
    label_layout.set_text(label);
}


void SpinBox::set_display_func(std::function<const char*(int)> fn)
{
    displayfn=fn;
}


void SpinBox::set_callback(Callback* cb)
{
    callback=cb;
}


bool SpinBox::onScroll(const ScrollEvent& event)
{
    if (!contains(event.pos))
        return false;

    switch (event.direction) {
    case kScrollDown:
    case kScrollRight:
        if (value>=maxvalue) return true;
        value++;
        break;
    case kScrollUp:
    case kScrollLeft:
        if (value<=minvalue) return true;
        value--;
        break;
    default:
        return false;
    }

    if (callback) {
        callback->value_change_begin(this, value);
        callback->value_changed(this, value);
        callback->value_change_end(this, value);
    }

    repaint();
    return true;
}


void SpinBox::onCairoDisplay(const CairoGraphicsContext& ctx)
{
    cairo_t* cr=ctx.handle;

    int w=getWidth();
    int h=getHeight();

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    cairo_rounded_rectangle(cr, w/2+0.5, 0.5, w/2-1.0, h-1.0, 8.0);

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_fill_preserve(cr);

    cairo_set_line_width(cr, 2.0);
    cairo_set_source_rgb(cr, 0.6, 0.8, 1.0);
    cairo_stroke(cr);


    glow.clear();

    cairo_t* crimg=glow.get_context();

    cairo_set_source_rgb(crimg, 0.6, 0.8, 1.0);
    cairo_move_to(crimg, 8, 8);
    label_layout.show(crimg);

    glow.glow();

    cairo_set_source_surface(cr, glow.get_surface(), 0, 0);
    cairo_rectangle(cr, 0, 0, w/2, h);
    cairo_fill(cr);
    
    cairo_set_source_rgb(cr, 0.6, 0.8, 1.0);
    cairo_move_to(cr, w/2+8.0, 8.0);
    value_layout.set_text(displayfn(value));
    value_layout.show(cr);
}


const char* SpinBox::default_display_func(int val)
{
    static char buf[64];
    sprintf(buf, "%d", val);
    return buf;
}

}
