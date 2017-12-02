//
//  envelope.h
//  jamboard
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

#include "jamboard.h"

template <class EnvelopeType> class Voice; // forward declaration for friend statment

// Envelope base class
class Envelope {
protected:
    template <class EnvelopeType> friend class Voice;
    bool triggered;
    int curr_position;
    float level;
public:
    Envelope();
    float get_level();
    // ABSTRACT METHODS
    void trigger();
    void advance();
};

// Infinite Envelope
class InfiniteEnvelope : public Envelope {
public:
    InfiniteEnvelope();
};

// Finite Envelope
class FiniteEnvelope : public Envelope {
    // envelope parameters
    int length; // in samples
    int node1;
    int node2;
    int node3;
    float sustain_level;
public:
    FiniteEnvelope(int a=DEFAULT_ATTACK, 
                   int d=DEFAULT_DECAY, 
                   int s=DEFAULT_SUSTAIN, 
                   int r=DEFAULT_RELEASE, 
                   float sustain_level=DEFAULT_SUSTAIN_LEVEL);
    void update_level();
    // CONCRETE METHODS
    void trigger();
    void advance();
};

#endif /* envelope_h */