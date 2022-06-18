#include "MyFirstDistortion.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
    kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT
};

MyFirstDistortion::MyFirstDistortion(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mFrequency(1.)
{
  TRACE;

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_RED);
  AttachGraphics(pGraphics);
  CreatePresets();
}

MyFirstDistortion::~MyFirstDistortion() {}

void MyFirstDistortion::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

    double* leftOutput = outputs[0];
    double* rightOutput = outputs[1];

    for (int i = 0; i < nFrames; ++i) {
        mMIDIReceiver.advance();
        int velocity = mMIDIReceiver.getLastVelocity();
        if (velocity > 0) {
            mOscillator.setFrequency(mMIDIReceiver.getLastFrequency());
            mOscillator.setMuted(false);
        }
        else {
            mOscillator.setMuted(true);
        }
        leftOutput[i] = rightOutput[i] = mOscillator.nextSample() * velocity / 127.0;
    }

    mMIDIReceiver.Flush(nFrames);
}

void MyFirstDistortion::Reset()
{
    TRACE;
    IMutexLock lock(this);
    mOscillator.setSampleRate(GetSampleRate());
}

void MyFirstDistortion::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
}

void MyFirstDistortion::CreatePresets() {
    MakeDefaultPreset((char*)"-", kNumPrograms);
}

void MyFirstDistortion::ProcessMidiMsg(IMidiMsg* pMsg) {
    mMIDIReceiver.onMessageReceived(pMsg);
}