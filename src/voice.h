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
    bool triggered;
    float pitch_incrementer;
public:
    // attributes
    int envelope_pos;
    float wavetable_pos[NUM_CHANNELS];
    // methods
    Voice(float=START_NOTE);
    void advance(int);
    void trigger(float, int);
    bool is_triggered();
};

#endif /* voice_h */