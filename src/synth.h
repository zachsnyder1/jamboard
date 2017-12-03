//
//  synth.h
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef synth_h
#define synth_h

#include "jamboard.h"
#include "voice.h"
#include "envelope.h"
#include "wavetable.h"

// Synth base class
class Synth {
    Voice *voices;
    Envelope *envelope;
    int curr_voice;
    int num_voices;
public:
    WaveTable table;
    Synth(int num_v=DEFAULT_NUM_VOICES);
    int trigger_note(float);
    void advance();
    float output(int);
};

#endif /* synth_h */