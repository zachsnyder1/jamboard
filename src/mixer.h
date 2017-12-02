//
//  mixer.h
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef mixer_h
#define mixer_h

#include "jamboard.h"
#include "voice.h"
#include "envelope.h"
#include "wavetable.h"

// Mixer base class
template<class EnvelopeType>
class Mixer {
    Voice<EnvelopeType> *voices;
    float level;
    int curr_voice;
    int num_voices;
    // fade controls
    bool fadein;
    bool fadeout;
public:
    Mixer(int num_v=DEFAULT_NUM_VOICES);
    void fade_in();
    void fade_out();
    void wait_for_fade();
    int trigger_note(float);
    float mix(int, WaveTable*);
    void advance();
};

#endif /* mixer_h */