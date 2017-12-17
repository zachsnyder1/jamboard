//
//  envelope.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef envelope_h
#define envelope_h

#include "littledaw.h"

class EnvelopeConstants {
public:
    static const int DEFAULT_ATTACK = 1000;
    static const int DEFAULT_DECAY = 10000;
    static const int DEFAULT_SUSTAIN = 1000;
    static const int DEFAULT_RELEASE = 50000;
    constexpr static const float DEFAULT_SUSTAIN_LEVEL = 0.5;
};

// Finite Envelope
class Envelope : public EnvelopeConstants{
    // envelope parameters
    int node1, node2, node3;
    float sustain_level;
public:
    int length; // in samples
    Envelope(int a=Envelope::DEFAULT_ATTACK, 
             int d=Envelope::DEFAULT_DECAY, 
             int s=Envelope::DEFAULT_SUSTAIN, 
             int r=Envelope::DEFAULT_RELEASE, 
             float sustain_level=Envelope::DEFAULT_SUSTAIN_LEVEL);
    float calculate(int, bool);
};

#endif /* envelope_h */