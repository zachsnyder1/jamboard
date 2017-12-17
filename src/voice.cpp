//
//  voice.cpp
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "voice.h"

Voice::Voice() {
    this->triggered = false;
    this->envelope_pos = 0;
}

void Voice::advance(int envelope_len) {
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

void Voice::trigger() {
    this->triggered = true;
}

bool Voice::is_triggered() {
    return this->triggered;
}
