//
//  envelope.cpp
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "envelope.h"

/*
 Envelope constructor
   TAKES:
     continuous --> indicates continuous envelope
     a --> attack length in samples
     d --> decay length in samples
     s --> sustain length in samples
     r --> release length in samples
     sustain_level --> 0 <= x <= 100
     
*/
Envelope::Envelope(int a, 
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
 Set envelope level based on envelope parameters and pos.
*/
float Envelope::calculate(int pos, bool triggered) {
    double node_height;
    int node_index;
    double progress;
    float level;
    
    if(triggered) {
        if(pos < this->node1) {
            node_height = (double)1.0;
            node_index = pos;
            progress = (double)node_index / (double)this->node1;
            return (float)(progress * node_height);
        } else if(pos < this->node2) {
            node_height = (double)1.0 - (double)this->sustain_level;
            node_index = pos - this->node1;
            progress = (double)node_index / ((double)(this->node2 - this->node1));
            return 1.0 - (progress * node_height);
        } else if(pos < this->node3) {
            return this->sustain_level;
        } else {
            node_height = (double)this->sustain_level;
            node_index = pos - this->node3;
            progress = (double)node_index / 
                       ((double)(this->length - this->node3));
            return this->sustain_level - (progress * node_height);
        }
    } else {
        return 0.0;
    }
}
