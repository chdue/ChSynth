#include "Oscillator.h"
#include <stdlib.h>
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

void Oscillator::setMode(OscillatorMode mode) {
    mOscillatorMode = mode;
}

void Oscillator::setFrequency(double frequency) {
    mFrequency = frequency;
    updateIncrement();
}

void Oscillator::setSampleRate(double sampleRate) {
    mSampleRate = sampleRate;
    updateIncrement();
}

void Oscillator::updateIncrement() {
    mPhaseIncrement = mFrequency * 2 * mPI / mSampleRate;
}

void Oscillator::generate(double* buffer, int nFrames) {
    const double twoPI = 2 * mPI;
    switch (mOscillatorMode) {
    case OSCILLATOR_MODE_SINE:
        for (int i = 0; i < nFrames; i++) {
            buffer[i] = sin(mPhase);
            mPhase += mPhaseIncrement;
            while (mPhase >= twoPI) {
                mPhase -= twoPI;
            }
        }
        break;
    case OSCILLATOR_MODE_SAW:
        for (int i = 0; i < nFrames; i++) {
            buffer[i] = 1.0 - (2.0 * mPhase / twoPI);
            mPhase += mPhaseIncrement;
            while (mPhase >= twoPI) {
                mPhase -= twoPI;
            }
        }
        break;
    case OSCILLATOR_MODE_SQUARE:
        for (int i = 0; i < nFrames; i++) {
            if (mPhase <= mPI) {
                buffer[i] = 1.0;
            }
            else {
                buffer[i] = -1.0;
            }
            mPhase += mPhaseIncrement;
            while (mPhase >= twoPI) {
                mPhase -= twoPI;
            }
        }
        break;
    case OSCILLATOR_MODE_TRIANGLE:
        for (int i = 0; i < nFrames; i++) {
            double value = -1.0 + (2.0 * mPhase / twoPI);
            buffer[i] = 2.0 * (fabs(value) - 0.5);
            mPhase += mPhaseIncrement;
            while (mPhase >= twoPI) {
                mPhase -= twoPI;
            }
        }
        break;
    case OSCILLATOR_MODE_RANDOM:
        for (int i = 0; i < nFrames; i++) {
            srand(time(0));
            buffer[i] = rand() % 200 - 100; // random between (-1, 1) TODO
            buffer[i] /= 100;
            mPhase += mPhaseIncrement;
            while (mPhase >= twoPI) {
                mPhase -= twoPI;
            }
        }
        break;
    case OSCILLATOR_MODE_ROUNDED_SQUARE:
        for (int i = 0; i < nFrames; i++) {
            // 0 < mPI < twoPI
            if (mPhase >= (mPI * 1.1) && mPhase <= (mPI * 1.9)) {
                buffer[i] = 1.0;
            }
            else if (mPhase >= (mPI * 0.1) && mPhase <= (mPI * 0.9)) {
                buffer[i] = -1.0;
            }
            else {
                buffer[i] = sin(-5 * M_PI * mPhase);
            }

            mPhase += mPhaseIncrement;
            while (mPhase >= twoPI) {
                mPhase -= twoPI;
            }
        }
        break;
    }
}

double Oscillator::nextSample() {
    double value = 0.0;
    if (isMuted) return value;

    switch (mOscillatorMode) {
    case OSCILLATOR_MODE_SINE:
        value = sin(mPhase);
        break;
    case OSCILLATOR_MODE_SAW:
        value = 1.0 - (2.0 * mPhase / twoPI);
        break;
    case OSCILLATOR_MODE_SQUARE:
        if (mPhase <= mPI) {
            value = 1.0;
        }
        else {
            value = -1.0;
        }
        break;
    case OSCILLATOR_MODE_TRIANGLE:
        value = -1.0 + (2.0 * mPhase / twoPI);
        value = 2.0 * (fabs(value) - 0.5);
        break;
    case OSCILLATOR_MODE_RANDOM: // fix this TODO
        srand(time(0));
        value = rand() % 200 - 100;
        value /= 100;
        break;
    case OSCILLATOR_MODE_ROUNDED_SQUARE:
        if (mPhase >= (mPI * 1.1) && mPhase <= (mPI * 1.9)) {
            value = 1.0;
        }
        else if (mPhase >= (mPI * 0.1) && mPhase <= (mPI * 0.9)) {
            value = -1.0;
        }
        else {
            value = sin(-5 * M_PI * mPhase);
        }
        break;
    }
    mPhase += mPhaseIncrement;
    while (mPhase >= twoPI) {
        mPhase -= twoPI;
    }
    return value;
}