/*
 * Studio Gems DISTRHO Plugins
 * Copyright (C) 2023 Stefan T. Boettner
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

#include <raisedpanel.h>
#include "PluginAmethyst.h"
#include "UIAmethyst.h"

START_NAMESPACE_DISTRHO

using namespace StudioGemsUI;

class DistrhoUIAmethyst::ExcitationPanel:public RaisedPanel, KnobEventHandler::Callback {
public:
    ExcitationPanel(DistrhoUIAmethyst*, int x0, int y0);

    void parameterChanged(uint32_t index, float value);

protected:
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

private:
    DistrhoUIAmethyst*              mainwnd;

    ScopedPointer<TextLabel>        header;
    ScopedPointer<Knob>             knob_attack;
    ScopedPointer<Knob>             knob_decay;
};


DistrhoUIAmethyst::ExcitationPanel::ExcitationPanel(DistrhoUIAmethyst* mainwnd, int x0, int y0):
    RaisedPanel(mainwnd, x0, y0, 320, 224),
    mainwnd(mainwnd)
{
    header=new TextLabel(this, x0+12, y0+12, 256, 32, 8);
    header->set_color(Color(0.6f, 0.8f, 1.0f));
    header->set_text("Excitation");

    knob_attack=new Knob(this, Knob::Size::MEDIUM, x0+16, y0+48, 128, 160);
    knob_attack->set_name("Attack");
    knob_attack->setRange(0.0f, 1000.0f);
    knob_attack->setId(DistrhoPluginAmethyst::PARAM_ATTACK);
    knob_attack->setCallback(this);

    knob_decay=new Knob(this, Knob::Size::MEDIUM, x0+144, y0+48, 128, 160);    
    knob_decay->set_name("Decay");
    knob_decay->setRange(0.0f, 1000.0f);
    knob_decay->setId(DistrhoPluginAmethyst::PARAM_DECAY);
    knob_decay->setCallback(this);
}


void DistrhoUIAmethyst::ExcitationPanel::parameterChanged(uint32_t index, float value)
{
    switch (index) {
    case DistrhoPluginAmethyst::PARAM_ATTACK:
        knob_attack->setValue(value);
        break;
    case DistrhoPluginAmethyst::PARAM_DECAY:
        knob_decay->setValue(value);
        break;
    }
}


void DistrhoUIAmethyst::ExcitationPanel::knobDragStarted(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), true);
}


void DistrhoUIAmethyst::ExcitationPanel::knobDragFinished(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), false);
}


void DistrhoUIAmethyst::ExcitationPanel::knobValueChanged(SubWidget* widget, float value)
{
    mainwnd->setParameterValue(widget->getId(), value);

    parameterChanged(widget->getId(), value);
}


class DistrhoUIAmethyst::CombFilterPanel:public RaisedPanel, KnobEventHandler::Callback {
public:
    CombFilterPanel(DistrhoUIAmethyst*, int x0, int y0, const char* label, int paramoffset);

    void parameterChanged(uint32_t index, float value);

protected:
    void knobDragStarted(SubWidget* widget) override;
    void knobDragFinished(SubWidget* widget) override;
    void knobValueChanged(SubWidget* widget, float value) override;

private:
    DistrhoUIAmethyst*              mainwnd;
    int                             paramoffset;

    ScopedPointer<TextLabel>        header;
    ScopedPointer<Knob>             knob_tuning;
    ScopedPointer<Knob>             knob_feedforward;
    ScopedPointer<Knob>             knob_feedback;
    ScopedPointer<Knob>             knob_envelope;
};


DistrhoUIAmethyst::CombFilterPanel::CombFilterPanel(DistrhoUIAmethyst* mainwnd, int x0, int y0, const char* label, int paramoffset):
    RaisedPanel(mainwnd, x0, y0, 544, 224),
    mainwnd(mainwnd),
    paramoffset(paramoffset)
{
    header=new TextLabel(this, x0+12, y0+12, 256, 32, 8);
    header->set_color(Color(0.6f, 0.8f, 1.0f));
    header->set_text(label);

    knob_tuning=new Knob(this, Knob::Size::MEDIUM, x0+16, y0+48, 128, 160);
    knob_tuning->set_name("Tuning");
    knob_tuning->setRange(-12.0f, 36.0f);
    knob_tuning->setId(DistrhoPluginAmethyst::PARAM_COMB1_TUNING+paramoffset);
    knob_tuning->setCallback(this);

    knob_feedforward=new Knob(this, Knob::Size::MEDIUM, x0+144, y0+48, 128, 160);
    knob_feedforward->set_name("Feedforward");
    knob_feedforward->setRange(-1.0f, 1.0f);
    knob_feedforward->setId(DistrhoPluginAmethyst::PARAM_COMB1_FEEDFORWARD+paramoffset);
    knob_feedforward->setCallback(this);

    knob_feedback=new Knob(this, Knob::Size::MEDIUM, x0+272, y0+48, 128, 160);
    knob_feedback->set_name("Feedback");
    knob_feedback->setRange(-1.0f, 1.0f);
    knob_feedback->setId(DistrhoPluginAmethyst::PARAM_COMB1_FEEDBACK+paramoffset);
    knob_feedback->setCallback(this);

    knob_envelope=new Knob(this, Knob::Size::MEDIUM, x0+400, y0+48, 128, 160);
    knob_envelope->set_name("Envelope");
    knob_envelope->setRange(0.0f, 1.0f);
    knob_envelope->setId(DistrhoPluginAmethyst::PARAM_COMB1_ENVELOPE+paramoffset);
    knob_envelope->setCallback(this);
}


void DistrhoUIAmethyst::CombFilterPanel::parameterChanged(uint32_t index, float value)
{
    switch (index-paramoffset) {
    case DistrhoPluginAmethyst::PARAM_COMB1_TUNING:
        knob_tuning->setValue(value);
        break;
    case DistrhoPluginAmethyst::PARAM_COMB1_FEEDFORWARD:
        knob_feedforward->setValue(value);
        break;
    case DistrhoPluginAmethyst::PARAM_COMB1_FEEDBACK:
        knob_feedback->setValue(value);
        break;
    case DistrhoPluginAmethyst::PARAM_COMB1_ENVELOPE:
        knob_envelope->setValue(value);
        break;
    }
}


void DistrhoUIAmethyst::CombFilterPanel::knobDragStarted(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), true);
}


void DistrhoUIAmethyst::CombFilterPanel::knobDragFinished(SubWidget* widget)
{
    mainwnd->editParameter(widget->getId(), false);
}


void DistrhoUIAmethyst::CombFilterPanel::knobValueChanged(SubWidget* widget, float value)
{
    mainwnd->setParameterValue(widget->getId(), value);

    parameterChanged(widget->getId(), value);
}


DistrhoUIAmethyst::DistrhoUIAmethyst()
{
    setSize(1536, 784);

    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Black.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Bold.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Medium.otf");
    TextLayout::register_font_file("/home/stb/fonts/orbitron-master/Orbitron Light.otf");

    title=new TextLabel(this, 12, 12, 384, 32, 8);
    title->set_color(Color(0.6f, 0.8f, 1.0f));
    title->set_text("Amethyst Percussion");

    excitationpanel=new ExcitationPanel(this, 16, 64);
    combfilterpanel1=new CombFilterPanel(this, 976, 64, "Comb Filter 1", 0);
    combfilterpanel2=new CombFilterPanel(this, 976, 304, "Comb Filter 2", 4);
}


DistrhoUIAmethyst::~DistrhoUIAmethyst()
{
}


void DistrhoUIAmethyst::parameterChanged(uint32_t index, float value)
{
    excitationpanel->parameterChanged(index, value);
    combfilterpanel1->parameterChanged(index, value);
    combfilterpanel2->parameterChanged(index, value);
}


void DistrhoUIAmethyst::onCairoDisplay(const CairoGraphicsContext& ctx)
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
    return new DistrhoUIAmethyst();
}

END_NAMESPACE_DISTRHO
