
#include <flext.h>
#include <random>
#include <cmath>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif

enum class Mode{
    Attack = 0,
    Sustain,
    Release,
    Idle
};

struct Envelope{

    Envelope():
    mAttackTime(5),
    mAttackTimeRandom(0),
    mAttackCurve(1),
    mAttackCurveRandom(0),
    mSustainTime(5),
    mSustainTimeRandom(0),
    mSustainLevel(1),
    mReleaseTimeRandom(0),
    mReleaseTime(5),
    mReleaseCurve(1),
    mMode(Mode::Idle),
    mElapsedSample(0){}

    void operator= (const Envelope &e ) { 
        mAttackTime = e.mAttackTime;
        mAttackTimeRandom = e.mAttackTimeRandom;
        mAttackCurve = e.mAttackCurve;
        mAttackCurveRandom = e.mAttackCurveRandom;
        mSustainTime = e.mSustainTime;
        mSustainTimeRandom = e.mSustainTimeRandom;
        mReleaseTime = e.mReleaseTime;
        mReleaseTimeRandom = e.mReleaseTimeRandom;
        mReleaseCurve = e.mReleaseCurve;
        mReleaseCurveRandom = e.mReleaseCurveRandom;
    }

    void trigger(float level){
        mSustainLevel = level;
        mMode = Mode::Attack;
        mElapsedSample = 0;
        mGlobalElapsedSample = 0;
        mAttackTime += random(mAttackTimeRandom);
        mAttackCurve += random(mAttackCurveRandom);
        mSustainTime += random(mSustainTimeRandom);
        mReleaseTime += random(mReleaseTimeRandom);
        mReleaseCurve += random(mReleaseCurveRandom);
        mEnvelopeDuration = mAttackTime + mSustainTime + mReleaseTime;
    }

    float getGlobalPhase(){
        return (float)mGlobalElapsedSample / (float)mEnvelopeDuration;
    }
    // 3 input
    float mSustainLevel;
    unsigned long mElapsedSample;
    unsigned long mGlobalElapsedSample;
    Mode mMode;

    // 10 parameters
    unsigned long mAttackTime;
    unsigned long mAttackTimeRandom;
    float mAttackCurve;
    float mAttackCurveRandom;
    unsigned long mSustainTime;
    unsigned long mSustainTimeRandom;
    unsigned long mReleaseTime;
    unsigned long mReleaseTimeRandom;
    float mReleaseCurve;
    float mReleaseCurveRandom;

    unsigned long mEnvelopeDuration;

private:
    unsigned long random(unsigned long max){
        std::uniform_int_distribution<unsigned long> unii(0, max);
        return unii(re);
    }

    float random(float max){
        std::uniform_real_distribution<float> unif(0, max);
        return unif(re);
    }

    std::default_random_engine re;

};

class envgen: public flext_dsp{

	FLEXT_HEADER(envgen,flext_dsp)
 
public:
	envgen();

protected:
    virtual void m_signal(int n, float *const *in, float *const *out);

    void m_info();
    void m_attack_time(float);
    void m_attack_time_rand(float);
    void m_attack_curve(float);
    void m_attack_curve_rand(float);

    void m_sustain_time(float);
    void m_sustain_time_rand(float);

    void m_release_time(float);
    void m_release_time_rand(float);
    void m_release_curve(float);
    void m_release_curve_rand(float);

private:
    unsigned long mstosamps(float ms);
    unsigned long random(unsigned long max);
    float random(float max);

    FLEXT_CALLBACK(m_info);
    FLEXT_CALLBACK_F(m_attack_time);
    FLEXT_CALLBACK_F(m_attack_time_rand);
    FLEXT_CALLBACK_F(m_attack_curve);
    FLEXT_CALLBACK_F(m_attack_curve_rand);
    FLEXT_CALLBACK_F(m_sustain_time);
    FLEXT_CALLBACK_F(m_sustain_time_rand);
    FLEXT_CALLBACK_F(m_release_time);
    FLEXT_CALLBACK_F(m_release_time_rand);
    FLEXT_CALLBACK_F(m_release_curve);
    FLEXT_CALLBACK_F(m_release_curve_rand);

    Envelope mInputEnvelope;
    Envelope mFixedEnvelope;

};

FLEXT_NEW_DSP("envgen~",envgen)

envgen::envgen(){

	AddInAnything("impulse in"); 
    AddOutSignal("envgen");
    AddOutSignal("phase");

    FLEXT_ADDMETHOD_(0, "info", m_info);
    FLEXT_ADDMETHOD_F(0,"attack_time",m_attack_time);
    FLEXT_ADDMETHOD_F(0,"attack_time_rand",m_attack_time_rand); 
    FLEXT_ADDMETHOD_F(0,"attack_curve",m_attack_curve); 
    FLEXT_ADDMETHOD_F(0,"attack_curve_rand",m_attack_curve_rand); 

    FLEXT_ADDMETHOD_F(0,"sustain_time",m_sustain_time); 
    FLEXT_ADDMETHOD_F(0,"sustain_time_rand",m_sustain_time_rand); 

    FLEXT_ADDMETHOD_F(0,"release_time",m_release_time); 
    FLEXT_ADDMETHOD_F(0,"release_time_rand",m_release_time_rand); 
    FLEXT_ADDMETHOD_F(0,"release_curve",m_release_curve);
    FLEXT_ADDMETHOD_F(0,"release_curve_rand",m_release_curve_rand); 
} 

void envgen::m_info(){
    post("attack time: %lu", mInputEnvelope.mAttackTime);
    post("attack time rand: %lu", mInputEnvelope.mAttackTimeRandom);
    post("attack curve: %f", mInputEnvelope.mAttackCurve);
    post("attack curve rand: %f", mInputEnvelope.mAttackTime);

    post("sustain time: %lu", mInputEnvelope.mSustainTime);
    post("sustain time rand: %lu", mInputEnvelope.mSustainTime);

    post("release rand: %lu", mInputEnvelope.mReleaseTime);
    post("release time rand: %lu", mInputEnvelope.mReleaseTimeRandom);
    post("release curve: %f", mInputEnvelope.mReleaseCurve);
    post("release curve rand: %f", mInputEnvelope.mReleaseCurveRandom);
}

void envgen::m_attack_time(float time){ mInputEnvelope.mAttackTime = mstosamps(time);}
void envgen::m_attack_time_rand(float time){ mInputEnvelope.mAttackTimeRandom = mstosamps(time);}
void envgen::m_attack_curve(float factor){ mInputEnvelope.mAttackCurve = exp(factor);}
void envgen::m_attack_curve_rand(float factor){ mInputEnvelope.mAttackCurveRandom = exp(factor);}
void envgen::m_sustain_time(float time){ mInputEnvelope.mSustainTime = mstosamps(time);}
void envgen::m_sustain_time_rand(float time){ mInputEnvelope.mSustainTimeRandom = mstosamps(time);}
void envgen::m_release_time(float time){ mInputEnvelope.mReleaseTime = mstosamps(time);}
void envgen::m_release_time_rand(float time){ mInputEnvelope.mReleaseTimeRandom = mstosamps(time);}
void envgen::m_release_curve(float factor){ mInputEnvelope.mReleaseCurve = exp(factor);}
void envgen::m_release_curve_rand(float factor){ mInputEnvelope.mReleaseCurveRandom = exp(factor);}

void envgen::m_signal(int n, float *const *in, float *const *out){

    float *ins = in[0];
    float *outs = out[0];
    float *pouts = out[1];
    while (n--){
        float input = *ins++;
        float sample = 0.0f;
        float globalPhase = 1.0f;
        if(mFixedEnvelope.mMode != Mode::Idle){
            // busy
            switch(mFixedEnvelope.mMode){
                case Mode::Attack:{
                    float localPhase = (float)mFixedEnvelope.mElapsedSample / (float)mFixedEnvelope.mAttackTime;
                    sample = pow(localPhase, mFixedEnvelope.mAttackCurve) * mFixedEnvelope.mSustainLevel;
                    if(mFixedEnvelope.mElapsedSample >= mFixedEnvelope.mAttackTime){
                        sample = mFixedEnvelope.mSustainLevel;
                        mFixedEnvelope.mMode = Mode::Sustain;
                        mFixedEnvelope.mElapsedSample = 0;
                    }
                    break;
                }
                case Mode::Sustain:{
                    sample = mFixedEnvelope.mSustainLevel;

                    if(mFixedEnvelope.mElapsedSample >= mFixedEnvelope.mSustainTime){
                        mFixedEnvelope.mMode = Mode::Release;
                        mFixedEnvelope.mElapsedSample = 0;
                    }
                    break;
                }
                case Mode::Release:{
                    float localPhase = 1.0f - (float)mFixedEnvelope.mElapsedSample / (float)mFixedEnvelope.mReleaseTime;
                    sample = pow(localPhase, mFixedEnvelope.mReleaseCurve) * mFixedEnvelope.mSustainLevel;

                    if(mFixedEnvelope.mElapsedSample >= mFixedEnvelope.mReleaseTime){
                        mFixedEnvelope.mMode = Mode::Idle;
                    }
                    break;
                }
            }
            globalPhase = mFixedEnvelope.getGlobalPhase();
            mFixedEnvelope.mElapsedSample++;
            mFixedEnvelope.mGlobalElapsedSample++;
        }else{
            // non busy
            sample = 0.0f;

            if(input > 0.0001f){
                mFixedEnvelope = mInputEnvelope;
                mFixedEnvelope.trigger(input);
            }
            globalPhase = 1.0;
        }
        *outs++ = sample;
        *pouts++ = globalPhase;
    }
} 

unsigned long envgen::mstosamps(float ms){
    return unsigned long(Samplerate() * 0.001f * ms);
}

