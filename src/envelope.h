//
//  envelope.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef envelope_h
#define envelope_h

#define DEFAULT_ATTACK (1000)
#define DEFAULT_DECAY (10000)
#define DEFAULT_SUSTAIN (1000)
#define DEFAULT_RELEASE (50000)
#define DEFAULT_SUSTAIN_LEVEL (0.5)

#include "littledaw.h"

// Finite Envelope
class Envelope {
    // envelope parameters
    int node1, node2, node3;
    float sustain_level;
    bool continuous;
public:
    int length; // in samples
    Envelope(bool continuous=false,
             int a=DEFAULT_ATTACK, 
             int d=DEFAULT_DECAY, 
             int s=DEFAULT_SUSTAIN, 
             int r=DEFAULT_RELEASE, 
             float sustain_level=DEFAULT_SUSTAIN_LEVEL);
    float calculate(int, bool);
};

#endif /* envelope_h */