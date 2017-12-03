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

// Voice base class
class Voice {
public:
    // attributes
    bool triggered;
    float pitch_incrementer;
    float wavetable_pos[NUM_CHANNELS];
    int envelope_pos;
    // methods
    Voice(float=START_NOTE);
    void advance(int);
    void trigger(float);
};

#endif /* voice_h */