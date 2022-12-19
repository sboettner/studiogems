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
#include "PluginOnyx.h"
#include "UIOnyx.h"
#include <cairohelper.h>
#include <graphdisplay.h>

START_NAMESPACE_DISTRHO

using namespace StudioGemsUI;

class WaveformDisplay:public GraphDisplay {
public:
    WaveformDisplay(Widget* parent, uint x0, uint y0, uint width, uint height);

    void set_color(const Color&);

    void set_waveform(DistrhoPluginOnyx::waveform_t);
    void set_parameter(float);

protected:
    void draw_graph(cairo_t*) override;

private:
    Color       color;

    DistrhoPluginOnyx::waveform_t   waveform=DistrhoPluginOnyx::WAVEFORM_SINE;
    float                           param=0.0f;
};


WaveformDisplay::WaveformDisplay(Widget* parent, uint x0, uint y0, uint width, uint height):GraphDisplay(parent, x0, y0, width, height)
{
}


void WaveformDisplay::set_color(const Color& col)
{
    color=col;
}


void WaveformDisplay::set_waveform(DistrhoPluginOnyx::waveform_t wf)
{
    waveform=wf;
    repaint();
}


void WaveformDisplay::set_parameter(float p)
{
    param=p;
    repaint();
}


void WaveformDisplay::draw_graph(cairo_t* cr)
{
    cairo_set_source_color(cr, color);
    cairo_set_line_width(cr, 3.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   
    switch (waveform) {
    case DistrhoPluginOnyx::WAVEFORM_SINE: {
        const float factor=2*M_PI*1.01f/(1.01f-param);
        const float last=sinf(factor);

        plot(cr, 0.0f, 1.0f, -1.25f, 1.25f, [factor, last](float t) {
            return std::pair<float, float>(
                sinf(factor*t),
                cosf(factor*t)*factor
            );
        });
        break;
    }
    case DistrhoPluginOnyx::WAVEFORM_TRIANGLE:
        cairo_move_to(cr, 0.0, getHeight()*0.875);
        cairo_line_to(cr, getWidth()*param, getHeight()*0.125);
        cairo_line_to(cr, getWidth(), getHeight()*0.875);
        cairo_stroke(cr);

        /*cairo_set_source_rgb(cr, 1.0, 0.6, 0.8);
        plot(cr, 0.0f, 1.0f, -0.125f, 0.125f, [this](float x) {
            float a=param;

            if (x<a) {
                float v=2*x-a;
                return std::pair<float, float>(
                    v*v*v/a/24 + v*(a-2)/24,
                    v*v/a/4 + (a-2)/12
                );
            }
            else {
                float v=1+a-2*x;
                return std::pair<float, float>(
                    v*v*v/(1-a)/24 - v*(a+1)/24,
                    -v*v/(1-a)/4 + (a+1)/12
                );
            }
        });*/
        break;
    case DistrhoPluginOnyx::WAVEFORM_PULSE:
        cairo_move_to(cr, 0.0, getHeight()*0.5);
        cairo_line_to(cr, 0.0, getHeight()*(0.5-0.4*(1.0-param)));
        cairo_line_to(cr, getWidth()*param, getHeight()*(0.5-0.4*(1.0-param)));
        cairo_line_to(cr, getWidth()*param, getHeight()*(0.5+0.4*param));
        cairo_line_to(cr, getWidth(), getHeight()*(0.5+0.4*param));
        cairo_line_to(cr, getWidth(), getHeight()*0.5);
        cairo_stroke(cr);

        /*cairo_set_source_rgb(cr, 1.0, 0.6, 0.8);
        plot(cr, 0.0f, 1.0f, -0.125f, 0.125f, [this](float x) {
            float a=param;

            if (x<a) {
                float v=2*x-a;
                return std::pair<float, float>(
                    v*v*(1-a)/4 + (a-2)*a*(1-a)/12,
                    v*(1-a)
                );
            }
            else {
                float v=1+a-2*x;
                return std::pair<float, float>(
                    -v*v*a/4 + (a+1)*a*(1-a)/12,
                    v*a
                );
            }
        });*/
        break;
    }
}


class OscillatorPanel:public RaisedPanel, KnobEventHandler::Callback {
public:
    OscillatorPanel(DistrhoUIOnyx*, int x0, int y0, int paramindex);

    void set_title(const char*);
    void set_color(const Color&);

    void parameterChanged(uint32_t index, float value);

protected:
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

private:
    DistrhoUIOnyx*                  mainwnd;
    int                             paramindex;

    ScopedPointer<TextLabel>        header;
    ScopedPointer<WaveformDisplay>  wave;
    ScopedPointer<Knob>             knob_waveform;
    ScopedPointer<Knob>             knob_shape;
    ScopedPointer<Knob>             knob_frequency;
    ScopedPointer<Knob>             knob_amplitude;
    ScopedPointer<Knob>             knob_excitation;
    ScopedPointer<Knob>             knob_lfo;
};


OscillatorPanel::OscillatorPanel(DistrhoUIOnyx* mainwnd, int x0, int y0, int paramindex):
    RaisedPanel(mainwnd, x0, y0, 576, 320),
    mainwnd(mainwnd),
    paramindex(paramindex)
{
    header=new TextLabel(this, x0+12, y0+12, 480, 32, 8);
    header->set_color(Color(0.6f, 0.8f, 1.0f));

    wave=new WaveformDisplay(this, x0+16, y0+48, 256, 256);

    knob_waveform=new Knob(this, Knob::Size::SMALL, x0+280, y0+48, 96, 120);
    knob_waveform->set_name("Waveform");
    knob_waveform->setRange(0.0f, 2.0f);
    knob_waveform->setStep(1.0f);
    knob_waveform->setCallback(this);
    knob_waveform->setId(DistrhoPluginOnyx::PARAM_OSCA_WAVEFORM + paramindex);

    knob_shape=new Knob(this, Knob::Size::SMALL, x0+376, y0+48, 96, 120);
    knob_shape->set_name("Shape");
    knob_shape->setRange(0.0f, 1.0f);
    knob_shape->setCallback(this);
    knob_shape->setId(DistrhoPluginOnyx::PARAM_OSCA_SHAPE + paramindex);

    knob_frequency=new Knob(this, Knob::Size::SMALL, x0+472, y0+48, 96, 120);
    knob_frequency->set_name("Frequency");
    knob_frequency->setRange(0.25f, 8.0f);
    knob_frequency->setCallback(this);
    knob_frequency->setId(DistrhoPluginOnyx::PARAM_OSCA_FREQUENCY + paramindex);

    knob_amplitude=new Knob(this, Knob::Size::SMALL, x0+280, y0+192, 96, 120);
    knob_amplitude->set_name("Amplitude");
    knob_amplitude->setRange(0.0f, 8.0f);
    knob_amplitude->setCallback(this);
    knob_amplitude->setId(DistrhoPluginOnyx::PARAM_OSCA_AMPLITUDE + paramindex);

    knob_excitation=new Knob(this, Knob::Size::SMALL, x0+376, y0+192, 96, 120);
    knob_excitation->set_name("Excitation");
    knob_excitation->setRange(0.0f, 4.0f);
    knob_excitation->setCallback(this);
    knob_excitation->setId(DistrhoPluginOnyx::PARAM_OSCA_EXCITATION + paramindex);

    knob_lfo=new Knob(this, Knob::Size::SMALL, x0+472, y0+192, 96, 120);
    knob_lfo->set_name("LFO");
    knob_lfo->setRange(0.0f, 1.0f);
    knob_lfo->setCallback(this);
    knob_lfo->setId(DistrhoPluginOnyx::PARAM_OSCA_LFO + paramindex);
}


void OscillatorPanel::set_title(const char* title)
{
    header->set_text(title);
}


void OscillatorPanel::set_color(const Color& color)
{
    wave->set_color(color);
    knob_waveform->set_color(color);
    knob_shape->set_color(color);
    knob_frequency->set_color(color);
    knob_amplitude->set_color(color);
    knob_excitation->set_color(color);
    knob_lfo->set_color(color);
}


void OscillatorPanel::parameterChanged(uint32_t index, float value)
{
    switch (index - paramindex) {
    case DistrhoPluginOnyx::PARAM_OSCA_WAVEFORM:
        knob_waveform->setValue(value);
        wave->set_waveform((DistrhoPluginOnyx::waveform_t) value);
        break;
    case DistrhoPluginOnyx::PARAM_OSCA_SHAPE:
        knob_shape->setValue(value);
        wave->set_parameter(value);
        break;
    case DistrhoPluginOnyx::PARAM_OSCA_FREQUENCY:
        knob_frequency->setValue(value);
        break;
    case DistrhoPluginOnyx::PARAM_OSCA_AMPLITUDE:
        knob_amplitude->setValue(value);
        break;
    case DistrhoPluginOnyx::PARAM_OSCA_EXCITATION:
        knob_excitation->setValue(value);
        break;
    case DistrhoPluginOnyx::PARAM_OSCA_LFO:
        knob_lfo->setValue(value);
        break;
    }
}


void OscillatorPanel::knobDragStarted(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), true);
}


void OscillatorPanel::knobDragFinished(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), false);
}


void OscillatorPanel::knobValueChanged(SubWidget* widget, float value)
{
    mainwnd->setParameterValue(widget->getId(), value);

    if (widget->getId()==DistrhoPluginOnyx::PARAM_OSCA_WAVEFORM+paramindex)
        wave->set_waveform((DistrhoPluginOnyx::waveform_t) value);

    if (widget->getId()==DistrhoPluginOnyx::PARAM_OSCA_SHAPE+paramindex)
        wave->set_parameter(value);
}


class ExcitationPanel:public RaisedPanel, KnobEventHandler::Callback {
public:
    ExcitationPanel(DistrhoUIOnyx*, int x0, int y0);

    void parameterChanged(uint32_t index, float value);

protected:
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

private:
    DistrhoUIOnyx*                  mainwnd;

    ScopedPointer<TextLabel>        header;
    ScopedPointer<Knob>             knob_burst;
    ScopedPointer<Knob>             knob_attack;
    ScopedPointer<Knob>             knob_sustain;
    ScopedPointer<Knob>             knob_decay;
    ScopedPointer<Knob>             knob_release;

    ScopedPointer<Knob>             knob_unison_voices;
    ScopedPointer<Knob>             knob_unison_detune;
    ScopedPointer<Knob>             knob_unison_width;
};


ExcitationPanel::ExcitationPanel(DistrhoUIOnyx* mainwnd, int x0, int y0):
    RaisedPanel(mainwnd, x0, y0, 512, 656),
    mainwnd(mainwnd)
{
    header=new TextLabel(this, x0+12, y0+12, 480, 32, 8);
    header->set_color(Color(0.6f, 0.8f, 1.0f));
    header->set_text("Excitation Model");

    knob_burst=new Knob(this, Knob::Size::SMALL, x0+16, y0+48, 96, 120);
    knob_burst->set_name("Burst");
    knob_burst->setRange(0.0f, 1.0f);
    knob_burst->setId(DistrhoPluginOnyx::PARAM_EXCITATION_BURST);
    knob_burst->setCallback(this);

    knob_attack=new Knob(this, Knob::Size::SMALL, x0+112, y0+48, 96, 120);
    knob_attack->set_name("Attack");
    knob_attack->setRange(0.1f, 1000.0f);
    knob_attack->setId(DistrhoPluginOnyx::PARAM_EXCITATION_ATTACK);
    knob_attack->setCallback(this);

    knob_sustain=new Knob(this, Knob::Size::SMALL, x0+208, y0+48, 96, 120);
    knob_sustain->set_name("Sustain");
    knob_sustain->setRange(0.0f, 1.0f);
    knob_sustain->setId(DistrhoPluginOnyx::PARAM_EXCITATION_SUSTAIN);
    knob_sustain->setCallback(this);

    knob_decay=new Knob(this, Knob::Size::SMALL, x0+304, y0+48, 96, 120);
    knob_decay->set_name("Decay");
    knob_decay->setRange(0.1f, 1000.0f);
    knob_decay->setId(DistrhoPluginOnyx::PARAM_EXCITATION_DECAY);
    knob_decay->setCallback(this);

    knob_release=new Knob(this, Knob::Size::SMALL, x0+400, y0+48, 96, 120);
    knob_release->set_name("Release");
    knob_release->setRange(0.1f, 1000.0f);
    knob_release->setId(DistrhoPluginOnyx::PARAM_EXCITATION_RELEASE);
    knob_release->setCallback(this);

    knob_unison_voices=new Knob(this, Knob::Size::SMALL, x0+16, y0+176, 96, 120);
    knob_unison_voices->set_name("U. Voices");
    knob_unison_voices->setRange(1.0f, 10.0f);
    knob_unison_voices->setStep(1.0f);
    knob_unison_voices->setId(DistrhoPluginOnyx::PARAM_UNISON_VOICES);
    knob_unison_voices->setCallback(this);

    knob_unison_detune=new Knob(this, Knob::Size::SMALL, x0+112, y0+176, 96, 120);
    knob_unison_detune->set_name("U. Detune");
    knob_unison_detune->setRange(0.1f, 100.0f);
    knob_unison_detune->setId(DistrhoPluginOnyx::PARAM_UNISON_DETUNE);
    knob_unison_detune->setCallback(this);    

    knob_unison_width=new Knob(this, Knob::Size::SMALL, x0+208, y0+176, 96, 120);
    knob_unison_width->set_name("U. Width");
    knob_unison_width->setRange(0.0f, 1.0f);
    knob_unison_width->setId(DistrhoPluginOnyx::PARAM_UNISON_WIDTH);
    knob_unison_width->setCallback(this);    
}


void ExcitationPanel::parameterChanged(uint32_t index, float value)
{
    switch (index) {
    case DistrhoPluginOnyx::PARAM_EXCITATION_BURST:
        knob_burst->setValue(value);
        break;
    case DistrhoPluginOnyx::PARAM_EXCITATION_ATTACK:
        knob_attack->setValue(value);
        break;
    case DistrhoPluginOnyx::PARAM_EXCITATION_SUSTAIN:
        knob_sustain->setValue(value);
        break;
    case DistrhoPluginOnyx::PARAM_EXCITATION_DECAY:
        knob_decay->setValue(value);
        break;
    case DistrhoPluginOnyx::PARAM_EXCITATION_RELEASE:
        knob_release->setValue(value);
        break;
    case DistrhoPluginOnyx::PARAM_UNISON_VOICES:
        knob_unison_voices->setValue(value);
        break;
    case DistrhoPluginOnyx::PARAM_UNISON_DETUNE:
        knob_unison_detune->setValue(value);
        break;
    case DistrhoPluginOnyx::PARAM_UNISON_WIDTH:
        knob_unison_width->setValue(value);
        break;
    }
}


void ExcitationPanel::knobDragStarted(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), true);
}


void ExcitationPanel::knobDragFinished(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), false);
}


void ExcitationPanel::knobValueChanged(SubWidget* widget, float value)
{
    mainwnd->setParameterValue(widget->getId(), value);
}


DistrhoUIOnyx::DistrhoUIOnyx()
{
    setSize(1728, 736);

    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Black.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Bold.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Medium.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Light.otf");

    oscapanel=new OscillatorPanel(this, 16, 64, 0);
    oscbpanel=new OscillatorPanel(this, 608, 64, 6);
    osccpanel=new OscillatorPanel(this, 16, 400, 12);
    oscdpanel=new OscillatorPanel(this, 608, 400, 18);

    oscapanel->set_title("Oscillator A");
    oscbpanel->set_title("Oscillator B");
    osccpanel->set_title("Oscillator C");
    oscdpanel->set_title("Oscillator D");

    oscapanel->set_color(Color(1.0f, 0.1f, 0.6f));
    oscbpanel->set_color(Color(0.4f, 1.0f, 0.1f));
    osccpanel->set_color(Color(1.0f, 0.8f, 0.1f));
    oscdpanel->set_color(Color(0.1f, 0.7f, 1.0f));

    excpanel=new ExcitationPanel(this, 1200, 64);
}


DistrhoUIOnyx::~DistrhoUIOnyx()
{
}


void DistrhoUIOnyx::parameterChanged(uint32_t index, float value)
{
    oscapanel->parameterChanged(index, value);
    oscbpanel->parameterChanged(index, value);
    osccpanel->parameterChanged(index, value);
    oscdpanel->parameterChanged(index, value);
    excpanel->parameterChanged(index, value);
}


void DistrhoUIOnyx::knobDragStarted(SubWidget* widget)
{
    editParameter(widget->getId(), true);
}


void DistrhoUIOnyx::knobDragFinished(SubWidget* widget)
{
    editParameter(widget->getId(), false);
}


void DistrhoUIOnyx::knobValueChanged(SubWidget* widget, float value)
{
    setParameterValue(widget->getId(), value);
}


void DistrhoUIOnyx::onCairoDisplay(const CairoGraphicsContext& ctx)
{
    cairo_t* cr=ctx.handle;

    cairo_pattern_t* backgnd=cairo_pattern_create_linear(0, 0, getWidth(), getHeight());
    cairo_pattern_add_color_stop_rgb(backgnd, 0.0, 0.05, 0.1, 0.2);
    cairo_pattern_add_color_stop_rgb(backgnd, 1.0, 0.1, 0.2, 0.4);
    cairo_set_source(cr, backgnd);
    cairo_paint(cr);
}

UI* createUI()
{
    return new DistrhoUIOnyx();
}

END_NAMESPACE_DISTRHO
