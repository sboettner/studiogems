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

#include <fftw3.h>
#include "PluginSapphire.h"

START_NAMESPACE_DISTRHO


DistrhoPluginSapphire::Waveform::Waveform(int length, int period):length(length), period(period)
{
    sample=new double[length];
}


DistrhoPluginSapphire::Waveform::~Waveform()
{
    delete[] sample;
}


struct DistrhoPluginSapphire::Voice {
    int     note;

    double  step;
    double  phase0;
    double  phase1;

    double  energy=0.0;
    double  excitation=1.0;

    Voice(int note, double samplerate):note(note)
    {
        phase0=ldexpf(rand()&0xfffff, -20);
        phase1=ldexpf(rand()&0xfffff, -20);
        step=440.0 * exp((note-69)*M_LN2/12) / 256 / samplerate;
    }

    void note_off()
    {
        excitation=0.0;
    }

    bool terminated() const
    {
        return energy<0.00001 && excitation<0.00001;
    }

    void produce(Waveform* waveform, float& out0, float& out1)
    {
        energy*=0.9999;
        energy+=excitation * 0.0001;

        double s=phase0*waveform->length;
        double t=phase1*waveform->length;

        int s0=(int) floor(s);
        int t0=(int) floor(t);
        s-=s0;
        t-=t0;

        out0+=(waveform->sample[s0]*(1.0-s) + waveform->sample[(s0+1)&(waveform->length-1)]*s) * energy;
        out1+=(waveform->sample[t0]*(1.0-t) + waveform->sample[(t0+1)&(waveform->length-1)]*t) * energy;

        phase0+=step;
        if (phase0>=1)
            phase0-=1;

        phase1+=step;
        if (phase1>=1)
            phase1-=1;
    }
};


DistrhoPluginSapphire::DistrhoPluginSapphire():Plugin(NUM_PARAMETERS, 0, 0)
{
}


DistrhoPluginSapphire::~DistrhoPluginSapphire()
{
    deactivate();
}


void DistrhoPluginSapphire::initAudioPort(bool input, uint32_t index, AudioPort& port)
{
    port.groupId = kPortGroupMono;

    Plugin::initAudioPort(input, index, port);
}


void DistrhoPluginSapphire::initParameter(uint32_t index, Parameter& parameter)
{
    switch (index) {
    case PARAM_BRIGHTNESS:
        parameter.hints      = 0;
        parameter.name       = "Brightness";
        parameter.symbol     = "brightness";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -0.25f;
        parameter.ranges.max = 4.0f;
        break;        
    case PARAM_FALLOFF:
        parameter.hints      = 0;
        parameter.name       = "Fall-off";
        parameter.symbol     = "falloff";
        parameter.ranges.def = 1.0;
        parameter.ranges.min = 0.5f;
        parameter.ranges.max = 4.0f;
        break;        
    case PARAM_TWO_FACTOR:
        parameter.hints      = 0;
        parameter.name       = "Two";
        parameter.symbol     = "two";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_THREE_FACTOR:
        parameter.hints      = 0;
        parameter.name       = "Three";
        parameter.symbol     = "three";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_FIVE_FACTOR:
        parameter.hints      = 0;
        parameter.name       = "Five";
        parameter.symbol     = "five";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_SEVEN_FACTOR:
        parameter.hints      = 0;
        parameter.name       = "Seven";
        parameter.symbol     = "seven";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_HIGHER_FACTOR:
        parameter.hints      = 0;
        parameter.name       = "Higher";
        parameter.symbol     = "higher";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_BANDWIDTH:
        parameter.hints      = 0;
        parameter.name       = "Bandwidth";
        parameter.symbol     = "bw";
        parameter.ranges.def = 10.0f;
        parameter.ranges.min = 0.1f;
        parameter.ranges.max = 100.0f;
        break;
    case PARAM_BANDWIDTH_EXPONENT:
        parameter.hints      = 0;
        parameter.name       = "Bandwidth Exponent";
        parameter.symbol     = "bwexp";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    }
}


float DistrhoPluginSapphire::getParameterValue(uint32_t index) const
{
    switch (index) {
    case PARAM_BRIGHTNESS:
        return brightness;
    case PARAM_FALLOFF:
        return falloff;
    case PARAM_TWO_FACTOR:
        return two_factor;
    case PARAM_THREE_FACTOR:
        return three_factor;
    case PARAM_FIVE_FACTOR:
        return five_factor;
    case PARAM_SEVEN_FACTOR:
        return seven_factor;
    case PARAM_HIGHER_FACTOR:
        return higher_factor;
    case PARAM_BANDWIDTH:
        return bandwidth;
    case PARAM_BANDWIDTH_EXPONENT:
        return bandwidth_exponent;
    default:
        return 0.0;
    }
}


void DistrhoPluginSapphire::setParameterValue(uint32_t index, float value)
{
    switch (index) {
    case PARAM_BRIGHTNESS:
        brightness=value;
        invalidate_waveform();
        break;
    case PARAM_FALLOFF:
        falloff=value;
        invalidate_waveform();
        break;
    case PARAM_TWO_FACTOR:
        two_factor=value;
        invalidate_waveform();
        break;
    case PARAM_THREE_FACTOR:
        three_factor=value;
        invalidate_waveform();
        break;
    case PARAM_FIVE_FACTOR:
        five_factor=value;
        invalidate_waveform();
        break;
    case PARAM_SEVEN_FACTOR:
        seven_factor=value;
        invalidate_waveform();
        break;
    case PARAM_HIGHER_FACTOR:
        higher_factor=value;
        invalidate_waveform();
        break;
    case PARAM_BANDWIDTH:
        bandwidth=value;
        invalidate_waveform();
        break;
    case PARAM_BANDWIDTH_EXPONENT:
        bandwidth_exponent=value;
        invalidate_waveform();
        break;
    }
}


void DistrhoPluginSapphire::activate()
{
    invalidate_waveform();
}


void DistrhoPluginSapphire::deactivate()
{
    delete waveform;
    waveform=nullptr;
}


static double beta(double a, double b)
{
    return exp(lgamma(a) + lgamma(b) - lgamma(a+b));
}


void DistrhoPluginSapphire::invalidate_waveform()
{
    Waveform* newwaveform=new Waveform(65536, 256);

    for (int i=0;i<newwaveform->length;i++)
        newwaveform->sample[i]=0.0;

    double scale=0.25 / sqrt(beta(2*brightness+1, 2*falloff-1));
    
    for (int i=1;i<=64;i++) {
        double x=i-0.5;
        double y=scale * pow(x, brightness) * pow(1.0+x, -brightness-falloff);

        int h=i;
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

        float bw=256.0f * powf((float) i, bandwidth_exponent) * expm1f(M_LN2*bandwidth/1200);

        for (int j=-127;j<=127;j++) {
            float amp=y * (erff((j+0.5f)/bw) - erff((j-0.5f)/bw)) / 2;
            float phase=2*M_PI*ldexpf(rand()&0xfffff, -20);

            newwaveform->sample[                    i*256+j]=amp * cosf(phase);
            newwaveform->sample[newwaveform->length-i*256-j]=amp * sinf(phase);
        }
    }

    fftw_plan ifft=fftw_plan_r2r_1d(newwaveform->length, newwaveform->sample, newwaveform->sample, FFTW_HC2R, FFTW_ESTIMATE);
    fftw_execute(ifft);
    fftw_destroy_plan(ifft);

    Waveform* oldwaveform=waveform;
    waveform=newwaveform;
    delete oldwaveform;
}


void DistrhoPluginSapphire::run(const float**, float** outputs, uint32_t frames, const MidiEvent* midievents, uint32_t nummidievents)
{
    for (uint32_t i=0;i<nummidievents;i++) {
        const auto& ev=midievents[i];

        if ((ev.data[0]&0xf0)==0x90)
            voices.emplace_back(new Voice(ev.data[1], getSampleRate()));

        if ((ev.data[0]&0xf0)==0x80)
            for (auto& v: voices)
                if (v->note==ev.data[1])
                    v->note_off();
    }

    for (uint32_t i=0;i<frames;i++) {
        outputs[0][i]=0.0f;
        outputs[1][i]=0.0f;

        if (!waveform) continue;

        for (auto& v: voices)
            v->produce(waveform, outputs[0][i], outputs[1][i]);
    }

    voices.erase(std::remove_if(voices.begin(), voices.end(), [](const std::unique_ptr<Voice>& voice) { return voice->terminated(); }), voices.end());
}


Plugin* createPlugin()
{
    return new DistrhoPluginSapphire();
}


END_NAMESPACE_DISTRHO
