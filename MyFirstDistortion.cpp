#include "MyFirstDistortion.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
    kGain = 0,
    sSwitch = 1,
    kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  sSwitchX = (kWidth - 64) / 2,
  sSwitchY = (kHeight - 64) / 2,
  sSwitchFrames = 6,

  kGainX = sSwitchX + 100,
  kGainY = sSwitchY + 100,
  kKnobFrames = 60
};

MyFirstDistortion::MyFirstDistortion(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mFrequency(1.)
{
  TRACE;

  GetParam(kGain)->InitDouble("Makeup Gain", 0, -23.0, 23.0, 0.01, "dBs");
  GetParam(kGain)->SetShape(1.);

  GetParam(sSwitch)->InitInt("Waveform", 0.0, 0.0, 5.0,  "Type");
  GetParam(sSwitch)->SetShape(1.);

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  //pGraphics->AttachPanelBackground(&COLOR_RED);
  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);

  IBitmap gKnob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  IBitmap knob = pGraphics->LoadIBitmap(sKNOB_ID, sKNOB_FN, sSwitchFrames);

  pGraphics->AttachControl(new IKnobMultiControl(this, kGainX, kGainY, kGain, &gKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, sSwitchX, sSwitchY, sSwitch, &knob));
  
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

    case sSwitch:
        if (GetParam(sSwitch)->Value() == 0) {
            mOscillator.setMode(OSCILLATOR_MODE_SINE);
        }
        else if (GetParam(sSwitch)->Value() == 1) {
            mOscillator.setMode(OSCILLATOR_MODE_SAW);
        }
        else if (GetParam(sSwitch)->Value() == 2) {
            mOscillator.setMode(OSCILLATOR_MODE_TRIANGLE);
        }
        else if (GetParam(sSwitch)->Value() == 3) {
            mOscillator.setMode(OSCILLATOR_MODE_SQUARE);
        }
        else if (GetParam(sSwitch)->Value() == 4) {
            mOscillator.setMode(OSCILLATOR_MODE_ROUNDED_SQUARE);
        }
        else if (GetParam(sSwitch)->Value()  == 5) {
            mOscillator.setMode(OSCILLATOR_MODE_RANDOM);
        }
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