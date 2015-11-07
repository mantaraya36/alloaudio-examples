

#include <allocore/al_Allocore.hpp>
#include <allocore/types/al_SingleRWRingBuffer.hpp>
#include "Cuttlebone/Cuttlebone.hpp"
#include "Gamma/scl.h"

#include "soundfilebuffered.hpp"
#include "allospherespeakerlayouts.h"

#include "../ambiMetering_renderer/state.hpp"

#define AUDIO_BLOCK_SIZE 512

using namespace al;
using namespace std;

//#define SIN gam::scl::sinT9
//#define COS gam::scl::cosT8

#define SIN sin
#define COS cos

class MeterParams {
public:
	MeterParams() :
	    mSphere(true),
	    mPeak(false),
	    mDecibel(false),
	    mGrayscale(false),
	    mPeakGain(3.0),
	    mRmsGain(20.0),
	    mDbRange(80.0)
	{}
	
	bool mSphere;
	bool mPeak;
	bool mDecibel;
	bool mGrayscale;
	
	
	float mPeakGain;
	float mRmsGain;
	double mDbRange;
};

class AmbiMeter : public App
{
public:
	
	AmbiMeter() :
	    mMeterValues(3, AlloFloat32Ty, SPATIAL_SAMPLING, SPATIAL_SAMPLING, SPATIAL_SAMPLING),
	    mTextureBuffer(((SPATIAL_SAMPLING * SPATIAL_SAMPLING * 3 * 4) + 1 ) * sizeof(float) )
	{
		addSphereWithTexcoords(mMesh, 1, 128);
		addCube(mFrontalMesh, false, 0.1);
		
		lens().near(0.1).far(25).fovy(45);
		nav().pos(0,0,4);
		nav().quat().fromAxisAngle(0. * M_2PI, 0, 1, 0);
		
		initWindow(Window::Dim(0,0, 600, 400), "Ambisonics Metering", 30);
		mStateTaker.start();
	}
	
	virtual void onCreate(const ViewpointWindow &win) override {

	}
	
	virtual void onDraw(Graphics &g) override {
		
//		g.polygonMode(Graphics::LINE);
		State s;
		
		if (mStateTaker.get(s) > 0) {
			while (mStateTaker.get(s) > 0) {} // Pop all states in queue
		
			memcpy(mMeterValues.data.ptr, s.rmsTexture,
			       SPATIAL_SAMPLING *SPATIAL_SAMPLING * 3 * sizeof(float));
			mTexture.submit(mMeterValues, true);
			
			mTexture.filter(Texture::NEAREST);
		}
		
//			mTexture.filter(Texture::LINEAR);
		if (params.mSphere) {
			mTexture.bind();
			g.draw(mMesh);
			mTexture.unbind();
			g.pushMatrix();
			g.translate(1.5, 0, 0);
			g.draw(mFrontalMesh);
			g.popMatrix();
		} else {
			mTexture.quad(g, 2, 2, -2, -1);
			g.pushMatrix();
			g.rotate(180, 0, 1, 0);
			mTexture.quad(g, 2, 2, -2, -1);
			g.popMatrix();
		}
	}

	
//	virtual void onKeyDown(const Keyboard& k) override {
//		if (k.key() == 'g') {
//			params.mGrayscale = !params.mGrayscale;
//		} else if (k.key() == 'h') {
//			params.mDecibel = !params.mDecibel;
//		} else if (k.key() == 'j') {
//			params.mSphere = !params.mSphere;
//		} else if (k.key() == 'k') {
//			params.mPeak = !params.mPeak;
//		} 
//	}
	
private:
	Mesh mMesh;
	Mesh mFrontalMesh;
	Texture mTexture;
	Array mMeterValues;
	SingleRWRingBuffer mTextureBuffer;
	
	
	cuttlebone::Taker<State> mStateTaker;
	
	MeterParams params;
};


int main(int argc, char *argv[])
{
	AmbiMeter app;
	app.start();
}
