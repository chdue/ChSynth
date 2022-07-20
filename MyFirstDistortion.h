#ifndef __MYFIRSTDISTORTION__
#define __MYFIRSTDISTORTION__

// much of this project was made following a guide to wdl-ol:
//    martin-finke.de/blog/tags/making_audio_plugins.html

#include "IPlug_include_in_plug_hdr.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"

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
  MIDIReceiver mMIDIReceiver;

  IControl* mVirtualKeyboard;
  void processVirtualKeyboard();

  VoiceManager voiceManager;
};

#endif
