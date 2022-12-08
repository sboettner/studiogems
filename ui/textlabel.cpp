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

#include "textlabel.h"

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO

TextLabel::TextLabel(Widget* parent, uint x0, uint y0, uint width, uint height, uint margin):
    CairoSubWidget(parent),
    margin(margin),
    glow(width, height),
    layout(glow.get_context())
{
    setSize(width, height);
    setAbsolutePos(x0, y0);

    layout.set_font("orbitron", 24, PANGO_WEIGHT_HEAVY);
    layout.set_width(width - 2*margin);
}


void TextLabel::set_color(const Color& col)
{
    color=col;
}


void TextLabel::set_text(const char* text)
{
    layout.set_text(text);
}


void TextLabel::onCairoDisplay(const CairoGraphicsContext& ctx)
{
    cairo_t* cr=ctx.handle;

    glow.clear();

    cairo_t* crimg=glow.get_context();

    cairo_set_source_color(crimg, color);
    cairo_move_to(crimg, margin, margin);
    layout.show(crimg);
    cairo_new_path(crimg);

    glow.glow();

    cairo_set_source_surface(cr, glow.get_surface(), 0, 0);
    cairo_rectangle(cr, 0, 0, getWidth(), getHeight());
    cairo_fill(cr);
}

}
