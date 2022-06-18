#ifndef __MYFIRSTDISTORTION__
#define __MYFIRSTDISTORTION__

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"

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
  void CreatePresets();
  Oscillator mOscillator;
  MIDIReceiver mMIDIReceiver;
};

#endif
