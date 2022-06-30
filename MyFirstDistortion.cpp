#include "MyFirstDistortion.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IKeyboardControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
    kGain = 0,
    sSwitch = 1,
    mAttack = 2,
    mDecay = 3,
    mSustain = 4,
    mRelease = 5,
    kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  sSwitchX = (kWidth - 64) / 2,     // - 64 because the knob width/height is 64
  sSwitchY = (kHeight - 64 - 64) / 2,    // another -64 because the keyboard height is 64
  sSwitchFrames = 6,

  kGainX = sSwitchX + 100,
  kGainY = sSwitchY + 100,
  kKnobFrames = 60,

  kKeybX = 1,
  kKeybY = kHeight - 64,

  adsrKnobXl = 90 - 48,
  adsrKnobXr = kWidth - 90,
  adsrKnobYu = 20,
  adsrKnobYd = 120
};

MyFirstDistortion::MyFirstDistortion(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mFrequency(1.),
    lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber - 1)
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

  IBitmap whiteKeyImage = pGraphics->LoadIBitmap(KEYW_ID, KEYW_FN, 6); // 6 frames
  IBitmap blackKeyImage = pGraphics->LoadIBitmap(KEYB_ID, KEYB_FN);
  //                            C#     D#          F#      G#      A#
  int keyCoordinates[12] = { 0, 7, 12, 20, 24, 36, 43, 48, 56, 60, 69, 72 };
  mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, /* octaves: */ 5, &whiteKeyImage, &blackKeyImage, keyCoordinates);
  pGraphics->AttachControl(mVirtualKeyboard);

  pGraphics->AttachControl(new IKnobMultiControl(this, kGainX, kGainY, kGain, &gKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, sSwitchX, sSwitchY, sSwitch, &knob));


  // Knob bitmap for ADSR
  // Attack knob:
  GetParam(mAttack)->InitDouble("Attack", 0.05, 0.01, 10.0, 0.001);
  GetParam(mAttack)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, adsrKnobXl, adsrKnobYu, mAttack, &gKnob));
  // Decay knob:
  GetParam(mDecay)->InitDouble("Decay", 0.5, 0.01, 15.0, 0.001);
  GetParam(mDecay)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, adsrKnobXl, adsrKnobYd, mDecay, &gKnob));
  // Sustain knob:
  GetParam(mSustain)->InitDouble("Sustain", 0.1, 0.001, 1.0, 0.001);
  GetParam(mSustain)->SetShape(2);
  pGraphics->AttachControl(new IKnobMultiControl(this, adsrKnobXr, adsrKnobYu, mSustain, &gKnob));
  // Release knob:
  GetParam(mRelease)->InitDouble("Release", 1.0, 0.001, 15.0, 0.001);
  GetParam(mRelease)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, adsrKnobXr, adsrKnobYd, mRelease, &gKnob));

  
  AttachGraphics(pGraphics);
  //CreatePresets();

  mMIDIReceiver.noteOn.Connect(this, &MyFirstDistortion::onNoteOn);
  mMIDIReceiver.noteOff.Connect(this, &MyFirstDistortion::onNoteOff);
}

MyFirstDistortion::~MyFirstDistortion() {}

void MyFirstDistortion::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

    double* leftOutput = outputs[0];
    double* rightOutput = outputs[1];

    processVirtualKeyboard();

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
        // leftOutput[i] = rightOutput[i] = mGain * mOscillator.nextSample() * velocity / 127.0;
        leftOutput[i] = rightOutput[i] = mGain * mOscillator.nextSample() * mEnvelopeGenerator.nextSample() * velocity / 127.0;
    }

    mMIDIReceiver.Flush(nFrames);
}

void MyFirstDistortion::Reset()
{
    TRACE;
    IMutexLock lock(this);
    mOscillator.setSampleRate(GetSampleRate());
    mEnvelopeGenerator.setSampleRate(GetSampleRate());
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

    case mAttack: //2
    case mDecay: //3
    case mSustain: //4
    case mRelease: //5
        mEnvelopeGenerator.setStageValue(static_cast<EnvelopeGenerator::EnvelopeStage>(paramIdx), GetParam(paramIdx)->Value());
        break;


    default:
      break;
  }
}

/*
void MyFirstDistortion::CreatePresets() {
    MakeDefaultPreset((char*)"-", kNumPrograms);
}
*/

void MyFirstDistortion::ProcessMidiMsg(IMidiMsg* pMsg) {
    mMIDIReceiver.onMessageReceived(pMsg);
    mVirtualKeyboard->SetDirty();
}

void MyFirstDistortion::processVirtualKeyboard() {
    IKeyboardControl* virtualKeyboard = (IKeyboardControl*)mVirtualKeyboard;
    int virtualKeyboardNoteNumber = virtualKeyboard->GetKey() + virtualKeyboardMinimumNoteNumber;

    if (lastVirtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
        // The note number has changed from a valid key to something else (valid key or nothing). Release the valid key:
        IMidiMsg midiMessage;
        midiMessage.MakeNoteOffMsg(lastVirtualKeyboardNoteNumber, 0);
        mMIDIReceiver.onMessageReceived(&midiMessage);
    }

    if (virtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
        // A valid key is pressed that wasn't pressed the previous call. Send a "note on" message to the MIDI receiver:
        IMidiMsg midiMessage;
        midiMessage.MakeNoteOnMsg(virtualKeyboardNoteNumber, virtualKeyboard->GetVelocity(), 0);
        mMIDIReceiver.onMessageReceived(&midiMessage);
    }

    lastVirtualKeyboardNoteNumber = virtualKeyboardNoteNumber;
}