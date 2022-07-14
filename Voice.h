#ifndef __VOICE__
#define __VOICE__

#include "Oscillator.h"
#include "EnvelopeGenerator.h"
#include "Filter.h"

class Voice {
public:
    friend class VoiceManager;
    Voice()
    : mNoteNumber(-1),
    mVelocity(0),
    mFilterEnvelopeAmount(0.0),
    mFilterLFOAmount(0.0),
    mOscillatorPitchAmount(0.0),
    mLFOValue(0.0),
    isActive(false) {
        // Set myself free everytime my volume envelope has fully faded out of RELEASE stage:
        mVolumeEnvelope.finishedEnvelopeCycle.Connect(this, &Voice::setFree);
    };

    inline void setFilterEnvelopeAmount(double amount) { mFilterEnvelopeAmount = amount; }
    inline void setFilterLFOAmount(double amount) { mFilterLFOAmount = amount; }
    inline void setOscillatorPitchAmount(double amount) { mOscillatorPitchAmount = amount; }
    inline void setLFOValue(double value) { mLFOValue = value; }

    inline void setNoteNumber(int noteNumber) {
        mNoteNumber = noteNumber;
        double frequency = 440.0 * pow(2.0, (mNoteNumber - 69.0) / 12.0);
        mOscillator.setFrequency(frequency);
    }
    double nextSample();
    void setFree();
    void reset();
private:
    Oscillator mOscillator;
    EnvelopeGenerator mVolumeEnvelope;
    EnvelopeGenerator mFilterEnvelope;
    Filter mFilter;

    int mNoteNumber;
    int mVelocity;

    double mFilterEnvelopeAmount;
    double mFilterLFOAmount;
    double mOscillatorPitchAmount;
    double mLFOValue;

    bool isActive;
};

#endif