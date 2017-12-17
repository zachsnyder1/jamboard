//
//  voice.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef voice_h
#define voice_h

#include "littledaw.h"

// Voice base class
class Voice {
    bool triggered;
public:
    // attributes
    int envelope_pos;
    // methods
    Voice();
    void advance(int);
    void trigger();
    bool is_triggered();
};

#endif /* voice_h */