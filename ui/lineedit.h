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

#ifndef INCLUDE_STUDIOGEMS_LINEEDIT_H
#define INCLUDE_STUDIOGEMS_LINEEDIT_H

#include <cairohelper.h>
#include <Cairo.hpp>

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO

class LineEdit:public CairoSubWidget {
public:
    class Callback {
    public:
        virtual void text_changed(SubWidget*, const char*) = 0;
        virtual void text_entered(SubWidget*, const char*) = 0;
        virtual void text_cancelled(SubWidget*, const char*) = 0;
    };

    LineEdit(Widget* parent, int x0, int y0, int width, int height);

    const char* get_text() const;
    void set_text(const char*);
    void set_textf(const char*, ...);

    void set_callback(Callback*);

protected:
    void onCairoDisplay(const CairoGraphicsContext&) override;

    bool onCharacterInput(const CharacterInputEvent&) override;

private:
    cairo_surface_t*    surface;
    cairo_t*            context;

    Callback*   callback=nullptr;

    TextLayout  layout;

    char        buffer[64];     // FIXME: static size buffer is bad
};

}

#endif
