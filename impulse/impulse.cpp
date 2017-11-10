
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
    virtual void m_signal(int n, float *const *in, float *const *out);

private:
    float random(float max);
    int mstosamps(float ms);

    float mNumOutlets;
    unsigned int mOutletCounter;
    int mSampleCounter;

};

FLEXT_NEW_DSP_1("impulse~",impulse, float)

impulse::impulse(float numOutlets):
mNumOutlets(numOutlets),
mOutletCounter(0),
mSampleCounter(1){

    if(mNumOutlets < 1){
        mNumOutlets = 1;
    }else if(mNumOutlets > 32){
        mNumOutlets = 32;
    }

    AddInSignal("interval in");
    AddInSignal("amp in");
    for(int i = 0 ; i < mNumOutlets; i++){
         AddOutSignal("impulse out");
    }
} 

void impulse::m_signal(int n, float *const *in, float *const *out){
    float *interval = in[0];
    float *amp = in[1];

    for (int i = 0; i < n; i++){
        float inValue = *interval++;
        float ampValue = *amp++;

        if(mSampleCounter <= 0){
            for(int j = 0; j < mNumOutlets; j++){
                out[j][i] = (mOutletCounter == j) ? ampValue : 0.0f;
            }          

            mOutletCounter++;
            if(mOutletCounter >= mNumOutlets){ 
                mOutletCounter = 0;
            }
            mSampleCounter = mstosamps(inValue);
            if(mSampleCounter <= 0) mSampleCounter = 1;
        }else{
            for(int j = 0; j < mNumOutlets; j++){
                out[j][i] = 0;
            }
        }
        mSampleCounter--;
    }
}

int impulse::mstosamps(float ms){
    return static_cast<int>(Samplerate() * 0.001f * ms);
}

