
#include <flext.h>
#include <random>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif

class impulse: public flext_dsp{

	FLEXT_HEADER(impulse,flext_dsp)
 
public:
	impulse(float numOutlets);

protected:
    void m_base_amp(float amp);
    void m_rand_amp(float amp);

    virtual void m_signal(int n, float *const *in, float *const *out);
    float mNumOutlets;

private:
    float random(float max);
    float mstosamps(float ms);
    float mBaseInterval;
    float mRandInterval;
    float mBaseAmp;
    float mRandAmp;
    float mCoef;

    std::default_random_engine re;

    unsigned int mOutletCounter;
    long mSampleCounter;

    FLEXT_CALLBACK(m_bang);
    FLEXT_CALLBACK_F(m_base_interval);
    FLEXT_CALLBACK_F(m_rand_interval);
    FLEXT_CALLBACK_F(m_base_amp);
    FLEXT_CALLBACK_F(m_rand_amp);

};

FLEXT_NEW_DSP_1("impulse~",impulse, float)

impulse::impulse(float numOutlets):
mNumOutlets(numOutlets),
mOutletCounter(0),
mSampleCounter(0){

    if(mNumOutlets < 1){
        mNumOutlets = 1;
    }else if(mNumOutlets > 32){
        mNumOutlets = 32;
    }

    AddInSignal("base interval");
    AddInSignal("random interval");
    AddInFloat("base amp");
    AddInFloat("random amp");

    FLEXT_ADDMETHOD(2, m_base_amp); 
    FLEXT_ADDMETHOD(3, m_rand_amp);

    for(int i = 0 ; i < mNumOutlets; i++){
         AddOutSignal("impulse");
    }
} 


void impulse::m_base_amp(float amp){
    mBaseAmp = amp;
}

void impulse::m_rand_amp(float amp){
    mRandAmp = amp;
}

void impulse::m_signal(int n, float *const *in, float *const *out){

    float *baseInterval = in[0];
    float *randInterval = in[1];

    for (int i = 0; i < n; i++){

        if(mSampleCounter <= 0){
            float value = mBaseAmp + random(mRandAmp);
            for(int j = 0; j < mNumOutlets; j++){
                out[j][i] = (mOutletCounter == j) ? value : 0.0f;
            }          

            mOutletCounter++;
            if(mOutletCounter >= mNumOutlets){ 
                mOutletCounter = 0;
            }

            mSampleCounter = (long)(mBaseInterval[i] + random(mRandInterval[i]));

        }else{
            for(int j = 0; j < mNumOutlets; j++){
                out[j][i] = 0;
            }
        }
        mSampleCounter--;
    }
} 

float impulse::random(float max){
   std::uniform_real_distribution<float> unif(0, max);
   return unif(re);
}

float impulse::mstosamps(float ms){
    return Samplerate() * 0.001f * ms;
}