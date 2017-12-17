//
//  instrument.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef instrument_h
#define instrument_h

#include "littledaw.h"
#include "voice.h"
#include "envelope.h"
#include "wavetable.h"
#include <vector>


class InstrumentConstants {
public:
    // NOTE CONSTANTS:
    static const int A1 = 12;
    static const int AS1 = 13;
    static const int B1 = 14;
    static const int C1 = 15;
    static const int CS1 = 16;
    static const int D1 = 17;
    static const int DS1 = 18;
    static const int E1 = 19;
    static const int F1 = 20;
    static const int FS1 = 21;
    static const int G1 = 22;
    static const int GS1 = 23;
    static const int A2 = 24;
    static const int AS2 = 25;
    static const int B2 = 26;
    static const int C2 = 27;
    static const int CS2 = 28;
    static const int D2 = 29;
    static const int DS2 = 30;
    static const int E2 = 31;
    static const int F2 = 32;
    static const int FS2 = 33;
    static const int G2 = 34;
    static const int GS2 = 35;
    static const int A3 = 36;
    static const int AS3 = 37;
    static const int B3 = 38;
    static const int C3 = 39;
    static const int CS3 = 40;
    static const int D3 = 41;
    static const int DS3 = 42;
    static const int E3 = 43;
    static const int F3 = 44;
    static const int FS3 = 45;
    static const int G3 = 46;
    static const int GS3 = 47;
    static const int A4 = 48;
    static const int AS4 = 49;
    static const int B4 = 50;
    static const int C4 = 51;
    static const int CS4 = 52;
    static const int D4 = 53;
    static const int DS4 = 54;
    static const int E4 = 55;
    static const int F4 = 56;
    static const int FS4 = 57;
    static const int G4 = 58;
    static const int GS4 = 59;
    static const int A5 = 60;
    static const int AS5 = 61;
    static const int B5 = 62;
    static const int C5 = 63;
    static const int CS5 = 64;
    static const int D5 = 65;
    static const int DS5 = 66;
    static const int E5 = 67;
    static const int F5 = 68;
    static const int FS5 = 69;
    static const int G5 = 70;
    static const int GS5 = 71;
};

// Instrument abstract base class
class Instrument : public InstrumentConstants {
protected:
    std::vector<Voice*> voices;
    Envelope *envelope;
    int curr_voice;
public:
    // CONSTR. DESTR.
    Instrument(int num_v=DEFAULT_NUM_VOICES);
    ~Instrument();
    // abstract interface
    virtual int trigger_note(const int) { return 1; };
    virtual void advance() {};
    virtual float output(int) { return 0.0; };
    virtual void command(const int, void*) {};
};

class WaveTableSynthConstants {
protected:
    float BASE_HZ[12] = {27.5, 29.14, 30.87, 32.7, 34.65, 36.71, 38.89, 
                       41.2, 43.65, 46.25, 49, 51.91};
public:
    // COMMAND CONSTANTS
    static const int COMMAND_SINE_WAVE = 100;
    static const int COMMAND_SQUARE_WAVE = 101;
    static const int COMMAND_CUSTOM_WAVE = 102;
};

// Synth instrument class
class WaveTableSynth : public Instrument, public WaveTableSynthConstants {
    WaveTable table;
    float *wavetable_positions;
    float *pitch_incrementers;
    // helper method(s)
    float calculate_note(const int);
public:
    // PUBLIC METHODS
    WaveTableSynth(int num_v=DEFAULT_NUM_VOICES);
    ~WaveTableSynth();
    // Instrument interface overrides
    int trigger_note(const int);
    void advance();
    float output(int);
    void command(const int, void*);
};

#endif /* instrument_h */