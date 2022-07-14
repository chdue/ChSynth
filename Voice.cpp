#include "Voice.h"

double Voice::nextSample() {
    if (!isActive) return 0.0;

    double oscillatorOutput = mOscillator.nextSample();

    double volumeEnvelopeValue = mVolumeEnvelope.nextSample();
    double filterEnvelopeValue = mFilterEnvelope.nextSample();

    mFilter.setCutoffMod(filterEnvelopeValue * mFilterEnvelopeAmount + mLFOValue * mFilterLFOAmount);

    mOscillator.setPitchMod(mLFOValue * mOscillatorPitchAmount);
    return mFilter.process(oscillatorOutput * volumeEnvelopeValue * mVelocity / 127.0);
}

void Voice::setFree() {
    isActive = false;
}

void Voice::reset() {
    mNoteNumber = -1;
    mVelocity = 0;
    mOscillator.reset();
    mVolumeEnvelope.reset();
    mFilterEnvelope.reset();
    mFilter.reset();
}