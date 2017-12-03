//
//  synth.cpp
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "synth.h"

/*
 Synth default constructor
*/
Synth::Synth(int num_v) {
    this->curr_voice = 0;
    this->num_voices = num_v;
    this->voices = new Voice[this->num_voices];
    this->envelope = new Envelope((this->num_voices == 1));
}

/*
 Synth destructor
*/
Synth::~Synth() {
    delete [] this->voices;
    delete this->envelope;
}

/*
 Trigger a note
   TAKES:
     note --> the note to trigger
*/
int Synth::trigger_note(float note) {
    if(this->voices[this->curr_voice].is_triggered()) {
        return 1; // No free voices
    } else {
        this->voices[this->curr_voice].trigger(note, this->num_voices);
        this->curr_voice++;
        this->curr_voice %= this->num_voices;
        return 0;
    }
}

/*
 Advance all voices.
*/
void Synth::advance() {
    int i;
    
    // advance voices
    for(i = 0; i < this->num_voices; i++) {
        this->voices[i].advance(this->envelope->length);
    }
}

/*
 Combine all voices on specified channel
   TAKES:
     chann --> int channel to mix
   RETURNS:
     float added voices
*/
float Synth::output(int chann) {
    float out = 0.0;
    int i;
    float voice_signal;
    float envelope_signal;
    
    for(i = 0; i < this->num_voices; i++) {
        voice_signal = this->table.table[(int)(this->voices[i].wavetable_pos[chann])];
        envelope_signal = this->envelope->calculate(this->voices[i].envelope_pos, 
                                                    this->voices[i].is_triggered());
        out += (voice_signal * envelope_signal);
    }
    return out;
}
