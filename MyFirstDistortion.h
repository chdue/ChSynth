#ifndef __MYFIRSTDISTORTION__
#define __MYFIRSTDISTORTION__

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"
#include "EnvelopeGenerator.h"

class MyFirstDistortion : public IPlug
{
public:
  MyFirstDistortion(IPlugInstanceInfo instanceInfo);
  ~MyFirstDistortion();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

  // to receive MIDI messages:
  void ProcessMidiMsg(IMidiMsg* pMsg);

private:
  double mFrequency;
  double mGain;
  void CreatePresets();
  Oscillator mOscillator;
  MIDIReceiver mMIDIReceiver;
  EnvelopeGenerator mEnvelopeGenerator;

  inline void onNoteOn(const int noteNumber, const int velocity) { mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK); };
  inline void onNoteOff(const int noteNumber, const int velocity) { mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE); };
};

#endif
