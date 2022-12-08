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

#include "knob.h"

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO

Knob::Knob(Widget* parent, Size size, uint x0, uint y0, uint width, uint height):
    CairoSubWidget(parent), 
    KnobEventHandler(this), 
    conepat1(width/2), 
    conepat2(width/2), 
    glow(width, height),
    header_layout(glow.get_context()),
    value_layout(glow.get_context())
{
    setSize(width, height);
    setAbsolutePos(x0, y0);

    color=Color(0.2f, 1.0f, 0.5f);

    switch (size) {
    case Size::TINY:
        scale=24.0;
        header_layout.set_font("orbitron", 12, PANGO_WEIGHT_HEAVY);
        value_layout.set_font("orbitron", 10, PANGO_WEIGHT_BOLD);
        break;
    case Size::SMALL:
        scale=32.0;
        header_layout.set_font("orbitron", 14, PANGO_WEIGHT_HEAVY);
        value_layout.set_font("orbitron", 12, PANGO_WEIGHT_BOLD);
        break;
    case Size::MEDIUM:
        scale=48.0;
        header_layout.set_font("orbitron", 16, PANGO_WEIGHT_HEAVY);
        value_layout.set_font("orbitron", 14, PANGO_WEIGHT_MEDIUM);
        break;
    case Size::LARGE:
        scale=64.0;
        header_layout.set_font("orbitron", 18, PANGO_WEIGHT_HEAVY);
        value_layout.set_font("orbitron", 16, PANGO_WEIGHT_MEDIUM);
        break;
    case Size::HUGE:
        scale=96.0;
        header_layout.set_font("orbitron", 20, PANGO_WEIGHT_HEAVY);
        value_layout.set_font("orbitron", 18, PANGO_WEIGHT_MEDIUM);
        break;
    }

    header_layout.add_attribute("salt");
    header_layout.set_width(width);
    header_layout.set_alignment(PANGO_ALIGN_CENTER);

    value_layout.add_attribute("salt");
    value_layout.set_width(width);
    value_layout.set_alignment(PANGO_ALIGN_CENTER);

    conepat1.add_stop(M_PI/4, Color(0.9f, 0.9f, 0.9f));
    conepat1.add_stop(M_PI*3/4, Color(0.3f, 0.3f, 0.3f));
    conepat1.add_stop(M_PI*5/4, Color(0.9f, 0.9f, 0.9f));
    conepat1.add_stop(M_PI*7/4, Color(0.3f, 0.3f, 0.3f));
    conepat1.add_stop(M_PI*9/4, Color(0.9f, 0.9f, 0.9f));
    conepat1.set_center(width/2, height/2);

    conepat2.add_stop(M_PI/4, Color(0.6f, 0.6f, 0.6f));
    conepat2.add_stop(M_PI*3/4, Color(0.2f, 0.2f, 0.2f));
    conepat2.add_stop(M_PI*5/4, Color(1.0f, 1.0f, 1.0f));
    conepat2.add_stop(M_PI*7/4, Color(0.2f, 0.2f, 0.2f));
    conepat2.add_stop(M_PI*9/4, Color(0.6f, 0.6f, 0.6f));
    conepat2.set_center(width/2, height/2);
}


void Knob::set_name(const char* name)
{
    header_layout.set_text(name);
}


void Knob::set_color(const Color& col)
{
    color=col;
}


bool Knob::onMouse(const MouseEvent& event)
{
    return mouseEvent(event);
}

bool Knob::onMotion(const MotionEvent& event)
{
    return motionEvent(event);
}

bool Knob::onScroll(const ScrollEvent& event)
{
    return scrollEvent(event);
}


void Knob::onCairoDisplay(const CairoGraphicsContext& ctx)
{
    cairo_t* cr=ctx.handle;

    const double cx=getWidth()/2;
    const double cy=getHeight()/2;

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.75);
    cairo_arc(cr, cx, cy, scale*0.96875, 0, 2*M_PI);
    cairo_fill(cr);

    cairo_set_source(cr, conepat1);
    cairo_arc(cr, cx, cy, scale*0.75, 0, 2*M_PI);
    cairo_fill(cr);

    cairo_set_source(cr, conepat2);
    cairo_arc(cr, cx, cy, scale*0.75, 0, 2*M_PI);
    cairo_set_line_width(cr, scale/16);
    cairo_stroke(cr);

    const float phi=M_PI*(0.75+1.5*getNormalizedValue());

    cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
    cairo_move_to(cr, cx+cos(phi)*scale*0.250, cy+sin(phi)*scale*0.250);
    cairo_line_to(cr, cx+cos(phi)*scale*0.625, cy+sin(phi)*scale*0.625);
    cairo_set_line_width(cr, 3.0);
    cairo_stroke(cr);


    //cairo_set_source_rgb(cr, 0.05, 0.25, 0.125);
    Color dark(0.0f, 0.0f, 0.0f);
    dark.interpolate(color, 0.25f);
    cairo_set_source_color(cr, dark);
    cairo_arc(cr, cx, cy, scale*0.875, M_PI*3/4, M_PI*9/4);
    cairo_set_line_width(cr, scale/12);
    cairo_stroke(cr);


    glow.clear();

    cairo_t* crimg=glow.get_context();

    cairo_set_source_rgb(crimg, 0.6, 0.8, 1.0);
    cairo_move_to(crimg, 0, 8);
    header_layout.show(crimg);
    cairo_new_path(crimg);

    cairo_move_to(crimg, 0, getHeight()-20);
    value_layout.set_textf("%.2f", getValue());
    value_layout.show(crimg);
    cairo_new_path(crimg);

    //cairo_set_source_rgb(crimg, 0.2, 1.0, 0.5);
    cairo_set_source_color(crimg, color);
    cairo_arc(crimg, cx, cy, scale*0.875, M_PI*0.75, phi);
    cairo_set_line_width(crimg, scale/12);
    cairo_stroke(crimg);

    glow.glow();

    cairo_set_source_surface(cr, glow.get_surface(), 0, 0);
    cairo_rectangle(cr, 0, 0, getWidth(), getHeight());
    cairo_fill(cr);
    

    cairo_pattern_t* shadow=cairo_pattern_create_linear(cx-M_SQRT1_2*scale, cy-M_SQRT1_2*scale, cx+M_SQRT1_2*scale, cy+M_SQRT1_2*scale);
    cairo_pattern_add_color_stop_rgba(shadow, 0.0, 0.0, 0.0, 0.0, 0.5);
    cairo_pattern_add_color_stop_rgba(shadow, 0.5, 0.0, 0.0, 0.0, 0.0);
    cairo_pattern_add_color_stop_rgba(shadow, 1.0, 0.25, 0.5, 1.0, 0.375);

    cairo_set_source(cr, shadow);
    //cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_arc(cr, cx, cy, scale*0.984375, 0, M_PI*2);
    cairo_set_line_width(cr, scale/16);
    cairo_stroke(cr);
}

}

