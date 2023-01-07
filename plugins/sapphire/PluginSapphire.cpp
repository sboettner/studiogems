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


class Excitation {
public:
    Excitation(const ExcitationParameters&, float velocity, float delay, float samplerate);

    void note_off(float delay);

    void produce(float* out, int count);

    bool terminated() const
    {
        return !alive && energy<1e-6f;
    }

private:
    float   energy=0.0f;
    float   sustain_energy=0.0f;
    float   latent_energy=0.0f;

    float   attack_rate=0.0f;
    float   decay_rate=0.0f;
    float   sustain_rate=0.0f;
    float   release_rate=0.0f;

    bool    alive=true;
};


Excitation::Excitation(const ExcitationParameters& params, float velocity, float delay, float samplerate)
{
    latent_energy=1.0f;

    attack_rate=-expm1f(-1000.0f/(params.attack*samplerate));
    decay_rate=-expm1f(-1000.0f/(params.decay*samplerate));
    sustain_rate=decay_rate*params.sustain;
    release_rate=-expm1f(-1000.0f/(params.release*samplerate));
}


void Excitation::note_off(float delay)
{
    // FIXME: properly implement delay

    alive=false;
    latent_energy=0.0f;
    sustain_energy=0.0f;
}


void Excitation::produce(float* out, int count)
{
    // FIXME: properly implement delay

    for (int i=0;i<count;i++) {
        if (alive)
            energy*=1.0f - decay_rate;
        else
            energy*=1.0f - release_rate;

        float u=latent_energy*attack_rate;
        latent_energy-=u;
        energy+=u;
        sustain_energy+=u;

        energy+=sustain_energy*sustain_rate;

        out[i]=energy;
    }    
}


class ExcitationUpsampler {
    const int   factor;

public:
    explicit ExcitationUpsampler(int factor);

    void upsample(float* out, const float* in, int count);
};


ExcitationUpsampler::ExcitationUpsampler(int factor):factor(factor)
{
}


void ExcitationUpsampler::upsample(float* out, const float* in, int count)
{
    // FIXME: use a better upsampling filter - this is naive sample and hold

    float v=*in++;
    int avail=factor;
    
    for (int i=0;i<count;i++) {
        if (!avail--) {
            v=*in++;
            avail=factor;
        }

        out[i]=v;
    }
}


class LowpassLadderFilter {
    float   s1=0.0f, s2=0.0f, s3=0.0f, s4=0.0f;

public:
    float operator()(float x, float g, float feedback)
    {
        float G=g / (1.0f+g);
        float S=(s4 + G*(s3 + G*(s2 + G*s1))) * (1.0f-G);
        float y=(x - feedback*S) / (1.0f + feedback*G*G*G*G);

        float v;

        v=(y-s1) * G;
        y=v+s1;
        s1=y+v;

        v=(y-s2) * G;
        y=v+s2;
        s2=y+v;

        v=(y-s3) * G;
        y=v+s3;
        s3=y+v;

        v=(y-s4) * G;
        y=v+s4;
        s4=y+v;

        return y;
    }
};


struct DistrhoPluginSapphire::Voice {
    const DistrhoPluginSapphire&    plugin;
    int     note;

    double  step;
    double  phase0;
    double  phase1;

    LowpassLadderFilter lp0, lp1;

    Excitation          excitation;
    ExcitationUpsampler excitation_upsampler;

    Voice(const DistrhoPluginSapphire& plugin, int note, float velocity, float delay, double samplerate):
        plugin(plugin),
        note(note),
        excitation(plugin.excitation, velocity, delay/16, (float) samplerate/16),
        excitation_upsampler(16)
    {
        phase0=ldexpf(rand()&0xfffff, -20);
        phase1=ldexpf(rand()&0xfffff, -20);
        step=440.0 * exp((note-69)*M_LN2/12) / 256 / samplerate;
    }

    void note_off()
    {
        excitation.note_off(0.0f);
    }

    bool terminated() const
    {
        return excitation.terminated();
    }

    void produce(Waveform* waveform, float* out0, float* out1, int count);
};


void DistrhoPluginSapphire::Voice::produce(Waveform* waveform, float* out0, float* out1, int count)
{
    float* excbuf=(float*) alloca(count*sizeof(float)/16);
    float* excbuf_upsampled=(float*) alloca(count*sizeof(float));

    excitation.produce(excbuf, count/16);
    excitation_upsampler.upsample(excbuf_upsampled, excbuf, count);

    for (int i=0;i<count;i++) {
        double s=phase0*waveform->length;
        double t=phase1*waveform->length;

        int s0=(int) floor(s);
        int t0=(int) floor(t);
        s-=s0;
        t-=t0;

        const float g=tanf(M_PI*std::min(0.49f, plugin.filter.cutoff*powf(excbuf_upsampled[i], plugin.filter.envelope)/(float) plugin.getSampleRate()));

        out0[i]+=lp0((waveform->sample[s0]*(1.0-s) + waveform->sample[(s0+1)&(waveform->length-1)]*s) * excbuf_upsampled[i], g, plugin.filter.feedback);
        out1[i]+=lp1((waveform->sample[t0]*(1.0-t) + waveform->sample[(t0+1)&(waveform->length-1)]*t) * excbuf_upsampled[i], g, plugin.filter.feedback);

        phase0+=step;
        if (phase0>=1)
            phase0-=1;

        phase1+=step;
        if (phase1>=1)
            phase1-=1;
    }
}


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
    case PARAM_EXCITATION_ATTACK:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Attack";
        parameter.symbol     = "attack";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.1f;
        parameter.ranges.max = 1000.0f;
        break;
    case PARAM_EXCITATION_DECAY:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Decay";
        parameter.symbol     = "decay";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.1f;
        parameter.ranges.max = 1000.0f;
        break;
    case PARAM_EXCITATION_SUSTAIN:
        parameter.hints      = 0;
        parameter.name       = "Sustain";
        parameter.symbol     = "sustain";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_EXCITATION_RELEASE:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Release";
        parameter.symbol     = "release";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.1f;
        parameter.ranges.max = 1000.0f;
        break;
    case PARAM_FILTER_CUTOFF:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Filter Cut-off";
        parameter.symbol     = "filtercutoff";
        parameter.ranges.def = 24000.0f;
        parameter.ranges.min = 10.0f;
        parameter.ranges.max = 24000.0f;
        break;
    case PARAM_FILTER_ENVELOPE:
        parameter.hints      = 0;
        parameter.name       = "Filter Envelope";
        parameter.symbol     = "filterenv";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0;
        parameter.ranges.max = 2.0f;
        break;
    case PARAM_FILTER_LFO:
        parameter.hints      = 0;
        parameter.name       = "Filter LFO";
        parameter.symbol     = "filterlfo";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0;
        parameter.ranges.max = 2.0f;
        break;
    case PARAM_FILTER_MODULATION:
        parameter.hints      = 0;
        parameter.name       = "Filter Modulation";
        parameter.symbol     = "filtermod";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0;
        parameter.ranges.max = 2.0f;
        break;
    case PARAM_FILTER_FEEDBACK:
        parameter.hints      = 0;
        parameter.name       = "Filter Feedback";
        parameter.symbol     = "filterfb";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -1.0f;
        parameter.ranges.max = 4.0f;
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
    case PARAM_EXCITATION_ATTACK:
        return excitation.attack;
    case PARAM_EXCITATION_DECAY:
        return excitation.decay;
    case PARAM_EXCITATION_SUSTAIN:
        return excitation.sustain;
    case PARAM_EXCITATION_RELEASE:
        return excitation.release;
    case PARAM_FILTER_CUTOFF:
        return filter.cutoff;
    case PARAM_FILTER_ENVELOPE:
        return filter.envelope;
    case PARAM_FILTER_LFO:
        return filter.lfo;
    case PARAM_FILTER_MODULATION:
        return filter.modulation;
    case PARAM_FILTER_FEEDBACK:
        return filter.feedback;
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
    case PARAM_EXCITATION_ATTACK:
        excitation.attack=value;
        break;
    case PARAM_EXCITATION_DECAY:
        excitation.decay=value;
        break;
    case PARAM_EXCITATION_SUSTAIN:
        excitation.sustain=value;
        break;
    case PARAM_EXCITATION_RELEASE:
        excitation.release=value;
        break;
    case PARAM_FILTER_CUTOFF:
        filter.cutoff=value;
        break;
    case PARAM_FILTER_ENVELOPE:
        filter.envelope=value;
        break;
    case PARAM_FILTER_LFO:
        filter.lfo=value;
        break;
    case PARAM_FILTER_MODULATION:
        filter.modulation=value;
        break;
    case PARAM_FILTER_FEEDBACK:
        filter.feedback=value;
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
            voices.emplace_back(new Voice(*this, ev.data[1], ev.data[2]/127.0f, ev.frame, getSampleRate()));

        if ((ev.data[0]&0xf0)==0x80)
            for (auto& v: voices)
                if (v->note==ev.data[1])
                    v->note_off();
    }

    for (uint32_t i=0;i<frames;i++) {
        outputs[0][i]=0.0f;
        outputs[1][i]=0.0f;
    }

    if (waveform)
        for (auto& v: voices)
            v->produce(waveform, outputs[0], outputs[1], frames);

    voices.erase(std::remove_if(voices.begin(), voices.end(), [](const std::unique_ptr<Voice>& voice) { return voice->terminated(); }), voices.end());
}


Plugin* createPlugin()
{
    return new DistrhoPluginSapphire();
}


END_NAMESPACE_DISTRHO
