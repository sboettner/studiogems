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

#include "lineedit.h"

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO


LineEdit::LineEdit(Widget* parent, int x0, int y0, int width, int height):
    CairoSubWidget(parent),
    surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height)),
    context(cairo_create(surface)),
    layout(context)
{
    setAbsolutePos(x0, y0);
    setSize(width, height);

    layout.set_font("orbitron", 12, PANGO_WEIGHT_MEDIUM);
}


const char* LineEdit::get_text() const
{
    return buffer;
}


void LineEdit::set_text(const char* text)
{
    strcpy(buffer, text);
    
    layout.set_text(buffer);

    repaint();
}


void LineEdit::set_textf(const char* fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, list);
    va_end(list);
    
    layout.set_text(buffer);

    repaint();
}


void LineEdit::set_callback(Callback* cb)
{
    callback=cb;
}


void LineEdit::onCairoDisplay(const CairoGraphicsContext& ctx)
{
    cairo_t* cr=ctx.handle;

    cairo_rectangle(cr, 0.0, 0.0, getWidth(), getHeight());

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_fill_preserve(cr);

    cairo_set_source_rgb(cr, 0.6, 0.8, 1.0);
    cairo_stroke(cr);

    cairo_move_to(cr, 4.0, 4.0);
    layout.show(cr);

    double curx, cury, curh;
    layout.get_cursor_pos(strlen(buffer), curx, cury, curh);

    curx+=4.0;
    cury+=4.0;

    cairo_move_to(cr, curx, cury);
    cairo_line_to(cr, curx, cury+curh);
    cairo_stroke(cr);
}


bool LineEdit::onCharacterInput(const CharacterInputEvent& event)
{
    if (event.character==8) {
        // backspace

        // this is utterly wrong for multi-byte characters
        int len=strlen(buffer);
        if (len)
            buffer[--len]=0;
        
        layout.set_text(buffer);

        if (callback)
            callback->text_changed(this, buffer);

        repaint();
        return true;
    }

    if (event.character==13) {
        // return
        if (callback)
            callback->text_entered(this, buffer);

        return true;
    }


    if (event.character==27) {
        // escape
        if (callback)
            callback->text_cancelled(this, buffer);

        return true;
    }

    if (event.string[0]<=32 || event.string[0]>=128)
        return false;

    strcat(buffer, event.string);

    layout.set_text(buffer);

    if (callback)
        callback->text_changed(this, buffer);

    repaint();

    return true;
}

}
