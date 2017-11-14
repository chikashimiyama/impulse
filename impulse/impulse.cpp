
#include <flext.h>
#include <random>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif

class impulse: public flext_dsp{

	FLEXT_HEADER(impulse,flext_dsp)
 
public:
	impulse(float numOutlets);
    void m_dropoff(float ratio);
protected:
    virtual void m_signal(int n, float *const *in, float *const *out);

private:
    int mstosamps(float ms);
    bool dropoff();

    float mNumOutlets;
    unsigned int mOutletCounter;
    int mSampleCounter;
    int mDropOffRatio;
    std::default_random_engine re;

    FLEXT_CALLBACK_F(m_dropoff);


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
    FLEXT_ADDMETHOD_F(0,"dropoff", m_dropoff);

} 

void impulse::m_dropoff(float ratio){
    mDropOffRatio = (int)ratio;
}

void impulse::m_signal(int n, float *const *in, float *const *out){
    float *interval = in[0];
    float *amp = in[1];

    for (int i = 0; i < n; i++){
        float inValue = *interval++;
        float ampValue = *amp++;

        if(mSampleCounter <= 0){
            bool valid = dropoff();;
            for(int j = 0; j < mNumOutlets; j++){
                ampValue *= valid;
                out[j][i] = (mOutletCounter == j) ? ampValue : 0.0f;
            }          

            if(valid){ mOutletCounter++;}
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

bool impulse::dropoff(){
    std::uniform_int_distribution<> unii(0, 99);
    return mDropOffRatio <= unii(re);
}

