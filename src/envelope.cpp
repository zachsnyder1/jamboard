//
//  envelope.cpp
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "envelope.h"

/*
 Envelope default constructor
*/
Envelope::Envelope() {
    this->triggered = false;
    this->curr_position = 0;
    this->level = (float)0.0;
}

/*
 Get envelope level
*/
float Envelope::get_level() {
    return this->level;
}

/*
 Abstract method to trigger the envelope.
*/
void Envelope::trigger() {
    return;
}

/*
 Abstract method to advance envelope
*/
void Envelope::advance() {
    return;
}


/*
 InfiniteEnvelope default constructor
*/
InfiniteEnvelope::InfiniteEnvelope() {
    this->level = (float)1.0;
}

/*
 FiniteEnvelope constructor
   TAKES:
     a --> attack length in samples
     d --> decay length in samples
     s --> sustain length in samples
     r --> release length in samples
     sustain_level --> 0 <= x <= 100
     
*/
FiniteEnvelope::FiniteEnvelope(int a, 
                               int d, 
                               int s, 
                               int r, 
                               float sustain_level) {
    this->length = a + d + s + r;
    this->node1 = a;
    this->node2 = node1 + d;
    this->node3 = node2 + s;
    this->sustain_level = sustain_level;
}

/*
 Set envelope level based on envelope parameters and this->curr_position.
*/
void FiniteEnvelope::update_level() {
    double node_height;
    int node_index;
    double progress;
    float l;
    
    if(this->triggered) {
        if(this->curr_position < this->node1) {
            node_height = (double)1.0;
            node_index = this->curr_position;
            progress = (double)node_index / (double)this->node1;
            this->level = (float)(progress * node_height);
        } else if(this->curr_position < this->node2) {
            node_height = (double)1.0 - (double)this->sustain_level;
            node_index = this->curr_position - this->node1;
            progress = (double)node_index / ((double)(this->node2 - this->node1));
            this->level = 1.0 - (progress * node_height);
        } else if(this->curr_position < this->node3) {
            this->level = this->sustain_level;
        } else {
            node_height = (double)this->sustain_level;
            node_index = this->curr_position - this->node3;
            progress = (double)node_index / 
                       ((double)(this->length - this->node3));
            this->level = this->sustain_level - (progress * node_height);
        }
    } else {
        this->level = 0.0;
    }
}

/*
 Concrete method to trigger the envelope.
*/
void FiniteEnvelope::trigger() {
    this->triggered = true;
}

/*
 Concrete method to advance the envelope.
*/
void FiniteEnvelope::advance() {
    if(triggered) {
        if(this->curr_position >= this->length) {
            this->triggered = false;
            this->curr_position = 0;
        } else {
            this->curr_position++;
        }
    }
    this->update_level();
}
