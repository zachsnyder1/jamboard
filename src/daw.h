//
//  daw.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef daw_h
#define daw_h

#include "littledaw.h"
#include "wavetable.h"
#include "mixer.h"
#include "envelope.h"
#include "controller.h"
#include "instrument.h"
#include <vector>

struct Mapping;

class Daw {
protected:
    // portaudio objects
    PaStreamParameters *outputParameters; //struct for stream parameters
    PaStream *stream;
    PaError err;
    // housekeeping
    void error();
    void end();
public:
    // ----- ATTRIBUTES -----
    std::vector<Mapping*> mappings;
    std::vector<Controller*> controllers;
    std::vector<Instrument*> instruments;
    Mixer *mixer;
    // ----- USER METHODS -----
    Daw();
    ~Daw();
    void add_instrument(Instrument*);
    void add_controller(Controller*);
    void map_controller(Controller*, Instrument*);
    void run();
    // ----- PORTAUDIO CALLBACK METHODS -----
    static int callback(const void*,
                        void*,
                        unsigned long,
                        const PaStreamCallbackTimeInfo *,
                        PaStreamCallbackFlags,
                        void *);
};

#endif /* daw_h */
