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

#include "PluginOpal.h"

START_NAMESPACE_DISTRHO


class DistrhoPluginOpal::Delay {
    int     length;
    float*  buffer;

    int     wrptr=0;

public:
    Delay(int length):length(length)
    {
        buffer=new float[length];

        for (int i=0;i<length;i++)
            buffer[i]=0.0f;
    }

    ~Delay()
    {
        delete[] buffer;
    }

    void put(float value)
    {
        buffer[wrptr++]=value;
        if (wrptr==length)
            wrptr=0;
    }

    float operator()(float delay) const
    {
        int delay_int=(int) floorf(delay);
        float t=delay - delay_int;
        
        int pos=wrptr - delay_int;
        if (pos<0)
            pos+=length;

        // FIXME: implement some better interpolation
        if (pos==0)
            return buffer[0]*(1.0f-t) + buffer[length-1]*t;
        else
            return buffer[pos]*(1.0f-t) + buffer[pos-1]*t;
    }
};


class DistrhoPluginOpal::BSplineNoise {
    float   coeffs[4] {};
    float   phase=0.0f;

public:
    float operator()(float freq)
    {
        const float t=phase;

        const float val=coeffs[0]*(1-t)*(1-t)*(1-t)/6 +
                        coeffs[1]*(3*t*t*t - 6*t*t + 4)/6 +
                        coeffs[2]*(-3*t*t*t + 3*t*t + 3*t + 1)/6 +
                        coeffs[3]*t*t*t/6;

        phase+=freq;
        if (phase>=1.0f) {
            phase-=1.0f;
            
            coeffs[0]=coeffs[1];
            coeffs[1]=coeffs[2];
            coeffs[2]=coeffs[3];
            coeffs[3]=ldexpf(rand()&0xfffff, -20);
        }

        return val;
    }
};


DistrhoPluginOpal::DistrhoPluginOpal():Plugin(NUM_PARAMETERS, 0, 0)
{
    deactivate();
}


DistrhoPluginOpal::~DistrhoPluginOpal()
{
}


void DistrhoPluginOpal::initAudioPort(bool input, uint32_t index, AudioPort& port)
{
    port.groupId = kPortGroupMono;

    Plugin::initAudioPort(input, index, port);
}


void DistrhoPluginOpal::initParameter(uint32_t index, Parameter& parameter)
{
    switch (index) {
    case PARAM_NUMVOICES:
        parameter.hints      = kParameterIsInteger;
        parameter.name       = "Voice Cnt";
        parameter.symbol     = "numvoices";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 1.0f;
        parameter.ranges.max = 8.0f;
        break;
    case PARAM_DEPTH:
        parameter.hints      = kParameterIsAutomatable;
        parameter.name       = "Depth";
        parameter.symbol     = "depth";
        parameter.ranges.def = 10.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 100.0f;
        break;
    case PARAM_FREQUENCY:
        parameter.hints      = kParameterIsAutomatable | kParameterIsLogarithmic;
        parameter.name       = "Frequency";
        parameter.symbol     = "freq";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.1f;
        parameter.ranges.max = 10.0f;
        break;
    }
}


float DistrhoPluginOpal::getParameterValue(uint32_t index) const
{
    switch (index) {
    case PARAM_NUMVOICES:
        return numvoices;
    case PARAM_DEPTH:
        return depth;
    case PARAM_FREQUENCY:
        return frequency;
    default:
        return 0.0;
    }
}


void  DistrhoPluginOpal::setParameterValue(uint32_t index, float value)
{
    switch (index) {
    case PARAM_NUMVOICES:
        numvoices=(int) value;
        break;
    case PARAM_DEPTH:
        depth=value;
        break;
    case PARAM_FREQUENCY:
        frequency=value;
        break;
    }
}


void DistrhoPluginOpal::activate()
{
    delay=new Delay(lrint(getSampleRate()*1.5));
    noise=new BSplineNoise[8];
}


void DistrhoPluginOpal::deactivate()
{
    delete delay;
    delay=nullptr;

    delete[] noise;
    noise=nullptr;
}


void DistrhoPluginOpal::run(const float** inputs, float** outputs, uint32_t frames)
{
    float maxoffset=(float) (depth*getSampleRate()/1000);
    float freq=(float) (frequency / getSampleRate());

    for (uint32_t i=0;i<frames;i++) {
        delay->put(inputs[0][i]);

        float result=0.0f;
        for (int j=0;j<numvoices;j++)
            result+=(*delay)(maxoffset * noise[j](freq));

        outputs[0][i]=result / numvoices;
    }
}


Plugin* createPlugin()
{
    return new DistrhoPluginOpal();
}



END_NAMESPACE_DISTRHO
