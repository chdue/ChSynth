#include "MyFirstDistortion.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IKeyboardControl.h"
#include "resource.h"
#include <functional>

const int kNumPrograms = 1;

enum EParams
{
    kGain = 0,
    mWaveform = 1,
    mAttack = 2,
    mDecay = 3,
    mSustain = 4,
    mRelease = 5,
    mFilterMode,
    mFilterCutoff,
    mFilterResonance,
    mFilterAttack,
    mFilterDecay,
    mFilterSustain,
    mFilterRelease,
    mFilterEnvelopeAmount,
    mLFOWaveform,
    mLFOFreq,
    mlfoFilterModAmount,
    mPitchMod,
    kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kKnobFrames = 60,

  kKeybX = 1,
  kKeybY = kHeight - 64
};

MyFirstDistortion::MyFirstDistortion(IPlugInstanceInfo instanceInfo) 
    : IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), 
    lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber - 1) 
{
    TRACE;


  // params

  GetParam(kGain)->InitDouble("Makeup Gain", 0, -23.0, 23.0, 0.01, "dBs");
  GetParam(kGain)->SetShape(1.);

  GetParam(mWaveform)->InitEnum("Waveform", Oscillator::OSCILLATOR_MODE_SINE, 6);

  GetParam(mAttack)->InitDouble("Attack", 0.05, 0.01, 10.0, 0.001);
  GetParam(mAttack)->SetShape(3);

  GetParam(mDecay)->InitDouble("Decay", 15.0, 0.01, 15.0, 0.001);
  GetParam(mDecay)->SetShape(3);

  GetParam(mSustain)->InitDouble("Sustain", 1.0, 0.001, 1.0, 0.001);
  GetParam(mSustain)->SetShape(2);

  GetParam(mRelease)->InitDouble("Release", 0.5, 0.001, 15.0, 0.001);
  GetParam(mRelease)->SetShape(3);

  GetParam(mFilterMode)->InitEnum("Filter Mode", Filter::FILTER_MODE_LOWPASS, Filter::kNumFilterModes);

  GetParam(mFilterCutoff)->InitDouble("Cutoff", 0.99, 0.01, 0.99, 0.001);
  GetParam(mFilterCutoff)->SetShape(2);

  GetParam(mFilterResonance)->InitDouble("Resonance", 0.01, 0.01, 1.0, 0.001);

  GetParam(mFilterAttack)->InitDouble("Filter Env Attack", 0.05, 0.01, 10.0, 0.001);
  GetParam(mFilterAttack)->SetShape(3);

  GetParam(mFilterDecay)->InitDouble("Filter Env Decay", 15.0, 0.01, 15.0, 0.001);
  GetParam(mFilterDecay)->SetShape(3);

  GetParam(mFilterSustain)->InitDouble("Filter Env Sustain", 1.0, 0.001, 1.0, 0.001);
  GetParam(mFilterSustain)->SetShape(2);

  GetParam(mFilterRelease)->InitDouble("Filter Env Release", 0.5, 0.001, 15.0, 0.001);
  GetParam(mFilterRelease)->SetShape(3);

  GetParam(mFilterEnvelopeAmount)->InitDouble("Filter Env Amount", 0.0, -1.0, 1.0, 0.001);

  GetParam(mLFOWaveform)->InitEnum("LFO Waveform", Oscillator::OSCILLATOR_MODE_SINE, 5);
  GetParam(mLFOFreq)->InitDouble("LFO Frequency", 6.0, 0.01, 30.0, 0.001);
  GetParam(mlfoFilterModAmount)->InitDouble("LFO Filter Mod Amount", 0.0, 0.00, 1.0, 0.001);

  GetParam(mPitchMod)->InitDouble("Pitch Mod", 0.0, 0.00, 1.0, 0.001);


  // graphics

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);

  IBitmap waveSwitchMap = pGraphics->LoadIBitmap(WAVESWITCH_ID, WAVESWITCH_FN, 6);

  IBitmap filtermodeBitmap = pGraphics->LoadIBitmap(FILTERMODE_ID, FILTERMODE_FN, 3);

  IBitmap whiteKeyImage = pGraphics->LoadIBitmap(KEYW_ID, KEYW_FN, 6); // 6 frames
  IBitmap blackKeyImage = pGraphics->LoadIBitmap(KEYB_ID, KEYB_FN);

  IBitmap aKnob = pGraphics->LoadIBitmap(aKNOB_ID, aKNOB_FN, kKnobFrames);
  IBitmap dKnob = pGraphics->LoadIBitmap(dKNOB_ID, dKNOB_FN, kKnobFrames);
  IBitmap sKnob = pGraphics->LoadIBitmap(sKNOB_ID, sKNOB_FN, kKnobFrames);
  IBitmap rKnob = pGraphics->LoadIBitmap(rKNOB_ID, rKNOB_FN, kKnobFrames);
  IBitmap gKnob = pGraphics->LoadIBitmap(gKNOB_ID, gKNOB_FN, kKnobFrames);
  IBitmap pmKnob = pGraphics->LoadIBitmap(pmKNOB_ID, pmKNOB_FN, kKnobFrames);
  IBitmap cutKnob = pGraphics->LoadIBitmap(cutKNOB_ID, cutKNOB_FN, kKnobFrames);
  IBitmap resKnob = pGraphics->LoadIBitmap(resKNOB_ID, resKNOB_FN, kKnobFrames);
  IBitmap lfoKnob = pGraphics->LoadIBitmap(lfoKNOB_ID, lfoKNOB_FN, kKnobFrames);
  IBitmap envKnob = pGraphics->LoadIBitmap(envKNOB_ID, envKNOB_FN, kKnobFrames);
  IBitmap freqKnob = pGraphics->LoadIBitmap(freqKNOB_ID, freqKNOB_FN, kKnobFrames);

  IBitmap lfowaveSwitchMap = pGraphics->LoadIBitmap(lfoWAVESWITCH_ID, lfoWAVESWITCH_FN, 5);


  // controls

  //                            C#     D#          F#      G#      A#
  int keyCoordinates[12] = { 0, 7, 12, 20, 24, 36, 43, 48, 56, 60, 69, 72 };
  mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, /* octaves: */ 5, &whiteKeyImage, &blackKeyImage, keyCoordinates);
  pGraphics->AttachControl(mVirtualKeyboard);

  pGraphics->AttachControl(new IKnobMultiControl(this, 330, 156, kGain, &gKnob));

  pGraphics->AttachControl(new ISwitchControl(this, 85, 82, mWaveform, &waveSwitchMap));

  pGraphics->AttachControl(new IKnobMultiControl(this, 72, 174, mAttack, &aKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 124, 174, mDecay, &dKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 178, 174, mSustain, &sKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 234, 174, mRelease, &rKnob));

  pGraphics->AttachControl(new ISwitchControl(this, 246, 401, mFilterMode, &filtermodeBitmap));
  pGraphics->AttachControl(new IKnobMultiControl(this, 165, 346, mFilterCutoff, &cutKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 165, 413, mFilterResonance, &resKnob));


  pGraphics->AttachControl(new IKnobMultiControl(this, 190, 485, mFilterAttack, &aKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 240, 485, mFilterDecay, &dKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 295, 485, mFilterSustain, &sKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 351, 485, mFilterRelease, &rKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 328, 567, mFilterEnvelopeAmount, &envKnob));

  pGraphics->AttachControl(new ISwitchControl(this, 43, 643, mLFOWaveform, &lfowaveSwitchMap));
  pGraphics->AttachControl(new IKnobMultiControl(this, 240, 637, mLFOFreq, &freqKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 118, 567, mlfoFilterModAmount, &lfoKnob));

  pGraphics->AttachControl(new IKnobMultiControl(this, 262, 75, mPitchMod, &pmKnob));

  AttachGraphics(pGraphics);


  mMIDIReceiver.noteOn.Connect(&voiceManager, &VoiceManager::onNoteOn);
  mMIDIReceiver.noteOff.Connect(&voiceManager, &VoiceManager::onNoteOff);

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
        leftOutput[i] = rightOutput[i] = mGain * voiceManager.nextSample();
    }

    mMIDIReceiver.Flush(nFrames);
}

void MyFirstDistortion::Reset()
{
    TRACE;
    IMutexLock lock(this);
    double sampleRate = GetSampleRate();
    voiceManager.setSampleRate(sampleRate);
}

void MyFirstDistortion::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  IParam* param = GetParam(paramIdx);
  if (paramIdx == mLFOWaveform) {
      voiceManager.setLFOMode(static_cast<Oscillator::OscillatorMode>(param->Int()));
  }
  else if (paramIdx == mLFOFreq) {
      voiceManager.setLFOFrequency(param->Value());
  }
  else if (paramIdx == kGain) {
      mGain = exp(GetParam(kGain)->Value() / 10); // 10 * log(ratio) = dB Change
  }
  else {
      using std::tr1::placeholders::_1;
      using std::tr1::bind;
      VoiceManager::VoiceChangerFunction changer;
      switch (paramIdx) {
        case mWaveform:
            changer = bind(&VoiceManager::setOscillatorMode, _1, static_cast<Oscillator::OscillatorMode>(param->Int()));
            break;
        case mPitchMod:
            changer = bind(&VoiceManager::setOscillatorPitchMod, _1, param->Value());
            break;
            // Filter Section:
        case mFilterMode:
            changer = bind(&VoiceManager::setFilterMode, _1, static_cast<Filter::FilterMode>(param->Int()));
            break;
        case mFilterCutoff:
            changer = bind(&VoiceManager::setFilterCutoff, _1, param->Value());
            break;
        case mFilterResonance:
            changer = bind(&VoiceManager::setFilterResonance, _1, param->Value());
            break;
        case mlfoFilterModAmount:
            changer = bind(&VoiceManager::setFilterLFOAmount, _1, param->Value());
            break;
        case mFilterEnvelopeAmount:
            changer = bind(&VoiceManager::setFilterEnvAmount, _1, param->Value());
            break;
            // Volume Envelope:
        case mAttack:
            changer = bind(&VoiceManager::setVolumeEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_ATTACK, param->Value());
            break;
        case mDecay:
            changer = bind(&VoiceManager::setVolumeEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_DECAY, param->Value());
            break;
        case mSustain:
            changer = bind(&VoiceManager::setVolumeEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_SUSTAIN, param->Value());
            break;
        case mRelease:
            changer = bind(&VoiceManager::setVolumeEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_RELEASE, param->Value());
            break;
            // Filter Envelope:
        case mFilterAttack:
            changer = bind(&VoiceManager::setFilterEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_ATTACK, param->Value());
            break;
        case mFilterDecay:
            changer = bind(&VoiceManager::setFilterEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_DECAY, param->Value());
            break;
        case mFilterSustain:
            changer = bind(&VoiceManager::setFilterEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_SUSTAIN, param->Value());
            break;
        case mFilterRelease:
            changer = bind(&VoiceManager::setFilterEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_RELEASE, param->Value());
            break;
      }
      voiceManager.changeAllVoices(changer);
  }
}


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