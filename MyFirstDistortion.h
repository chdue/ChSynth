#ifndef __MYFIRSTDISTORTION__
#define __MYFIRSTDISTORTION__

// much of this project was made following a guide to wdl-ol:
//    martin-finke.de/blog/tags/making_audio_plugins.html

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"
#include "EnvelopeGenerator.h"
#include "Filter.h"

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

  // Needed for the GUI keyboard:
  // Should return non-zero if one or more keys are playing.
  inline int GetNumKeys() const { return mMIDIReceiver.getNumKeys(); };
  // Should return true if the specified key is playing.
  inline bool GetKeyStatus(int key) const { return mMIDIReceiver.getKeyStatus(key); };
  static const int virtualKeyboardMinimumNoteNumber = 36;
  int lastVirtualKeyboardNoteNumber;

private:
  double mFrequency;
  double mGain;
  Oscillator mOscillator;
  MIDIReceiver mMIDIReceiver;
  EnvelopeGenerator mEnvelopeGenerator;

  inline void onNoteOn(const int noteNumber, const int velocity) { mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK); };
  inline void onNoteOff(const int noteNumber, const int velocity) { mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE); };

  IControl* mVirtualKeyboard;
  void processVirtualKeyboard();

  inline void onBeganEnvelopeCycle() { mOscillator.setMuted(false); }
  inline void onFinishedEnvelopeCycle() { mOscillator.setMuted(true); }

  Filter mFilter;

  Oscillator mLFO;
  double lfoFilterModAmount;
};

#endif
