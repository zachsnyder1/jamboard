//
//  voice.cpp
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "voice.h"

template<class EnvelopeType>
Voice<EnvelopeType>::Voice() {
    this->pitch_incrementer = START_NOTE;
}

template<class EnvelopeType>
Voice<EnvelopeType>::Voice(float start_note) {
    this->pitch_incrementer = start_note;
}

template<class EnvelopeType>
float Voice<EnvelopeType>::get_signal(int chann, WaveTable *table) {
    float signal = table->table[(int)this->channel_positions[chann]];
    return (signal * this->envelope.get_level());
}

template<class EnvelopeType>
void Voice<EnvelopeType>::advance() {
    int i;
    
    // advance channel positions
    for(i = 0; i < NUM_CHANNELS; i++) {
        this->channel_positions[i] += this->pitch_incrementer;
        if(this->channel_positions[i] > TABLE_SIZE) {
            this->channel_positions[i] -=  TABLE_SIZE;
        }
    }
    // advance envelope
    this->envelope.advance();
}

template<class EnvelopeType>
void Voice<EnvelopeType>::trigger(float note) {
    this->pitch_incrementer = note;
    this->envelope.trigger();
}

template<class EnvelopeType>
bool Voice<EnvelopeType>::is_triggered() {
    return this->envelope.triggered;
}

template class Voice<InfiniteEnvelope>;
template class Voice<FiniteEnvelope>;
