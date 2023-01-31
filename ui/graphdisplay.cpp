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

#include <alloca.h>
#include "graphdisplay.h"

namespace StudioGemsUI {

USE_NAMESPACE_DISTRHO

GraphDisplay::GraphDisplay(Widget* parent, uint x0, uint y0, uint width, uint height):
    CairoSubWidget(parent), 
    glow(width, height)
{
    setSize(width, height);
    setAbsolutePos(x0, y0);

    inset=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

    unsigned char* pixels=cairo_image_surface_get_data(inset);

    unsigned char* shadowx=new unsigned char[width];
    unsigned char* shadowy=new unsigned char[height];

    for (unsigned int x=0;x<width;x++) {
        unsigned int s=255;
        if (x<16)
            s=255-((16-x)*(16-x)*(16-x)+16)/32;
        else if (width-x<16)
            s=255-((16-width+x)*(16-width+x)*(16-width+x)+16)/32;

        unsigned int t=128 + 256 * x*(width-x-1) / (width*width);

        shadowx[x]=s*t/255;
    }

    for (unsigned int y=0;y<height;y++) {
        unsigned int s=255;
        if (y<16)
            s=255-((16-y)*(16-y)*(16-y)+16)/32;
        else if (height-y<16)
            s=255-((16-height+y)*(16-height+y)*(16-height+y)+16)/32;

        unsigned int t=128 + 256 * y*(height-y-1) / (height*height);

        shadowy[y]=s*t/255;
    }

    for (unsigned int y=0;y<height;y++) {
        for (unsigned int x=0;x<width;x++) {
            unsigned int s=shadowx[x]*shadowy[y]/255;

            *pixels++=s / 2;
            *pixels++=s / 4;
            *pixels++=s / 8;
            *pixels++=255;
        }
    }

    delete[] shadowx;
    delete[] shadowy;

    cairo_surface_mark_dirty(inset);
}


GraphDisplay::~GraphDisplay()
{
    cairo_surface_destroy(inset);
}


void GraphDisplay::onCairoDisplay(const CairoGraphicsContext& ctx)
{
    glow.clear();

    cairo_set_operator(glow.get_context(), CAIRO_OPERATOR_ADD);

    draw_graph(glow.get_context());

    glow.glow();

    cairo_t* cr=ctx.handle;

    cairo_set_source_surface(cr, inset, 0.0, 0.0);
    cairo_rounded_rectangle(cr, 0, 0, getWidth(), getHeight(), 8);
    cairo_fill_preserve(cr);

    cairo_set_source_surface(cr, glow.get_surface(), 0.0, 0.0);
    cairo_fill(cr);
}


void GraphDisplay::plot(cairo_t* cr, float x0, float x1, float y0, float y1, std::function<float(float)> fn, int nsamples)
{
    float* y=(float*) alloca((nsamples+1)*sizeof(float));
    float* dy=(float*) alloca((nsamples+1)*sizeof(float));

    for (int i=0;i<=nsamples;i++)
        y[i]=(fn(x0 + (x1-x0)*i/nsamples) - y1) / (y0 - y1) * getHeight();

    for (int i=1;i<nsamples;i++)
        dy[i]=(y[i+1] - y[i-1]) / 2;

    dy[0]=-1.5f*y[0] + 2.0f*y[1] - 0.5f*y[2];
    dy[nsamples]=1.5f*y[nsamples] - 2.0f*y[nsamples-1] + 0.5f*y[nsamples-2];

    cairo_move_to(cr, 0.0, y[0]);

    for (int i=0;i<nsamples;i++)
        cairo_curve_to(cr,
            (i+0.333333f)/nsamples*getWidth(), y[i] + dy[i]/3,
            (i+0.666667f)/nsamples*getWidth(), y[i+1] - dy[i+1]/3,
            (i+1.000000f)/nsamples*getWidth(), y[i+1]);

    cairo_stroke(cr);
}


void GraphDisplay::plot(cairo_t* cr, float x0, float x1, float y0, float y1, std::function<std::pair<float, float>(float)> fn, int nsamples)
{
    float xs=x0;
    auto [ys, ds]=fn(xs);

    cairo_move_to(cr, 0.0f, (ys-y1)/(y0-y1)*getHeight());

    for (int i=1;i<=nsamples;i++) {
        float t=(float) i/nsamples;
        float xt=x0*(1.0f-t) + x1*t;
        auto [yt, dt]=fn(xt);

        cairo_curve_to(cr, 
            (t-0.666667f/nsamples)*getWidth(), (ys+ds*(x1-x0)/nsamples/3-y1)/(y0-y1)*getHeight(),
            (t-0.333333f/nsamples)*getWidth(), (yt-dt*(x1-x0)/nsamples/3-y1)/(y0-y1)*getHeight(),
            t*getWidth(), (yt-y1)/(y0-y1)*getHeight());

        xs=xt;
        ys=yt;
        ds=dt;
    }

    cairo_stroke(cr);
}

}
