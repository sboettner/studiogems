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
#include <spinbox.h>
#include "PluginSapphire.h"
#include "UISapphire.h"

START_NAMESPACE_DISTRHO

using namespace StudioGemsUI;

class DistrhoUISapphire::SpectrumPanel:public RaisedPanel, KnobEventHandler::Callback, public SpinBox::Callback {
public:
    SpectrumPanel(DistrhoUISapphire*, int x0, int y0);

    void parameterChanged(uint32_t index, float value);

protected:
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

    void value_changed(SubWidget*, int) override;
    void value_change_begin(SubWidget*, int) override;
    void value_change_end(SubWidget*, int) override;

private:
    class SpectrumView;

    static const char* display_func_power_of_two(int);

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

    ScopedPointer<SpinBox>          spin_random;
    ScopedPointer<SpinBox>          spin_harmonics;
    ScopedPointer<SpinBox>          spin_periods;
};


class DistrhoUISapphire::SpectrumPanel::SpectrumView:public GraphDisplay {
public:
    SpectrumView(Widget* parent, uint x0, uint y0);

    void set_randomseed(unsigned int);
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
    unsigned int    randomseed=0;

    float   brightness=0.0f;
    float   falloff=1.0f;
    
    float   two_factor=1.0f;
    float   three_factor=1.0f;
    float   five_factor=1.0f;
    float   seven_factor=1.0f;
    float   higher_factor=1.0f;
};


DistrhoUISapphire::SpectrumPanel::SpectrumView::SpectrumView(Widget* parent, uint x0, uint y0):
    GraphDisplay(parent, x0, y0, 1600, 256)
{
}


void DistrhoUISapphire::SpectrumPanel::SpectrumView::set_randomseed(unsigned int r)
{
    randomseed=r;
    repaint();
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
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

    cairo_set_line_width(cr, 3.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    float scale=1/sqrtf(beta(brightness*2+1, 2*falloff-1));

    XorshiftRNG randomharmonic(randomseed*40507U);

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

        if (randomseed)
            y*=sqrtf(-logf(ldexpf((randomharmonic()&0xfffff)+1, -20)));

        if (y<=0) continue;

        cairo_move_to(cr, x*25.0, 256.0);
        cairo_line_to(cr, x*25.0, 256.0*(1.0f-y));
        cairo_stroke(cr);
    }

    cairo_set_source_rgb(cr, 0.4, 0.7, 1.0);
    plot(cr, 0.0f, 64.0f, 0.0f, 1.0f, [a=brightness, b=brightness+falloff, scale](float x) {
        return scale * powf(0.01f+x, a) * powf(1.0f+x, -b);
    }, 128);
}


DistrhoUISapphire::SpectrumPanel::SpectrumPanel(DistrhoUISapphire* mainwnd, int x0, int y0):
    RaisedPanel(mainwnd, x0, y0, 1664, 512),
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

    spin_harmonics=new SpinBox(this, x0+1344, y0+344, 256, 32);
    spin_harmonics->setId(DistrhoPluginSapphire::PARAM_HARMONICS);
    spin_harmonics->set_label("Harmonics");
    spin_harmonics->set_display_func(display_func_power_of_two);
    spin_harmonics->set_bounds(0, 8);
    spin_harmonics->set_callback(this);

    spin_periods=new SpinBox(this, x0+1344, y0+400, 256, 32);
    spin_periods->setId(DistrhoPluginSapphire::PARAM_PERIODS);
    spin_periods->set_label("Periods");
    spin_periods->set_display_func(display_func_power_of_two);
    spin_periods->set_bounds(0, 12);
    spin_periods->set_callback(this);

    spin_random=new SpinBox(this, x0+1344, y0+456, 256, 32);
    spin_random->setId(DistrhoPluginSapphire::PARAM_RANDOMSEED);
    spin_random->set_label("Random");
    spin_random->set_bounds(0, 65535);
    spin_random->set_callback(this);
}


const char* DistrhoUISapphire::SpectrumPanel::display_func_power_of_two(int val)
{
    static char buf[64];
    sprintf(buf, "%d", 1<<val);
    return buf;
}


void DistrhoUISapphire::SpectrumPanel::parameterChanged(uint32_t index, float value)
{
    switch (index) {
    case DistrhoPluginSapphire::PARAM_HARMONICS:
        spin_harmonics->set_value((int) value);
        break;
    case DistrhoPluginSapphire::PARAM_PERIODS:
        spin_periods->set_value((int) value);
        break;
    case DistrhoPluginSapphire::PARAM_RANDOMSEED:
        spectrumview->set_randomseed((unsigned int) value);
        spin_random->set_value((int) value);
        break;
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


void DistrhoUISapphire::SpectrumPanel::value_change_begin(SubWidget* widget, int value)
{
    mainwnd->editParameter(widget->getId(), true);
}


void DistrhoUISapphire::SpectrumPanel::value_change_end(SubWidget* widget, int value)
{
    mainwnd->editParameter(widget->getId(), false);
}


void DistrhoUISapphire::SpectrumPanel::value_changed(SubWidget* widget, int value)
{
    mainwnd->setParameterValue(widget->getId(), (float) value);

    parameterChanged(widget->getId(), (float) value);
}


class DistrhoUISapphire::EnvelopePanel:public RaisedPanel, KnobEventHandler::Callback {
public:
    EnvelopePanel(DistrhoUISapphire*, int x0, int y0, const char* label, uint32_t parambase);

    void parameterChanged(uint32_t index, float value);

protected:
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

private:
    DistrhoUISapphire*              mainwnd;

    uint32_t                        parambase;

    ScopedPointer<TextLabel>        header;
    ScopedPointer<Knob>             knob_attack;
    ScopedPointer<Knob>             knob_decay;
    ScopedPointer<Knob>             knob_sustain;
    ScopedPointer<Knob>             knob_release;
    ScopedPointer<Knob>             knob_keyfollow;
};


DistrhoUISapphire::EnvelopePanel::EnvelopePanel(DistrhoUISapphire* mainwnd, int x0, int y0, const char* label, uint32_t parambase):
    RaisedPanel(mainwnd, x0, y0, 512, 176),
    mainwnd(mainwnd),
    parambase(parambase)
{
    header=new TextLabel(this, x0+12, y0+12, 480, 32, 8);
    header->set_color(Color(0.6f, 0.8f, 1.0f));
    header->set_text(label);

    knob_attack=new Knob(this, Knob::Size::SMALL, x0+16, y0+48, 96, 120);
    knob_attack->set_name("Attack");
    knob_attack->setRange(0.1f, 1000.0f);
    knob_attack->setId(parambase);
    knob_attack->setCallback(this);

    knob_decay=new Knob(this, Knob::Size::SMALL, x0+112, y0+48, 96, 120);
    knob_decay->set_name("Decay");
    knob_decay->setRange(0.1f, 1000.0f);
    knob_decay->setId(parambase+1);
    knob_decay->setCallback(this);

    knob_sustain=new Knob(this, Knob::Size::SMALL, x0+208, y0+48, 96, 120);
    knob_sustain->set_name("Sustain");
    knob_sustain->setRange(0.0f, 1.0f);
    knob_sustain->setId(parambase+2);
    knob_sustain->setCallback(this);

    knob_release=new Knob(this, Knob::Size::SMALL, x0+304, y0+48, 96, 120);
    knob_release->set_name("Release");
    knob_release->setRange(0.1f, 1000.0f);
    knob_release->setId(parambase+3);
    knob_release->setCallback(this);

    knob_keyfollow=new Knob(this, Knob::Size::SMALL, x0+400, y0+48, 96, 120);
    knob_keyfollow->set_name("Key Follow");
    knob_keyfollow->setRange(0.0f, 1.0f);
    knob_keyfollow->setId(parambase+4);
    knob_keyfollow->setCallback(this);
}


void DistrhoUISapphire::EnvelopePanel::parameterChanged(uint32_t index, float value)
{
    switch (index - parambase) {
    case 0:
        knob_attack->setValue(value);
        break;
    case 1:
        knob_decay->setValue(value);
        break;
    case 2:
        knob_sustain->setValue(value);
        break;
    case 3:
        knob_release->setValue(value);
        break;
    case 4:
        knob_keyfollow->setValue(value);
        break;
    }
}


void DistrhoUISapphire::EnvelopePanel::knobDragStarted(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), true);
}


void DistrhoUISapphire::EnvelopePanel::knobDragFinished(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), false);
}


void DistrhoUISapphire::EnvelopePanel::knobValueChanged(SubWidget* widget, float value)
{
    mainwnd->setParameterValue(widget->getId(), value);
}


class DistrhoUISapphire::LFOPanel:public RaisedPanel, KnobEventHandler::Callback {
public:
    LFOPanel(DistrhoUISapphire*, int x0, int y0, uint parambase, const char* label);

    void parameterChanged(uint32_t index, float value);

protected:
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

private:
    DistrhoUISapphire*              mainwnd;
    uint                            parambase;

    ScopedPointer<TextLabel>        header;
    ScopedPointer<Knob>             knob_frequency;
    ScopedPointer<Knob>             knob_depth;
};


DistrhoUISapphire::LFOPanel::LFOPanel(DistrhoUISapphire* mainwnd, int x0, int y0, uint parambase, const char* label):
    RaisedPanel(mainwnd, x0, y0, 224, 176),
    mainwnd(mainwnd),
    parambase(parambase)
{
    header=new TextLabel(this, x0+12, y0+12, 480, 32, 8);
    header->set_color(Color(0.6f, 0.8f, 1.0f));
    header->set_text(label);

    knob_frequency=new Knob(this, Knob::Size::SMALL, x0+16, y0+48, 96, 120);
    knob_frequency->set_name("Frequency");
    knob_frequency->setRange(0.1f, 10.0f);
    knob_frequency->setId(parambase);
    knob_frequency->setCallback(this);

    knob_depth=new Knob(this, Knob::Size::SMALL, x0+112, y0+48, 96, 120);
    knob_depth->set_name("Depth");
    knob_depth->setRange(0.0f, 1.0f);
    knob_depth->setId(parambase+1);
    knob_depth->setCallback(this);
}


void DistrhoUISapphire::LFOPanel::parameterChanged(uint32_t index, float value)
{
    switch (index - parambase) {
    case 0:
        knob_frequency->setValue(value);
        break;
    case 1:
        knob_depth->setValue(value);
        break;
    }
}


void DistrhoUISapphire::LFOPanel::knobDragStarted(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), true);
}


void DistrhoUISapphire::LFOPanel::knobDragFinished(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), false);
}


void DistrhoUISapphire::LFOPanel::knobValueChanged(SubWidget* widget, float value)
{
    mainwnd->setParameterValue(widget->getId(), value);
}


class DistrhoUISapphire::FilterPanel:public RaisedPanel, KnobEventHandler::Callback {
public:
    FilterPanel(DistrhoUISapphire*, int x0, int y0);

    void parameterChanged(uint32_t index, float value);

protected:
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

private:
    DistrhoUISapphire*              mainwnd;

    ScopedPointer<TextLabel>        header;
    ScopedPointer<Knob>             knob_cutoff;
    ScopedPointer<Knob>             knob_spread;
    ScopedPointer<Knob>             knob_envelope;
    ScopedPointer<Knob>             knob_modulation;
    ScopedPointer<Knob>             knob_keyfollow;
    ScopedPointer<Knob>             knob_feedback;
};


DistrhoUISapphire::FilterPanel::FilterPanel(DistrhoUISapphire* mainwnd, int x0, int y0):
    RaisedPanel(mainwnd, x0, y0, 608, 176),
    mainwnd(mainwnd)
{
    header=new TextLabel(this, x0+12, y0+12, 480, 32, 8);
    header->set_color(Color(0.6f, 0.8f, 1.0f));
    header->set_text("Filter");

    knob_cutoff=new Knob(this, Knob::Size::SMALL, x0+16, y0+48, 96, 120);
    knob_cutoff->set_name("Cut-off");
    knob_cutoff->setRange(20.0f, 20000.0f);
    knob_cutoff->setId(DistrhoPluginSapphire::PARAM_FILTER_CUTOFF);
    knob_cutoff->setCallback(this);

    knob_spread=new Knob(this, Knob::Size::SMALL, x0+112, y0+48, 96, 120);
    knob_spread->set_name("Spread");
    knob_spread->setRange(0.0f, 3.0f);
    knob_spread->setId(DistrhoPluginSapphire::PARAM_FILTER_SPREAD);
    knob_spread->setCallback(this);

    knob_envelope=new Knob(this, Knob::Size::SMALL, x0+208, y0+48, 96, 120);
    knob_envelope->set_name("Envelope");
    knob_envelope->setRange(0.0f, 1.0f);
    knob_envelope->setId(DistrhoPluginSapphire::PARAM_FILTER_ENVELOPE);
    knob_envelope->setCallback(this);

    knob_modulation=new Knob(this, Knob::Size::SMALL, x0+304, y0+48, 96, 120);
    knob_modulation->set_name("Modulation");
    knob_modulation->setRange(0.0f, 1.0f);
    knob_modulation->setId(DistrhoPluginSapphire::PARAM_FILTER_MODULATION);
    knob_modulation->setCallback(this);

    knob_keyfollow=new Knob(this, Knob::Size::SMALL, x0+400, y0+48, 96, 120);
    knob_keyfollow->set_name("Key follow");
    knob_keyfollow->setRange(0.0f, 1.0f);
    knob_keyfollow->setId(DistrhoPluginSapphire::PARAM_FILTER_KEYFOLLOW);
    knob_keyfollow->setCallback(this);

    knob_feedback=new Knob(this, Knob::Size::SMALL, x0+496, y0+48, 96, 120);
    knob_feedback->set_name("Feedback");
    knob_feedback->setRange(-1.0f, 4.0f);
    knob_feedback->setId(DistrhoPluginSapphire::PARAM_FILTER_FEEDBACK);
    knob_feedback->setCallback(this);
}


void DistrhoUISapphire::FilterPanel::parameterChanged(uint32_t index, float value)
{
    switch (index) {
    case DistrhoPluginSapphire::PARAM_FILTER_CUTOFF:
        knob_cutoff->setValue(value);
        break;
    case DistrhoPluginSapphire::PARAM_FILTER_SPREAD:
        knob_spread->setValue(value);
        break;
    case DistrhoPluginSapphire::PARAM_FILTER_ENVELOPE:
        knob_envelope->setValue(value);
        break;
    case DistrhoPluginSapphire::PARAM_FILTER_KEYFOLLOW:
        knob_keyfollow->setValue(value);
        break;
    case DistrhoPluginSapphire::PARAM_FILTER_MODULATION:
        knob_modulation->setValue(value);
        break;
    case DistrhoPluginSapphire::PARAM_FILTER_FEEDBACK:
        knob_feedback->setValue(value);
        break;
    }
}


void DistrhoUISapphire::FilterPanel::knobDragStarted(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), true);
}


void DistrhoUISapphire::FilterPanel::knobDragFinished(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), false);
}


void DistrhoUISapphire::FilterPanel::knobValueChanged(SubWidget* widget, float value)
{
    mainwnd->setParameterValue(widget->getId(), value);
}


DistrhoUISapphire::DistrhoUISapphire()
{
    setSize(1696, 976);

    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Black.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Bold.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Medium.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Light.otf");

    title=new TextLabel(this, 12, 12, 384, 32, 8);
    title->set_color(Color(0.6f, 0.8f, 1.0f));
    title->set_text("Sapphire Pad Synth");

    spectrumpanel=new SpectrumPanel(this, 16, 64);

    ampenvpanel=new EnvelopePanel(this, 16, 592, "Amp. Envelope", DistrhoPluginSapphire::PARAM_AMPENV_ATTACK);
    filterenvpanel=new EnvelopePanel(this, 544, 592, "Filter Envelope", DistrhoPluginSapphire::PARAM_FLTENV_ATTACK);
    filterpanel=new FilterPanel(this, 1072, 592);

    amplfopanel=new LFOPanel(this, 16, 784, DistrhoPluginSapphire::PARAM_AMPLFO_FREQUENCY, "Amp. Drift");
    pitchlfopanel=new LFOPanel(this, 256, 784, DistrhoPluginSapphire::PARAM_PITCHLFO_FREQUENCY, "Pitch Drift");
}


DistrhoUISapphire::~DistrhoUISapphire()
{
}


void DistrhoUISapphire::parameterChanged(uint32_t index, float value)
{
    spectrumpanel->parameterChanged(index, value);
    ampenvpanel->parameterChanged(index, value);
    filterenvpanel->parameterChanged(index, value);
    filterpanel->parameterChanged(index, value);
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