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

#include "PluginOnyx.h"

START_NAMESPACE_DISTRHO

class SVF {
public:
    SVF(float c1, float c2, float d0, float d1, float d2):c1(c1), c2(c2), d0(d0), d1(d1), d2(d2)
    {
    }

    float operator()(float x)
    {
        x-=z1+z2;
        float out=d0*x + d1*z1 + d2*z2;
        z2+=c2*z1;
        z1+=c1*x;
        return out;
    }

    static SVF biquad(float a0, float a1, float a2, float b1, float b2)
    {
        float c1=b1+2.0f;
        float c2=(1.0f+b1+b2)/c1;
        return SVF(c1, c2, a0, (2.0f*a0+a1)/c1, (a0+a1+a2)/(c1*c2));
    }

private:
    float   z1=0.0f, z2=0.0f;
    float   c1, c2, d0, d1, d2;
};


struct OscillatorWaveform {
    float   table[256];

    OscillatorWaveform(DistrhoPluginOnyx::waveform_t waveform, float shape)
    {
        switch (waveform) {
        case DistrhoPluginOnyx::WAVEFORM_SINE: {
            const float factor=2*M_PI*1.01f/(1.01f-shape);
            const float last=sinf(factor);

            for (int i=0;i<256;i++) {
                float t=i/256.0f;
                table[i]=(sinf(factor*t) - t*last) / (factor*factor);
            }
            break;
        }
        case DistrhoPluginOnyx::WAVEFORM_TRIANGLE:
            for (int i=0;i<256;i++) {
                float t=i/256.0f;
                if (t<shape) {
                    float u=2.0f*t-shape;
                    table[i]=u*u*u/shape/24 + u*(shape-2.0f)/24;
                }
                else {
                    float u=1.0f+shape-2.0f*t;
                    table[i]=u*u*u/(1.0f-shape)/24 - u*(shape+1.0f)/24;
                }
            }
            break;        
        case DistrhoPluginOnyx::WAVEFORM_PULSE:
            for (int i=0;i<256;i++) {
                float t=i/256.0f;
                if (t<shape) {
                    float u=2.0f*t-shape;
                    table[i]=u*u*(1.0f-shape)/4 + (shape-2.0f)*shape*(1.0f-shape)/12;
                }
                else {
                    float u=1.0f+shape-2.0f*t;
                    table[i]=-u*u*shape/4 + (shape+1.0f)*shape*(1.0f-shape)/12;
                }
            }
            break;        
        }
    }

    double eval(double x) const
    {
        x*=256;
        int x0=(int) floor(x);
        x-=x0;

        return table[ x0   &255]*(1-x)*(1-x)*(1-x)/6 +
               table[(x0+1)&255]*(3*x*x*x - 6*x*x + 4)/6 +
               table[(x0+2)&255]*(-3*x*x*x + 3*x*x + 3*x + 1)/6 +
               table[(x0+3)&255]*x*x*x/6;
    }

    double evald(double x) const
    {
        x*=256;
        int x0=(int) floor(x);
        x-=x0;

        return (table[ x0   &255]*-(1-x)*(1-x)/2 +
               table[(x0+1)&255]*(9*x*x - 12*x)/6 +
               table[(x0+2)&255]*(-9*x*x + 6*x + 3)/6 +
               table[(x0+3)&255]*x*x/2) * 256;
    }

    double evaldd(double x) const
    {
        x*=256;
        int x0=(int) floor(x);
        x-=x0;

        return (table[ x0   &255]*(1-x) +
               table[(x0+1)&255]*(18*x - 12)/6 +
               table[(x0+2)&255]*(-18*x + 6)/6 +
               table[(x0+3)&255]*x) * 256 * 256;
    }
};


class Oscillator {
public:
    Oscillator(const OscillatorWaveform& wave, float freq):
        wave(wave),
        integrator(SVF::biquad(1.0f, 0.0f, -1.0f, -2.0f*0.999f, 0.999f*0.999f))
    {
        timestep=freq;
        phasebase=ldexpf(rand()&0xfffff, -20);
        phase0=phasebase-2*timestep;
        phase1=phasebase-timestep;

        val0=wave.eval(phase0);
        val1=wave.eval(phase1);

        dval01=(val0-val1) / (phase0-phase1);
    }

    float get_timestep() const
    {
        return timestep;
    }

    float operator()(float mod=0.0f)
    {
        float phase2=phasebase + timestep*stepcount;
        if (phase2>1.0f) {
            phase0-=1.0f;
            phase1-=1.0f;
            phase2-=1.0f;
            phasebase=phase2;
            stepcount=0;
        }

        stepcount++;

        phase2+=integrator(mod);

        double val2=wave.eval(phase2);

        double dval12=fabsf(phase1-phase2)<1.0f/65536 ? wave.evald((phase1+phase2)/2) : (val1-val2) / (phase1-phase2);

        double val=fabsf(phase0-phase2)<1.0f/65536 ? wave.evaldd((phase0+phase2)/2)/2 : (dval01-dval12) / (phase0-phase2);

        phase0=phase1;
        phase1=phase2;

        val0=val1;
        val1=val2;
        dval01=dval12;

        return (float) val;
    }

private:
    const OscillatorWaveform&   wave;

    float   phasebase=0.0f;
    float   phase0=0.0f;
    float   phase1=0.0f;
    SVF     integrator;

    double  val0=0.0f;
    double  val1=0.0f;
    double  dval01=0.0f;

    float   timestep=0.0f;
    int     stepcount=0;
};


class DistrhoPluginOnyx::Excitation {
public:
    Excitation(const ExcitationParameters& params, float samplerate):samplerate(samplerate)
    {
        latent_energy=params.burst;

        attack_rate=-expm1f(-1000.0f/(params.attack*samplerate));
        decay_rate=-expm1f(-1000.0f/(params.decay*samplerate));
        sustain_rate=decay_rate*params.sustain;
        release_rate=-expm1f(-1000.0f/(params.release*samplerate));

        for (uint i=0;i<bufsize;i++)
            buffer[i]=0.0f;
    }

    void note_off()
    {
        alive=false;
        latent_energy=0.0f;
        sustain_rate=0.0f;
    }

    bool decayed() const
    {
        return !alive && energy<1e-6f;
    }

    float operator[](uint i) const
    {
        return buffer[i&bufmask];
    }

    void advance(uint to)
    {
        while (fillptr<to) {
            if (alive)
                energy*=1.0f - decay_rate;
            else
                energy*=1.0f - release_rate;

            float u=latent_energy*attack_rate;
            latent_energy-=u;
            energy+=u;
            energy+=sustain_rate;

            buffer[fillptr++ & bufmask]=energy;
        }
    }

    uint get_position() const
    {
        return fillptr;
    }

private:
    const static uint bufsize=256;
    const static uint bufmask=bufsize-1;

    float   buffer[bufsize];
    float   samplerate;
    uint    fillptr=0;

    float   energy=0.0f;
    float   latent_energy=0.0f;

    float   attack_rate=0.0f;
    float   decay_rate=0.0f;
    float   sustain_rate=0.0f;
    float   release_rate=0.0f;

    bool    alive=true;
};


class DistrhoPluginOnyx::OscillatorExcitation {
public:
    struct Values {
        float   oscexc[4];
    };

    OscillatorExcitation(const Excitation& excitation, const DistrhoPluginOnyx::OscillatorParameters* params):excitation(excitation), params(params)
    {
        for (int i=0;i<bufsize;i++)
            for (int j=0;j<4;j++)
                buffer[i].oscexc[j]=0.0f;
    }

    const Values& operator[](uint i) const
    {
        return buffer[i&bufmask];
    }

    void advance()
    {
        while (fillptr<excitation.get_position()) {
            float exc=excitation[fillptr];
            float lfo=sinf(fillptr*0.01f);

            for (int i=0;i<4;i++)
                buffer[fillptr&bufmask].oscexc[i]=exc>0 ? expf(logf(exc)*params[i].excitation + lfo*params[i].lfo)*params[i].amplitude : 0.0f;

            fillptr++;
        }
    }

private:
    const static int bufsize=256;
    const static int bufmask=bufsize-1;

    const Excitation&     excitation;
    const OscillatorParameters*  params;

    Values  buffer[bufsize];
    uint    fillptr=0;
};


class DistrhoPluginOnyx::Voice {
public:
    Voice(const DistrhoPluginOnyx& plugin, int note, float samplerate, float detune=0.0f, float voll=1.0f, float volr=1.0f):
        note(note),
        volumeleft(voll),
        volumeright(volr),
        wavea(plugin.oscparams[0].waveform, plugin.oscparams[0].shape),
        waveb(plugin.oscparams[1].waveform, plugin.oscparams[1].shape),
        wavec(plugin.oscparams[2].waveform, plugin.oscparams[2].shape),
        osca(wavea, plugin.oscparams[0].frequency * expf((note-69+detune)*M_LN2/12.0f) * 440.0f / samplerate),
        oscb(waveb, plugin.oscparams[1].frequency * expf((note-69+detune)*M_LN2/12.0f) * 440.0f / samplerate),
        oscc(wavec, plugin.oscparams[2].frequency * expf((note-69+detune)*M_LN2/12.0f) * 440.0f / samplerate),
        excitation(plugin.excparams, samplerate/32),
        oscexc(excitation, plugin.oscparams),
        dcblock(SVF::biquad(1.0f, -2.0f, 1.0f, -2.0f*0.999f, 0.999f*0.999f))
    {
    }

    int get_note() const
    {
        return note;
    }

    bool decayed() const
    {
        return excitation.decayed();
    }

    void note_off()
    {
        note=-1;
        excitation.note_off();
    }

    void produce(float* left, float *right, uint frames)
    {
        excitation.advance((excptr+frames+31)/32);
        oscexc.advance();

        for (uint i=0;i<frames;i++) {
            float t=((excptr&31)+1) / 32.0f;
            float exca=oscexc[excptr/32-3].oscexc[0]*(1-t)*(1-t)*(1-t)/6 +
                       oscexc[excptr/32-2].oscexc[0]*(3*t*t*t - 6*t*t + 4)/6 +
                       oscexc[excptr/32-1].oscexc[0]*(-3*t*t*t + 3*t*t + 3*t + 1)/6 +
                       oscexc[excptr/32  ].oscexc[0]*t*t*t/6;

            float excb=oscexc[excptr/32-3].oscexc[1]*(1-t)*(1-t)*(1-t)/6 +
                       oscexc[excptr/32-2].oscexc[1]*(3*t*t*t - 6*t*t + 4)/6 +
                       oscexc[excptr/32-1].oscexc[1]*(-3*t*t*t + 3*t*t + 3*t + 1)/6 +
                       oscexc[excptr/32  ].oscexc[1]*t*t*t/6;

            float excc=oscexc[excptr/32-3].oscexc[2]*(1-t)*(1-t)*(1-t)/6 +
                       oscexc[excptr/32-2].oscexc[2]*(3*t*t*t - 6*t*t + 4)/6 +
                       oscexc[excptr/32-1].oscexc[2]*(-3*t*t*t + 3*t*t + 3*t + 1)/6 +
                       oscexc[excptr/32  ].oscexc[2]*t*t*t/6;

            float out=dcblock(osca(oscb(oscc() * oscc.get_timestep() * excc) * oscb.get_timestep() * excb) * exca);
            left[i]+=out * volumeleft;
            right[i]+=out * volumeright;

            excptr++;
        }
    }

private:
    int         note;
    float       volumeleft;
    float       volumeright;

    OscillatorWaveform  wavea;
    OscillatorWaveform  waveb;
    OscillatorWaveform  wavec;
    Oscillator  osca;
    Oscillator  oscb;
    Oscillator  oscc;

    Excitation  excitation;
    OscillatorExcitation    oscexc;

    SVF         dcblock;

    uint        excptr=0;
};


class DistrhoPluginOnyx::Downsampler {
public:
    Downsampler();
    ~Downsampler();

    float* get_input_buffer(uint& size);
    void write_output(float* dst, uint size);

private:
    const static uint bufsize=8192;
    const static uint bufmask=bufsize-1;

    float*  buffer;
    uint    fillptr=0;
};


DistrhoPluginOnyx::Downsampler::Downsampler()
{
    buffer=new float[bufsize];

    for (uint i=0;i<bufsize;i++)
        buffer[i]=0.0f;
}


DistrhoPluginOnyx::Downsampler::~Downsampler()
{
    delete[] buffer;
}


float* DistrhoPluginOnyx::Downsampler::get_input_buffer(uint& size)
{
    if (fillptr+size>bufsize)
        size=bufsize-fillptr;
    
    float* tmp=buffer + fillptr;

    fillptr+=size;
    fillptr&=bufmask;

    for (uint i=0;i<size;i++)
        tmp[i]=0.0f;

    return tmp;
}


void DistrhoPluginOnyx::Downsampler::write_output(float* dst, uint size)
{
    // FIXME: implement better lowpass filter
    for (uint i=0;i<size;i++)
        dst[i]=buffer[(fillptr + (i-size)*2 - 2) & bufmask] * 0.125f +
               buffer[(fillptr + (i-size)*2 - 1) & bufmask] * 0.375f +
               buffer[(fillptr + (i-size)*2    ) & bufmask] * 0.375f +
               buffer[(fillptr + (i-size)*2 + 1) & bufmask] * 0.125f;
}


DistrhoPluginOnyx::DistrhoPluginOnyx():Plugin(NUM_PARAMETERS, 0, 0)
{
    deactivate();
}


DistrhoPluginOnyx::~DistrhoPluginOnyx()
{
}


void DistrhoPluginOnyx::initAudioPort(bool input, uint32_t index, AudioPort& port)
{
    port.groupId = kPortGroupMono;

    Plugin::initAudioPort(input, index, port);
}


void DistrhoPluginOnyx::initParameter(uint32_t index, Parameter& parameter)
{
    switch (index) {
    case PARAM_OSCA_WAVEFORM:
        parameter.hints      = kParameterIsInteger;
        parameter.name       = "Osc. A Waveform";
        parameter.symbol     = "waveforma";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 2.0f;
        break;
    case PARAM_OSCA_SHAPE:
        parameter.hints      = 0;
        parameter.name       = "Osc. A Shape";
        parameter.symbol     = "shapea";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_OSCA_FREQUENCY:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Osc. A Frequency";
        parameter.symbol     = "freqa";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.25f;
        parameter.ranges.max = 8.0f;
        break;
    case PARAM_OSCA_AMPLITUDE:
        parameter.hints      = 0;
        parameter.name       = "Osc. A Amplitude";
        parameter.symbol     = "amplitudea";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 8.0f;
        break;
    case PARAM_OSCA_EXCITATION:
        parameter.hints      = 0;
        parameter.name       = "Osc. A Excitation";
        parameter.symbol     = "excitationa";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 4.0f;
        break;
    case PARAM_OSCA_LFO:
        parameter.hints      = 0;
        parameter.name       = "Osc. A LFO";
        parameter.symbol     = "lfoa";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_OSCB_WAVEFORM:
        parameter.hints      = kParameterIsInteger;
        parameter.name       = "Osc. B Waveform";
        parameter.symbol     = "waveformb";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 2.0f;
        break;
    case PARAM_OSCB_SHAPE:
        parameter.hints      = 0;
        parameter.name       = "Osc. B Shape";
        parameter.symbol     = "shapeb";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_OSCB_FREQUENCY:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Osc. B Frequency";
        parameter.symbol     = "freqb";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.25f;
        parameter.ranges.max = 8.0f;
        break;
    case PARAM_OSCB_AMPLITUDE:
        parameter.hints      = 0;
        parameter.name       = "Osc. B Amplitude";
        parameter.symbol     = "amplitudeb";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 8.0f;
        break;
    case PARAM_OSCB_EXCITATION:
        parameter.hints      = 0;
        parameter.name       = "Osc. B Excitation";
        parameter.symbol     = "excitationb";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 4.0f;
        break;
    case PARAM_OSCB_LFO:
        parameter.hints      = 0;
        parameter.name       = "Osc. B LFO";
        parameter.symbol     = "lfob";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_OSCC_WAVEFORM:
        parameter.hints      = kParameterIsInteger;
        parameter.name       = "Osc. C Waveform";
        parameter.symbol     = "waveformc";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 2.0f;
        break;
    case PARAM_OSCC_SHAPE:
        parameter.hints      = 0;
        parameter.name       = "Osc. C Shape";
        parameter.symbol     = "shapec";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_OSCC_FREQUENCY:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Osc. C Frequency";
        parameter.symbol     = "freqc";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.25f;
        parameter.ranges.max = 8.0f;
        break;
    case PARAM_OSCC_AMPLITUDE:
        parameter.hints      = 0;
        parameter.name       = "Osc. C Amplitude";
        parameter.symbol     = "amplitudec";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 8.0f;
        break;
    case PARAM_OSCC_EXCITATION:
        parameter.hints      = 0;
        parameter.name       = "Osc. C Excitation";
        parameter.symbol     = "excitationc";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 4.0f;
        break;
    case PARAM_OSCC_LFO:
        parameter.hints      = 0;
        parameter.name       = "Osc. C LFO";
        parameter.symbol     = "lfoc";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_EXCITATION_BURST:
        parameter.hints      = 0;
        parameter.name       = "Burst";
        parameter.symbol     = "burst";
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
    case PARAM_EXCITATION_SUSTAIN:
        parameter.hints      = 0;
        parameter.name       = "Sustain";
        parameter.symbol     = "sustain";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    case PARAM_EXCITATION_DECAY:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Decay";
        parameter.symbol     = "decay";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.1f;
        parameter.ranges.max = 1000.0f;
        break;
    case PARAM_EXCITATION_RELEASE:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Release";
        parameter.symbol     = "release";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.1f;
        parameter.ranges.max = 1000.0f;
        break;
    case PARAM_UNISON_VOICES:
        parameter.hints      = kParameterIsInteger;
        parameter.name       = "Unison Voices";
        parameter.symbol     = "univoices";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 1.0f;
        parameter.ranges.max = 10.0f;
        break;
    case PARAM_UNISON_DETUNE:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Unison Detune";
        parameter.symbol     = "unidetune";
        parameter.ranges.def = 10.0f;
        parameter.ranges.min = 0.1f;
        parameter.ranges.max = 100.0f;
        break;
    case PARAM_UNISON_WIDTH:
        parameter.hints      = kParameterIsLogarithmic;
        parameter.name       = "Unison Width";
        parameter.symbol     = "uniwidth";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    }
}


float DistrhoPluginOnyx::getParameterValue(uint32_t index) const
{
    switch (index) {
    case PARAM_OSCA_WAVEFORM:
        return oscparams[0].waveform;
    case PARAM_OSCA_SHAPE:
        return oscparams[0].shape;
    case PARAM_OSCA_FREQUENCY:
        return oscparams[0].frequency;
    case PARAM_OSCA_AMPLITUDE:
        return oscparams[0].amplitude;
    case PARAM_OSCA_EXCITATION:
        return oscparams[0].excitation;
    case PARAM_OSCA_LFO:
        return oscparams[0].lfo;
    case PARAM_OSCB_WAVEFORM:
        return oscparams[1].waveform;
    case PARAM_OSCB_SHAPE:
        return oscparams[1].shape;
    case PARAM_OSCB_FREQUENCY:
        return oscparams[1].frequency;
    case PARAM_OSCB_AMPLITUDE:
        return oscparams[1].amplitude;
    case PARAM_OSCB_EXCITATION:
        return oscparams[1].excitation;
    case PARAM_OSCB_LFO:
        return oscparams[1].lfo;
    case PARAM_OSCC_WAVEFORM:
        return oscparams[2].waveform;
    case PARAM_OSCC_SHAPE:
        return oscparams[2].shape;
    case PARAM_OSCC_FREQUENCY:
        return oscparams[2].frequency;
    case PARAM_OSCC_AMPLITUDE:
        return oscparams[2].amplitude;
    case PARAM_OSCC_EXCITATION:
        return oscparams[2].excitation;
    case PARAM_OSCC_LFO:
        return oscparams[2].lfo;
    case PARAM_EXCITATION_BURST:
        return excparams.burst;
    case PARAM_EXCITATION_ATTACK:
        return excparams.attack;
    case PARAM_EXCITATION_SUSTAIN:
        return excparams.sustain;
    case PARAM_EXCITATION_DECAY:
        return excparams.decay;
    case PARAM_EXCITATION_RELEASE:
        return excparams.release;
    case PARAM_UNISON_VOICES:
        return unison_voices;
    case PARAM_UNISON_DETUNE:
        return unison_detune;
    case PARAM_UNISON_WIDTH:
        return unison_width;
    default:
        return 0.0;
    }
}


void DistrhoPluginOnyx::setParameterValue(uint32_t index, float value)
{
    switch (index) {
    case PARAM_OSCA_WAVEFORM:
        oscparams[0].waveform=(waveform_t) value;
        break;
    case PARAM_OSCA_SHAPE:
        oscparams[0].shape=value;
        break;
    case PARAM_OSCA_FREQUENCY:
        oscparams[0].frequency=value;
        break;
    case PARAM_OSCA_AMPLITUDE:
        oscparams[0].amplitude=value;
        break;
    case PARAM_OSCA_EXCITATION:
        oscparams[0].excitation=value;
        break;
    case PARAM_OSCA_LFO:
        oscparams[0].lfo=value;
        break;
    case PARAM_OSCB_WAVEFORM:
        oscparams[1].waveform=(waveform_t) value;
        break;
    case PARAM_OSCB_SHAPE:
        oscparams[1].shape=value;
        break;
    case PARAM_OSCB_FREQUENCY:
        oscparams[1].frequency=value;
        break;
    case PARAM_OSCB_AMPLITUDE:
        oscparams[1].amplitude=value;
        break;
    case PARAM_OSCB_EXCITATION:
        oscparams[1].excitation=value;
        break;
    case PARAM_OSCB_LFO:
        oscparams[1].lfo=value;
        break;
    case PARAM_OSCC_WAVEFORM:
        oscparams[2].waveform=(waveform_t) value;
        break;
    case PARAM_OSCC_SHAPE:
        oscparams[2].shape=value;
        break;
    case PARAM_OSCC_FREQUENCY:
        oscparams[2].frequency=value;
        break;
    case PARAM_OSCC_AMPLITUDE:
        oscparams[2].amplitude=value;
        break;
    case PARAM_OSCC_EXCITATION:
        oscparams[2].excitation=value;
        break;
    case PARAM_OSCC_LFO:
        oscparams[2].lfo=value;
        break;
    case PARAM_EXCITATION_BURST:
        excparams.burst=value;
        break;
    case PARAM_EXCITATION_ATTACK:
        excparams.attack=value;
        break;
    case PARAM_EXCITATION_SUSTAIN:
        excparams.sustain=value;
        break;
    case PARAM_EXCITATION_DECAY:
        excparams.decay=value;
        break;
    case PARAM_EXCITATION_RELEASE:
        excparams.release=value;
        break;
    case PARAM_UNISON_VOICES:
        unison_voices=(int) value;
        break;
    case PARAM_UNISON_DETUNE:
        unison_detune=value;
        break;
    case PARAM_UNISON_WIDTH:
        unison_width=value;
        break;
    }
}


void DistrhoPluginOnyx::activate()
{
    downsamplers=new Downsampler[2];
}


void DistrhoPluginOnyx::deactivate()
{
    voices.clear();

    delete[] downsamplers;
    downsamplers=nullptr;
}


void DistrhoPluginOnyx::run(const float**, float** outputs, uint32_t frames, const MidiEvent* midievents, uint32_t nummidievents)
{
    while (nummidievents--) {
        if ((midievents->data[0] & 0xf0) == 0x90) {
            if (unison_voices>1)
                for (int i=0;i<unison_voices;i++) {
                    float t=2.0f*i/(unison_voices-1) - 1.0f;
                    voices.emplace_back(new Voice(*this, midievents->data[1], 2.0f*getSampleRate(), 0.01f*t*unison_detune, (1.0f+t*unison_width)/unison_voices, (1.0f-t*unison_width)/unison_voices));
                }
            else
                voices.emplace_back(new Voice(*this, midievents->data[1], 2.0f*getSampleRate(), 0.0f));
        }

        if ((midievents->data[0] & 0xf0) == 0x80) {
            for (auto& voice: voices)
                if (voice->get_note()==midievents->data[1])
                    voice->note_off();
        }

        midievents++;
    }

    uint outptr=0;
    while (outptr<2*frames) {
        uint chunksize=2*frames-outptr;

        float* bufleft=downsamplers[0].get_input_buffer(chunksize);
        float* bufright=downsamplers[1].get_input_buffer(chunksize);

        for (auto& voice: voices)
            voice->produce(bufleft, bufright, chunksize);

        outptr+=chunksize;
    }

    downsamplers[0].write_output(outputs[0], frames);
    downsamplers[1].write_output(outputs[1], frames);

    voices.erase(std::remove_if(voices.begin(), voices.end(), [](const std::unique_ptr<Voice>& voice) { return voice->decayed(); }), voices.end());
}


Plugin* createPlugin()
{
    return new DistrhoPluginOnyx();
}



END_NAMESPACE_DISTRHO
