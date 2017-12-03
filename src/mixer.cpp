//
//  mixer.cpp
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "mixer.h"

/*
 Mixer default constructor
*/
Mixer::Mixer() {
    this->master = 0.0;
    this->fadein = false;
    this->fadeout = false;
}

/*
 Fade in signal amplitude
*/
void Mixer::fade_in() {
    if(this->fadeout) this->fadeout = false;
    this->fadein = true;
    this->wait_for_fade();
}

/*
 Fade out signal amplitude
*/
void Mixer::fade_out() {
    if(this->fadein) this->fadein = false;
    this->fadeout = true;
    this->wait_for_fade();
}

/*
 Waits for fade
*/
void Mixer::wait_for_fade() {
    
}

/*
 Mix all tracks on specified channel
   TAKES:
     chann --> int channel to mix
     table --> ptr WaveTable object
     tracks --> array of Track objects
   RETURNS:
     float mixed track signals
*/
float Mixer::mix(int chann, Synth *synths) {
    float out = 0;
    int i;
    
    // for each infinite synth
    for(i = 0; i < (sizeof(synths) / sizeof(Synth)); i++) {
        out += synths[i].output(chann);
    }
    return (out * this->master);
}

/*
 Handle fading.
*/
void Mixer::advance() {
    int i;

    if(this->fadein && (this->master < 1.0)) {
        this->master += FADE_INCREMENT;
    } else {
        this->fadein = false;
    }
    if(this->fadeout && (this->master > 0.0)) {
        this->master -= FADE_INCREMENT;
    } else {
        this->fadeout = false;
    }
}
