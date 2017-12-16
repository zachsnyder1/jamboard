//
//  mixer.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef mixer_h
#define mixer_h

#include "littledaw.h"
#include "instrument.h"
#include <vector>

#define FADE_INCREMENT (0.00003)

// Mixer base class
class Mixer {
    float master;
    bool fadein;
    bool fadeout;
public:
    Mixer();
    void fade_in();
    void fade_out();
    void wait_for_fade();
    float mix(int, std::vector<Instrument*>);
    void advance();
};

#endif /* mixer_h */