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
template<class EnvelopeType>
Mixer<EnvelopeType>::Mixer(int num_v) {
    this->level = 0.0;
    this->curr_voice = 0;
    this->fadein = false;
    this->fadeout = false;
    this->num_voices = num_v;
    this->voices = new Voice<EnvelopeType>[num_voices];
}

/*
 Fade in signal amplitude
*/
template<class EnvelopeType>
void Mixer<EnvelopeType>::fade_in() {
    if(this->fadeout) this->fadeout = false;
    this->fadein = true;
    this->wait_for_fade();
}

/*
 Fade out signal amplitude
*/
template<class EnvelopeType>
void Mixer<EnvelopeType>::fade_out() {
    if(this->fadein) this->fadein = false;
    this->fadeout = true;
    this->wait_for_fade();
}

/*
 Waits for fade
*/
template<class EnvelopeType>
void Mixer<EnvelopeType>::wait_for_fade() {
    while(this->fadein || this->fadeout){}
}

/*
 Trigger a note
   TAKES:
     note --> the note to trigger
*/
template<class EnvelopeType>
int Mixer<EnvelopeType>::trigger_note(float note) {
    if(this->voices[this->curr_voice].is_triggered()) {
        return 1; // No free voices
    } else {
      this->voices[this->curr_voice].trigger(note);
      this->curr_voice++;
      this->curr_voice %= this->num_voices;
      return 0;
    }
}

/*
 Mix all voices on specified channel
   TAKES:
     chann --> int channel to mix
   RETURNS:
     float added voices
*/
template<class EnvelopeType>
float Mixer<EnvelopeType>::mix(int chann, WaveTable *table) {
    float master = 0;
    int i;
    
    for(i = 0; i < this->num_voices; i++) {
        master += this->voices[i].get_signal(chann, table);
    }
    master *= this->level;
    return master;
}

/*
 Advance all voices.
*/
template<class EnvelopeType>
void Mixer<EnvelopeType>::advance() {
    int i;
    
    // advance voices
    for(i = 0; i < this->num_voices; i++) {
        this->voices[i].advance();
    }
    // handle fade
    if(this->fadein && (this->level < 1.0)) {
        this->level += 0.00003;
    } else {
        this->fadein = false;
    }
    if(this->fadeout && (this->level > 0.0)) {
        this->level -= 0.00003;
    } else {
        this->fadeout = false;
    }
}

template class Mixer<InfiniteEnvelope>;
template class Mixer<FiniteEnvelope>;
