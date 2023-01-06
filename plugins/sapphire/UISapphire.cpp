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

#include <functional>
#include <raisedpanel.h>
#include <graphdisplay.h>
#include "PluginSapphire.h"
#include "UISapphire.h"

START_NAMESPACE_DISTRHO

using namespace StudioGemsUI;

class DistrhoUISapphire::SpectrumPanel:public RaisedPanel, KnobEventHandler::Callback {
public:
    SpectrumPanel(DistrhoUISapphire*, int x0, int y0);

    void parameterChanged(uint32_t index, float value);

protected:
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

private:
    class SpectrumView;

    DistrhoUISapphire*              mainwnd;

    ScopedPointer<TextLabel>        header;
    ScopedPointer<SpectrumView>     spectrumview;

    ScopedPointer<Knob>             knob_brightness;
    ScopedPointer<Knob>             knob_falloff;
    
    ScopedPointer<Knob>             knob_two_factor;
    ScopedPointer<Knob>             knob_three_factor;
    ScopedPointer<Knob>             knob_five_factor;
    ScopedPointer<Knob>             knob_seven_factor;
    ScopedPointer<Knob>             knob_higher_factor;

    ScopedPointer<Knob>             knob_bandwidth;
    ScopedPointer<Knob>             knob_bandwidth_exponent;
};


class DistrhoUISapphire::SpectrumPanel::SpectrumView:public GraphDisplay {
public:
    SpectrumView(Widget* parent, uint x0, uint y0);

    void set_brightness(float);
    void set_falloff(float);
    void set_two_factor(float);
    void set_three_factor(float);
    void set_five_factor(float);
    void set_seven_factor(float);
    void set_higher_factor(float);

protected:
    void draw_graph(cairo_t*) override;

private:
    float   brightness=0.0f;
    float   falloff=1.0f;
    
    float   two_factor=1.0f;
    float   three_factor=1.0f;
    float   five_factor=1.0f;
    float   seven_factor=1.0f;
    float   higher_factor=1.0f;
};


DistrhoUISapphire::SpectrumPanel::SpectrumView::SpectrumView(Widget* parent, uint x0, uint y0):
    GraphDisplay(parent, x0, y0, 1536, 256)
{
}


void DistrhoUISapphire::SpectrumPanel::SpectrumView::set_brightness(float b)
{
    brightness=b;
    repaint();
}


void DistrhoUISapphire::SpectrumPanel::SpectrumView::set_falloff(float f)
{
    falloff=f;
    repaint();
}


void DistrhoUISapphire::SpectrumPanel::SpectrumView::set_two_factor(float v)
{
    two_factor=v;
    repaint();
}


void DistrhoUISapphire::SpectrumPanel::SpectrumView::set_three_factor(float v)
{
    three_factor=v;
    repaint();
}


void DistrhoUISapphire::SpectrumPanel::SpectrumView::set_five_factor(float v)
{
    five_factor=v;
    repaint();
}


void DistrhoUISapphire::SpectrumPanel::SpectrumView::set_seven_factor(float v)
{
    seven_factor=v;
    repaint();
}


void DistrhoUISapphire::SpectrumPanel::SpectrumView::set_higher_factor(float v)
{
    higher_factor=v;
    repaint();
}


static float beta(float a, float b)
{
    return expf(lgammaf(a) + lgammaf(b) - lgammaf(a+b));
}


void DistrhoUISapphire::SpectrumPanel::SpectrumView::SpectrumView::draw_graph(cairo_t* cr)
{
    cairo_set_line_width(cr, 3.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    float scale=1/sqrtf(beta(brightness*2+1, 2*falloff-1));

    cairo_set_source_rgb(cr, 1.0, 1.0, 0.2);
    for (int i=0;i<64;i++) {
        float x=i+0.5f;
        float y=scale * powf(x, brightness) * powf(1.0f+x, -brightness-falloff);

        int h=i+1;
        while (!(h&1)) {
            h/=2;
            y*=two_factor;
        }

        while (!(h%3)) {
            h/=3;
            y*=three_factor;
        }

        while (!(h%5)) {
            h/=5;
            y*=five_factor;
        }

        while (!(h%7)) {
            h/=7;
            y*=seven_factor;
        }

        const static int primes_beyond_seven[]={ 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61 };
        for (int p: primes_beyond_seven)
            while (!(h%p)) {
                h/=p;
                y*=higher_factor;
            }

        if (y<=0) continue;

        cairo_move_to(cr, x*24.0, 256.0);
        cairo_line_to(cr, x*24.0, 256.0*(1.0f-y));
        cairo_stroke(cr);
    }

    cairo_set_source_rgb(cr, 0.4, 0.7, 1.0);
    plot(cr, 0.0f, 64.0f, 0.0f, 1.0f, [a=brightness, b=brightness+falloff, scale](float x) {
        return std::pair<float, float>(
            scale * powf(x, a) * powf(1.0f+x, -b),
            scale*a*powf(x, a-1) * powf(1.0f+x, -b) - scale*b*powf(x, a) * powf(1.0f+x, -b-1.0f));
    }, 128);
}


DistrhoUISapphire::SpectrumPanel::SpectrumPanel(DistrhoUISapphire* mainwnd, int x0, int y0):
    RaisedPanel(mainwnd, x0, y0, 1600, 512),
    mainwnd(mainwnd)
{
    header=new TextLabel(this, x0+12, y0+12, 480, 32, 8);
    header->set_color(Color(0.6f, 0.8f, 1.0f));
    header->set_text("Harmonic Spectrum");

    spectrumview=new SpectrumView(this, x0+16, y0+56);

    knob_brightness=new Knob(this, Knob::Size::MEDIUM, x0+16, y0+336, 128, 160);
    knob_brightness->set_name("Brightness");
    knob_brightness->setRange(-0.25f, 4.0f);
    knob_brightness->setId(DistrhoPluginSapphire::PARAM_BRIGHTNESS);
    knob_brightness->setCallback(this);

    knob_falloff=new Knob(this, Knob::Size::MEDIUM, x0+144, y0+336, 128, 160);
    knob_falloff->set_name("Fall-off");
    knob_falloff->setRange(0.5f, 4.0f);
    knob_falloff->setId(DistrhoPluginSapphire::PARAM_FALLOFF);
    knob_falloff->setCallback(this);

    knob_two_factor=new Knob(this, Knob::Size::MEDIUM, x0+336, y0+336, 128, 160);
    knob_two_factor->set_name("Two");
    knob_two_factor->setRange(0.0f, 1.0f);
    knob_two_factor->setId(DistrhoPluginSapphire::PARAM_TWO_FACTOR);
    knob_two_factor->setCallback(this);

    knob_three_factor=new Knob(this, Knob::Size::MEDIUM, x0+464, y0+336, 128, 160);
    knob_three_factor->set_name("Three");
    knob_three_factor->setRange(0.0f, 1.0f);
    knob_three_factor->setId(DistrhoPluginSapphire::PARAM_THREE_FACTOR);
    knob_three_factor->setCallback(this);

    knob_five_factor=new Knob(this, Knob::Size::MEDIUM, x0+592, y0+336, 128, 160);
    knob_five_factor->set_name("Five");
    knob_five_factor->setRange(0.0f, 1.0f);
    knob_five_factor->setId(DistrhoPluginSapphire::PARAM_FIVE_FACTOR);
    knob_five_factor->setCallback(this);

    knob_seven_factor=new Knob(this, Knob::Size::MEDIUM, x0+720, y0+336, 128, 160);
    knob_seven_factor->set_name("Seven");
    knob_seven_factor->setRange(0.0f, 1.0f);
    knob_seven_factor->setId(DistrhoPluginSapphire::PARAM_SEVEN_FACTOR);
    knob_seven_factor->setCallback(this);

    knob_higher_factor=new Knob(this, Knob::Size::MEDIUM, x0+848, y0+336, 128, 160);
    knob_higher_factor->set_name("Higher");
    knob_higher_factor->setRange(0.0f, 1.0f);
    knob_higher_factor->setId(DistrhoPluginSapphire::PARAM_HIGHER_FACTOR);
    knob_higher_factor->setCallback(this);

    knob_bandwidth=new Knob(this, Knob::Size::MEDIUM, x0+1040, y0+336, 128, 160);
    knob_bandwidth->set_name("Bandwidth");
    knob_bandwidth->setRange(0.1f, 100.0f);
    knob_bandwidth->setId(DistrhoPluginSapphire::PARAM_BANDWIDTH);
    knob_bandwidth->setCallback(this);

    knob_bandwidth_exponent=new Knob(this, Knob::Size::MEDIUM, x0+1168, y0+336, 128, 160);
    knob_bandwidth_exponent->set_name("Scaling");
    knob_bandwidth_exponent->setRange(0.0f, 1.0f);
    knob_bandwidth_exponent->setId(DistrhoPluginSapphire::PARAM_BANDWIDTH_EXPONENT);
    knob_bandwidth_exponent->setCallback(this);
}


void DistrhoUISapphire::SpectrumPanel::parameterChanged(uint32_t index, float value)
{
    switch (index) {
    case DistrhoPluginSapphire::PARAM_BRIGHTNESS:
        spectrumview->set_brightness(value);
        knob_brightness->setValue(value, false);
        break;
    case DistrhoPluginSapphire::PARAM_FALLOFF:
        spectrumview->set_falloff(value);
        knob_falloff->setValue(value, false);
        break;
    case DistrhoPluginSapphire::PARAM_TWO_FACTOR:
        spectrumview->set_two_factor(value);
        knob_two_factor->setValue(value, false);
        break;
    case DistrhoPluginSapphire::PARAM_THREE_FACTOR:
        spectrumview->set_three_factor(value);
        knob_three_factor->setValue(value, false);
        break;
    case DistrhoPluginSapphire::PARAM_FIVE_FACTOR:
        spectrumview->set_five_factor(value);
        knob_five_factor->setValue(value, false);
        break;
    case DistrhoPluginSapphire::PARAM_SEVEN_FACTOR:
        spectrumview->set_seven_factor(value);
        knob_seven_factor->setValue(value, false);
        break;
    case DistrhoPluginSapphire::PARAM_HIGHER_FACTOR:
        spectrumview->set_higher_factor(value);
        knob_higher_factor->setValue(value, false);
        break;
    case DistrhoPluginSapphire::PARAM_BANDWIDTH:
        knob_bandwidth->setValue(value, false);
        break;
    case DistrhoPluginSapphire::PARAM_BANDWIDTH_EXPONENT:
        knob_bandwidth_exponent->setValue(value, false);
        break;
    }
}


void DistrhoUISapphire::SpectrumPanel::knobDragStarted(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), true);
}


void DistrhoUISapphire::SpectrumPanel::knobDragFinished(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), false);
}


void DistrhoUISapphire::SpectrumPanel::knobValueChanged(SubWidget* widget, float value)
{
    mainwnd->setParameterValue(widget->getId(), value);

    parameterChanged(widget->getId(), value);
}


class DistrhoUISapphire::ExcitationPanel:public RaisedPanel, KnobEventHandler::Callback {
public:
    ExcitationPanel(DistrhoUISapphire*, int x0, int y0);

    void parameterChanged(uint32_t index, float value);

protected:
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

private:
    DistrhoUISapphire*              mainwnd;

    ScopedPointer<TextLabel>        header;
    ScopedPointer<Knob>             knob_attack;
    ScopedPointer<Knob>             knob_sustain;
    ScopedPointer<Knob>             knob_decay;
    ScopedPointer<Knob>             knob_release;
};


DistrhoUISapphire::ExcitationPanel::ExcitationPanel(DistrhoUISapphire* mainwnd, int x0, int y0):
    RaisedPanel(mainwnd, x0, y0, 416, 176),
    mainwnd(mainwnd)
{
    header=new TextLabel(this, x0+12, y0+12, 480, 32, 8);
    header->set_color(Color(0.6f, 0.8f, 1.0f));
    header->set_text("Envelope");

    knob_attack=new Knob(this, Knob::Size::SMALL, x0+16, y0+48, 96, 120);
    knob_attack->set_name("Attack");
    knob_attack->setRange(0.1f, 1000.0f);
    knob_attack->setId(DistrhoPluginSapphire::PARAM_EXCITATION_ATTACK);
    knob_attack->setCallback(this);

    knob_sustain=new Knob(this, Knob::Size::SMALL, x0+112, y0+48, 96, 120);
    knob_sustain->set_name("Sustain");
    knob_sustain->setRange(0.0f, 1.0f);
    knob_sustain->setId(DistrhoPluginSapphire::PARAM_EXCITATION_SUSTAIN);
    knob_sustain->setCallback(this);

    knob_decay=new Knob(this, Knob::Size::SMALL, x0+208, y0+48, 96, 120);
    knob_decay->set_name("Decay");
    knob_decay->setRange(0.1f, 1000.0f);
    knob_decay->setId(DistrhoPluginSapphire::PARAM_EXCITATION_DECAY);
    knob_decay->setCallback(this);

    knob_release=new Knob(this, Knob::Size::SMALL, x0+304, y0+48, 96, 120);
    knob_release->set_name("Release");
    knob_release->setRange(0.1f, 1000.0f);
    knob_release->setId(DistrhoPluginSapphire::PARAM_EXCITATION_RELEASE);
    knob_release->setCallback(this);
}


void DistrhoUISapphire::ExcitationPanel::parameterChanged(uint32_t index, float value)
{
    switch (index) {
    case DistrhoPluginSapphire::PARAM_EXCITATION_ATTACK:
        knob_attack->setValue(value);
        break;
    case DistrhoPluginSapphire::PARAM_EXCITATION_SUSTAIN:
        knob_sustain->setValue(value);
        break;
    case DistrhoPluginSapphire::PARAM_EXCITATION_DECAY:
        knob_decay->setValue(value);
        break;
    case DistrhoPluginSapphire::PARAM_EXCITATION_RELEASE:
        knob_release->setValue(value);
        break;
    }
}


void DistrhoUISapphire::ExcitationPanel::knobDragStarted(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), true);
}


void DistrhoUISapphire::ExcitationPanel::knobDragFinished(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), false);
}


void DistrhoUISapphire::ExcitationPanel::knobValueChanged(SubWidget* widget, float value)
{
    mainwnd->setParameterValue(widget->getId(), value);
}


DistrhoUISapphire::DistrhoUISapphire()
{
    setSize(1632, 784);

    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Black.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Bold.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Medium.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Light.otf");

    title=new TextLabel(this, 12, 12, 384, 32, 8);
    title->set_color(Color(0.6f, 0.8f, 1.0f));
    title->set_text("Sapphire Pad Synth");

    spectrumpanel=new SpectrumPanel(this, 16, 64);
    excitationpanel=new ExcitationPanel(this, 16, 592);
}


DistrhoUISapphire::~DistrhoUISapphire()
{
}


void DistrhoUISapphire::parameterChanged(uint32_t index, float value)
{
    spectrumpanel->parameterChanged(index, value);
    excitationpanel->parameterChanged(index, value);
}


void DistrhoUISapphire::knobDragStarted(SubWidget* widget)
{
    editParameter(widget->getId(), true);
}


void DistrhoUISapphire::knobDragFinished(SubWidget* widget)
{
    editParameter(widget->getId(), false);
}


void DistrhoUISapphire::knobValueChanged(SubWidget* widget, float value)
{
    setParameterValue(widget->getId(), value);
}


void DistrhoUISapphire::onCairoDisplay(const CairoGraphicsContext& ctx)
{
    cairo_pattern_t* pat=cairo_pattern_create_linear(0.0, 0.0, getWidth(), getHeight());
    cairo_pattern_add_color_stop_rgb(pat, 0.0, 0.0, 0.1, 0.2);
    cairo_pattern_add_color_stop_rgb(pat, 0.5, 0.1, 0.2, 0.4);
    cairo_pattern_add_color_stop_rgb(pat, 1.0, 0.2, 0.4, 0.8);

    cairo_set_source(ctx.handle, pat);
    cairo_paint(ctx.handle);

    cairo_pattern_destroy(pat);
}


UI* createUI()
{
    return new DistrhoUISapphire();
}

END_NAMESPACE_DISTRHO
