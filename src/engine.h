//
//  engine.h
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef engine_h
#define engine_h

#include "jamboard.h"
#include "wavetable.h"
#include "mixer.h"
#include "envelope.h"
#include "ui.h"
#include "synth.h"

/* Define base pitches so that the pitch can be calculated for any note in any
 octave using the relatively simple switch board in each engine. */
static float BASE_HZ[] = {27.5, 29.14, 30.87, 32.7, 34.65, 36.71, 38.89, 41.2,
    43.65, 46.25, 49, 51.91};

// Engine class
class Engine {
protected:
    // ----- ATTRIBUTES -----
    // jamboard objects
    Synth *synths;
    Mixer *mixer;
    UserInterface ui;
    // portaudio objects
    PaStreamParameters *outputParameters; //struct for stream parameters
    PaStream *stream;
    PaError err;
    // housekeeping
    float calculate_note(int, int);
    void error();
    void end();
public:
    // ----- USER METHODS -----
    Engine(int num_v=DEFAULT_NUM_VOICES, float start_note=START_NOTE);
    void run();
    // ----- PORTAUDIO CALLBACK METHODS -----
    static int callback(const void*,
                        void*,
                        unsigned long,
                        const PaStreamCallbackTimeInfo *,
                        PaStreamCallbackFlags,
                        void *);
};

#endif /* engine_h */
