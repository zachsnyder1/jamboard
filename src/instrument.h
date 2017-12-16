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

/* Define base pitches so that the pitch can be calculated for any note in any
 octave using the relatively simple switch board in each engine. */
static float BASE_HZ[] = {27.5, 29.14, 30.87, 32.7, 34.65, 36.71, 38.89, 41.2,
    43.65, 46.25, 49, 51.91};

// Instrument abstract base class
class Instrument {
public:
    // NOTE CONSTANTS:
    static const int A1 = 0;
    static const int AS1 = 1;
    static const int B1 = 2;
    static const int C1 = 3;
    static const int CS1 = 4;
    static const int D1 = 5;
    static const int DS1 = 6;
    static const int E1 = 7;
    static const int F1 = 8;
    static const int FS1 = 9;
    static const int G1 = 10;
    static const int GS1 = 11;
    static const int A2 = 12;
    static const int AS2 = 13;
    static const int B2 = 14;
    static const int C2 = 15;
    static const int CS2 = 16;
    static const int D2 = 17;
    static const int DS2 = 18;
    static const int E2 = 19;
    static const int F2 = 20;
    static const int FS2 = 21;
    static const int G2 = 22;
    static const int GS2 = 23;
    static const int A3 = 24;
    static const int AS3 = 25;
    static const int B3 = 26;
    static const int C3 = 27;
    static const int CS3 = 28;
    static const int D3 = 29;
    static const int DS3 = 30;
    static const int E3 = 31;
    static const int F3 = 32;
    static const int FS3 = 33;
    static const int G3 = 34;
    static const int GS3 = 35;
    static const int A4 = 36;
    static const int AS4 = 37;
    static const int B4 = 38;
    static const int C4 = 39;
    static const int CS4 = 40;
    static const int D4 = 41;
    static const int DS4 = 42;
    static const int E4 = 43;
    static const int F4 = 44;
    static const int FS4 = 45;
    static const int G4 = 46;
    static const int GS4 = 47;
    static const int A5 = 48;
    static const int AS5 = 49;
    static const int B5 = 50;
    static const int C5 = 51;
    static const int CS5 = 52;
    static const int D5 = 53;
    static const int DS5 = 54;
    static const int E5 = 55;
    static const int F5 = 56;
    static const int FS5 = 57;
    static const int G5 = 58;
    static const int GS5 = 59;
    // helper method(s)
    float calculate_note(const int);
    // abstract interface
    virtual int trigger_note(const int) { return 1; };
    virtual void advance() {};
    virtual float output(int) { return 0.0; };
    virtual void command(const int, void*) {};
};


// Synth instrument class
class WaveTableSynth : public Instrument {
    std::vector<Voice*> voices;
    Envelope *envelope;
    WaveTable table;
    int curr_voice;
public:
    // COMMAND CONSTANTS
    static const int COMMAND_SINE_WAVE = 100;
    static const int COMMAND_SQUARE_WAVE = 101;
    static const int COMMAND_CUSTOM_WAVE = 102;
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