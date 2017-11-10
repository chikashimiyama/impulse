
#include <flext.h>
#include <random>
#include <chrono>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif

class glisson: public flext_dsp{

	FLEXT_HEADER(glisson,flext_dsp)
 
public:
	glisson();

protected:

    virtual void m_signal(int n, float *const *in, float *const *out);

    void m_start_low(float);
    void m_start_high(float);
    void m_end_low(float);
    void m_end_high(float);
    void m_curve(float);
    void m_curve_rand(float);

private:
    double fastPow(double a, double b);
    float random(float min, float max);
    void trigger();

    FLEXT_CALLBACK_F(m_start_low);
    FLEXT_CALLBACK_F(m_start_high);
    FLEXT_CALLBACK_F(m_end_low);
    FLEXT_CALLBACK_F(m_end_high);
    FLEXT_CALLBACK_F(m_curve);
    FLEXT_CALLBACK_F(m_curve_rand);

    float mPreviousSample;
    float mStart;
    float mDistance;

    float mStartLow;
    float mStartHigh;
    float mEndLow;
    float mEndHigh;

    float mCurve;
    float mCurveRandom;
    float mCurveFix;

    std::mt19937 mMt;

};

FLEXT_NEW_DSP("glisson~",glisson)

glisson::glisson():
mPreviousSample(1.0){

	AddInAnything("phase"); 
    AddOutSignal("glisson");

    FLEXT_ADDMETHOD_F(0,"start_low", m_start_low);
    FLEXT_ADDMETHOD_F(0,"start_high", m_start_high); 
    FLEXT_ADDMETHOD_F(0,"end_low", m_end_low); 
    FLEXT_ADDMETHOD_F(0,"end_high", m_end_high); 
    FLEXT_ADDMETHOD_F(0,"curve", m_curve); 
    FLEXT_ADDMETHOD_F(0,"curve_rand", m_curve_rand); 

    std::random_device rd;
    mMt.seed(rd());

} 

void glisson::m_start_low(float value){ mStartLow = value;}
void glisson::m_start_high(float value){ mStartHigh = value;}
void glisson::m_end_low(float value){ mEndLow = value;}
void glisson::m_end_high(float value){ mEndHigh = value;}
void glisson::m_curve(float value){ mCurve = value;}
void glisson::m_curve_rand(float value){ mCurveRandom = value;}

void glisson::m_signal(int n, float *const *in, float *const *out){

    float *ins = in[0];
    float *outs = out[0];

    while(n--){
    	float currentSample = *ins++;
    	if( currentSample < mPreviousSample){
    		trigger();
    	}

    	*outs++ = fastPow(currentSample, mCurveFix) * mDistance + mStart;
    	mPreviousSample = currentSample;
    }
} 

void glisson::trigger(){
	mStart = random(mStartLow, mStartHigh);
	mDistance = random(mEndLow, mEndHigh) - mStart;
    mCurveFix = exp(mCurve+random(-mCurveRandom, mCurveRandom));

}

float glisson::random(float min, float max){
    std::uniform_real_distribution<float> unif(min, max);
    return unif(mMt);
}

inline double glisson::fastPow(double a, double b) {
    union {
        double d;
        int x[2];
    } u = { a };
    u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}