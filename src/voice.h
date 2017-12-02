//
//  voice.h
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef voice_h
#define voice_h

#include "jamboard.h"
#include "wavetable.h"
#include "envelope.h"

// Voice base class
template <class EnvelopeType>
class Voice {
    float pitch_incrementer;
    float channel_positions[NUM_CHANNELS];
    EnvelopeType envelope;
public:
    Voice();
    Voice(float);
    float get_signal(int, WaveTable*);
    void advance();
    void trigger(float);
    bool is_triggered();
};

#endif /* voice_h */