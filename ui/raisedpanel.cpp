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

#include "raisedpanel.h"

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO

RaisedPanel::RaisedPanel(Widget* parent, uint x0, uint y0, uint width, uint height):CairoSubWidget(parent)
{
    const uint shadesize=16;
    const uint w=width  + 2*shadesize;
    const uint h=height + 2*shadesize;

    setAbsolutePos(x0-shadesize, y0-shadesize);
    setSize(w, h);

    surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);

    cairo_t* cr=cairo_create(surface);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    cairo_pattern_t* backgnd=cairo_pattern_create_linear(0, 0, width, height);
    cairo_pattern_add_color_stop_rgb(backgnd, 0.0, 0.05, 0.1, 0.2);
    cairo_pattern_add_color_stop_rgb(backgnd, 1.0, 0.1, 0.2, 0.4);
    cairo_set_source(cr, backgnd);

    cairo_rounded_rectangle(cr, shadesize, shadesize, width, height, 8);
    cairo_fill_preserve(cr);

    double dx=height, dy=width;
    double dlen=hypot(dx, dy);
    dx*=4.0/dlen;
    dy*=4.0/dlen;

    cairo_pattern_t* framepat=cairo_pattern_create_linear(w/2-dx, h/2-dy, w/2+dx, h/2+dy);
    cairo_pattern_add_color_stop_rgba(framepat, 0.0, 0.25, 0.5, 1.0, 0.25);
    cairo_pattern_add_color_stop_rgba(framepat, 1.0, 0.0, 0.0, 0.0, 0.5);
    cairo_set_source(cr, framepat);
    cairo_stroke(cr);

    cairo_surface_flush(surface);

    // create smooth drop shadow
    unsigned char* pixels=cairo_image_surface_get_data(surface);

    unsigned char* shadowx=new unsigned char[w];
    unsigned char* shadowy=new unsigned char[h];

    for (int x=0;x<2*shadesize;x++) {
        float sh=(0.5f*x+0.25f) / shadesize;
        sh*=sh*sh*(10.0f + sh*(6.0f*sh - 15.0f));
        shadowx[x]=shadowx[w-x-1]=lrintf(sh*255.0f);
    }

    for (int x=2*shadesize;x+2*shadesize<w;x++)
        shadowx[x]=255;

    for (int y=0;y<2*shadesize;y++) {
        float sh=(0.5f*y+0.25f) / shadesize;
        sh*=sh*sh*(10.0f + sh*(6.0f*sh - 15.0f));
        shadowy[y]=shadowy[h-y-1]=lrintf(sh*255.0f);
    }

    for (int y=2*shadesize;y+2*shadesize<h;y++)
        shadowy[y]=255;

    for (uint y=0;y<h;y++) {
        for (uint x=0;x<w;x++) {
            int sh=shadowx[x]*shadowy[y];
            int alpha=pixels[3];

            pixels[0]=pixels[0]*alpha/255;
            pixels[1]=pixels[1]*alpha/255;
            pixels[2]=pixels[2]*alpha/255;
            pixels[3]=(pixels[3]*alpha*255 + sh*(255-alpha)) / 65025;
            pixels+=4;
        }
    }

    delete[] shadowx;
    delete[] shadowy;

    cairo_surface_mark_dirty(surface);
}


RaisedPanel::~RaisedPanel()
{
    cairo_surface_destroy(surface);
}


void RaisedPanel::onCairoDisplay(const CairoGraphicsContext& ctx)
{
    cairo_t* cr=ctx.handle;

    cairo_set_source_surface(cr, surface, 0.0, 0.0);
    cairo_rectangle(cr, 0, 0, getWidth(), getHeight());
    cairo_fill(cr);
}

}
