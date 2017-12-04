//
//  wavetable.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef wavetable_h
#define wavetable_h

#include "littledaw.h"

/*
 Class WaveTable:
   Holds a wave table for audio synthesis, and can populate
   itself with certain waveforms
     * sine wave
     * pseudo square wave
     * custom wave (user defined)
*/
class WaveTable {
public:
    float *table; // table for wavetable synthesis
    int *harmonic_amplitudes; // for custom synth
    WaveTable(); // constructor
    // wave table methods
    void sine_wave();
    void square_wave();
    void custom_wave();
};

#endif /* wavetable_h */
