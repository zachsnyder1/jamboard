//
//  voice.cpp
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "voice.h"

Voice::Voice(float start_note) {
    this->pitch_incrementer = start_note;
    this->triggered = false;
    this->envelope_pos = 0;
    for(int i = 0; i < NUM_CHANNELS; i++){
        this->wavetable_pos[i] = 0.0;
    }
}

void Voice::advance(int envelope_len) {
    int i;

    // advance channel positions
    for(i = 0; i < NUM_CHANNELS; i++) {
        this->wavetable_pos[i] += this->pitch_incrementer;
        if(this->wavetable_pos[i] > TABLE_SIZE) {
            this->wavetable_pos[i] -=  TABLE_SIZE;
        }
    }
    // advance envelope position
    if(this->triggered) {
        if(this->envelope_pos >= envelope_len) {
            this->triggered = false;
            this->envelope_pos = 0;
        } else {
            this->envelope_pos++;
        }
    }
}

void Voice::trigger(float note, int num_v) {
    this->pitch_incrementer = note;
    this->triggered = true;
}

bool Voice::is_triggered() {
    return this->triggered;
}
