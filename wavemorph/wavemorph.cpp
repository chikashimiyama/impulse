
#include <flext.h>
#include <random>
#include <cmath>
#include <algorithm>
#include <memory>
#include <array>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif


/* a 2D waveform overlapping */
class wavemorph: public flext_dsp{

	FLEXT_HEADER(wavemorph,flext_dsp)
 
public:
    struct Point{
        Point():mX(0),mY(0){}
        void set(float inX, float inY){
            mX = clamp(inX);
            mY = clamp(inY);
        }

        float mX;
        float mY;

        private:
        float clamp(float in){
            return std::max(0.0f, std::min(in, 1.0f));
        }
    };

	wavemorph();
    void m_start(float x, float y);
    void m_start_rand(float x, float y);
    void m_end(float x, float y);
    void m_end_rand(float x, float y);
    void m_curve(float);
    void m_curve_rand(float);

protected:
    virtual void m_signal(int n, float *const *in, float *const *out);
private:
    std::random_device rd;
    std::mt19937 mGen; 

    void trigger();
    unsigned long mstosamps(float ms);
    float random(float max);
    float clamp(float in);
    double fastSqrt(double);
    double fastPow(double , double);

    FLEXT_CALLBACK_FF(m_start);
    FLEXT_CALLBACK_FF(m_start_rand);
    FLEXT_CALLBACK_FF(m_end);
    FLEXT_CALLBACK_FF(m_end_rand);
    FLEXT_CALLBACK_F(m_curve);
    FLEXT_CALLBACK_F(m_curve_rand);

    Point mStart;
    Point mEnd;
    Point mStartRandom;
    Point mEndRandom;

    Point mStartFixed;
    Point mMovement;
    Point mCurrentPoint;
    float mPreviousSample;

    float mCurve;
    float mCurveRandom;
    float mCurveFixed;

};

FLEXT_NEW_DSP("wavemorph~",wavemorph)

inline wavemorph::wavemorph():
mPreviousSample(1.0),
mGen(rd()){

	AddInAnything("phase in"); 
    AddOutSignal("amp0");
    AddOutSignal("amp1");
    AddOutSignal("amp2");
    AddOutSignal("amp3");

    FLEXT_ADDMETHOD_FF(0, "start", m_start);
    FLEXT_ADDMETHOD_FF(0, "start_rand", m_start_rand); 
    FLEXT_ADDMETHOD_FF(0, "end", m_end); 
    FLEXT_ADDMETHOD_FF(0, "end_rand", m_end_rand); 
    FLEXT_ADDMETHOD_F(0, "curve", m_curve); 
    FLEXT_ADDMETHOD_F(0, "curve_rand", m_curve_rand); 

} 

inline void wavemorph::m_start(float x, float y){ 
    mStart.set(x,y);
}

inline void wavemorph::m_end(float x, float y){ 
    mEnd.set(x,y);
}

inline void wavemorph::m_start_rand(float x, float y){
    mStartRandom.set(x,y);
}

inline void wavemorph::m_end_rand(float x, float y){ 
    mEndRandom.set(x,y);
}

inline void wavemorph::m_curve(float value){ 
    mCurve = value;
}

inline void wavemorph::m_curve_rand(float value){ 
    mCurveRandom = value;
}

inline void wavemorph::trigger(){
    Point endFixed;
    mStartFixed.set(mStart.mX + random(mStartRandom.mX), mStart.mY + random(mStartRandom.mY));
    endFixed.set(mEnd.mX + random(mEndRandom.mX), mEnd.mY + random(mEndRandom.mY));
    mMovement.set(endFixed.mX - mStart.mX, endFixed.mY - mStart.mY);
    mCurrentPoint = mStartFixed;
    mCurveFixed = exp(mCurve + random(mCurveRandom));
}

inline void wavemorph::m_signal(int n, float *const *in, float *const *out){
    float *ins= in[0];

    Point currentPoint;
    float distances[4];

    for(int i = 0 ;i < n; i++){

        float phase = clamp(*ins++);
        if(phase < mPreviousSample){
            trigger();
        }
        float distorted = fastPow(phase, mCurveFixed);
        currentPoint.mX = mMovement.mX * distorted + mStartFixed.mX;
        currentPoint.mY = mMovement.mY * distorted + mStartFixed.mY;
        float fromRight = 1.0f - currentPoint.mX;
        float fromBottom =  1.0f - currentPoint.mY;
        distances[0] = fastSqrt(currentPoint.mX * currentPoint.mX +  currentPoint.mY * currentPoint.mY);
        distances[1] = fastSqrt(fromRight * fromRight + currentPoint.mY * currentPoint.mY);
        distances[2] = fastSqrt(fromRight * fromRight +  fromBottom * fromBottom);
        distances[3] = fastSqrt(currentPoint.mX * currentPoint.mX + fromBottom * fromBottom);

        for(int j = 0; j < 4; j++){
            float amp = 1.0 - distances[j];
            if(amp < 0.0f){ amp = 0.0f; }
            out[j][i] = amp;
        }
        mPreviousSample = phase;
    }
} 

inline float wavemorph::random(float max){
    std::uniform_real_distribution<float> unif(-max, max);
    return unif(mGen);
}

inline float wavemorph::clamp(float max){
    return std::min(1.0f, std::max(max, 0.0f));
}

inline double wavemorph::fastSqrt(double a) {
    return fastPow(a, 0.5);
}

inline double wavemorph::fastPow(double a, double b) {
    union {
        double d;
        int x[2];
    } u = { a };

    u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}