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

#include <pango/pangocairo.h>
#include <fontconfig/fontconfig.h>
#include "cairohelper.h"


void cairo_rounded_rectangle(cairo_t* cr, double x0, double y0, double w, double h, double r)
{
    double r4=r/4;

    cairo_move_to(cr, x0+r, y0);
    cairo_line_to(cr, x0+w-r, y0);
    cairo_curve_to(cr, x0+w-r4, y0, x0+w, y0+r4, x0+w, y0+r);
    cairo_line_to(cr, x0+w, y0+h-r);
    cairo_curve_to(cr, x0+w, y0+h-r4, x0+w-r4, x0+h, x0+w-r, y0+h);
    cairo_line_to(cr, x0+r, y0+h);
    cairo_curve_to(cr, x0+r4, y0+h, x0, y0+h-r4, x0, y0+h-r);
    cairo_line_to(cr, x0, y0+r);
    cairo_curve_to(cr, x0, y0+r4, x0+r4, y0, x0+r, y0);
}


void cairo_set_source_color(cairo_t* cr, const DGL_NAMESPACE::Color& color)
{
    cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
}


namespace StudioGemsUI {

ConicPattern::ConicPattern(float radius):radius(radius)
{
    pat=nullptr;
}


ConicPattern::~ConicPattern()
{
    if (pat)
        cairo_pattern_destroy(pat);
}


void ConicPattern::add_stop(float phi, const DGL_NAMESPACE::Color& color)
{
    if (!pat)
        pat=cairo_pattern_create_mesh();
    else if (phi>lastphi) {
        float ux=cosf(lastphi) * radius;
        float uy=sinf(lastphi) * radius;
        float vx=cosf(phi) * radius;
        float vy=sinf(phi) * radius;

        float a=1.33333f*tanf((phi-lastphi)/4);

        cairo_mesh_pattern_begin_patch(pat);
        cairo_mesh_pattern_move_to(pat, 0.0, 0.0);
        cairo_mesh_pattern_line_to(pat, ux, uy);        
        cairo_mesh_pattern_curve_to(pat, ux-uy*a, uy+ux*a, vx+vy*a, vy-vx*a, vx, vy);        
        cairo_mesh_pattern_line_to(pat, 0.0, 0.0);
        cairo_mesh_pattern_set_corner_color_rgba(pat, 0, lastcolor.red, lastcolor.green, lastcolor.blue, lastcolor.alpha);
        cairo_mesh_pattern_set_corner_color_rgba(pat, 1, lastcolor.red, lastcolor.green, lastcolor.blue, lastcolor.alpha);
        cairo_mesh_pattern_set_corner_color_rgba(pat, 2, color.red, color.green, color.blue, color.alpha);
        cairo_mesh_pattern_set_corner_color_rgba(pat, 3, color.red, color.green, color.blue, color.alpha);
        cairo_mesh_pattern_end_patch(pat);
    }

    lastphi=phi;
    lastcolor=color;
}


void ConicPattern::set_center(double x, double y)
{
    cairo_matrix_t mat;
    cairo_matrix_init_identity(&mat);
    cairo_matrix_translate(&mat, -x, -y);
    cairo_pattern_set_matrix(pat, &mat);
}


GlowSurface::GlowSurface(int w, int h):width(w), height(h)
{
    surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    ctx=cairo_create(surface);
}


GlowSurface::~GlowSurface()
{
    cairo_destroy(ctx);
    cairo_surface_destroy(surface);
}


void GlowSurface::clear()
{
    cairo_save(ctx);
    cairo_set_source_rgba(ctx, 0.0, 0.0, 0.0, 0.0);
    cairo_set_operator(ctx, CAIRO_OPERATOR_SOURCE);
    cairo_paint(ctx);
    cairo_restore(ctx);
}


void GlowSurface::glow()
{
    cairo_surface_flush(surface);

    unsigned char* pixels=cairo_image_surface_get_data(surface);

    unsigned char* copy=new unsigned char[width*height*4];
    for (int i=0;i<width*height*4;i++)
        copy[i]=pixels[i];

    for (int i=0;i<width*4;i++)
        blur_line(pixels+i, height, width*4);

    for (int i=0;i<height;i++) {
        blur_line(pixels+i*width*4, width, 4);
        blur_line(pixels+i*width*4+1, width, 4);
        blur_line(pixels+i*width*4+2, width, 4);
        blur_line(pixels+i*width*4+3, width, 4);
    }

    for (int i=0;i<width*height;i++) {
        int alpha=copy[4*i+3];

        for (int j=0;j<4;j++)
            pixels[4*i+j]=(copy[4*i+j]*alpha + pixels[4*i+j]*(255-alpha)) / 255;
    }

    cairo_surface_mark_dirty(surface);

    delete[] copy;
}


void GlowSurface::blur_line(unsigned char* pixels, int length, int stride)
{
    int sum1, sum2, sum3;

    sum1=sum2=sum3=pixels[0];
    for (int i=0;i<length;i++) {
        sum1+=pixels[i*stride];
        sum1/=2;
        sum2+=sum1;
        sum2/=2;
        sum3+=sum2;
        sum3/=2;
        pixels[i*stride]=sum3;
    }

    sum1=sum2=sum3=pixels[(length-1)*stride];
    for (int i=length-1;i>=0;i--) {
        sum1+=pixels[i*stride];
        sum1/=2;
        sum2+=sum1;
        sum2/=2;
        sum3+=sum2;
        sum3/=2;
        pixels[i*stride]=sum3;
    }
}


TextLayout::TextLayout(cairo_t* cr)
{
    font=pango_font_description_new();

    attributes=pango_attr_list_new();

    layout=pango_cairo_create_layout(cr);
    pango_layout_set_attributes(layout, attributes);
}


TextLayout::~TextLayout()
{
    pango_font_description_free(font);
    pango_attr_list_unref(attributes);
    g_object_unref(layout);
}


void TextLayout::add_attribute(const char* name)
{
    pango_attr_list_insert(attributes, pango_attr_font_features_new(name));
}


void TextLayout::set_font(const char* family, int size, PangoWeight weight, PangoStyle style)
{
    pango_font_description_set_family(font, family);
    pango_font_description_set_weight(font, weight);
    pango_font_description_set_style(font, style);
    pango_font_description_set_absolute_size(font, size*PANGO_SCALE);

    pango_layout_set_font_description(layout, font);
}


void TextLayout::set_width(int width)
{
    pango_layout_set_width(layout, width*PANGO_SCALE);
}


void TextLayout::set_alignment(PangoAlignment align)
{
    pango_layout_set_alignment(layout, align);
}


void TextLayout::set_text(const char* text)
{
    pango_layout_set_text(layout, text, -1);
}


void TextLayout::set_textf(const char* fmt, ...)
{
    char buffer[256];

    va_list list;
    va_start(list, fmt);
    int len=vsnprintf(buffer, sizeof(buffer), fmt, list);
    va_end(list);

    pango_layout_set_text(layout, buffer, len);
}


void TextLayout::show(cairo_t* cr)
{
    pango_cairo_show_layout(cr, layout);
}


void TextLayout::register_font_file(const char* filename)
{
    FcConfigAppFontAddFile(FcConfigGetCurrent(), (const FcChar8*) filename);
}

}
