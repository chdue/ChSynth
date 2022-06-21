#include "MyFirstDistortion.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
    kGain = 0,
    kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kGainX = (kWidth - 48)/2,
  kGainY = (kHeight - 48) / 2,
  kKnobFrames = 60
};

MyFirstDistortion::MyFirstDistortion(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mFrequency(1.)
{
  TRACE;

  GetParam(kGain)->InitDouble("Makeup Gain", 0, -10.0, 10.0, 0.01, "dBs");
  GetParam(kGain)->SetShape(1.);

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_RED);

  IBitmap gKnob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames); //will be changed when new resources are made

  pGraphics->AttachControl(new IKnobMultiControl(this, kGainX, kGainY, kGain, &gKnob));
  
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
        leftOutput[i] = rightOutput[i] = mGain * mOscillator.nextSample() * velocity / 127.0;
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

  switch (paramIdx)
  {
    case kGain:
      mGain = exp(GetParam(kGain)->Value() / 10); // 10 * log(ratio) = dB Change
      break;

    default:
      break;
  }
}

void MyFirstDistortion::CreatePresets() {
    MakeDefaultPreset((char*)"-", kNumPrograms);
}

void MyFirstDistortion::ProcessMidiMsg(IMidiMsg* pMsg) {
    mMIDIReceiver.onMessageReceived(pMsg);
}