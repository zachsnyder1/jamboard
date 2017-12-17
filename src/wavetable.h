//
//  wavetable.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef wavetable_h
#define wavetable_h

#include <math.h>
//#include "littledaw.h"

class WaveTableConstants {
public:
    static const int TABLE_SIZE = 400;
    static const int HIGHEST_HARMONIC = 10;
    constexpr static const float SINE_MAX_AMP = 0.5;
    constexpr static const float CUSTOM_MAX_AMP = 0.01;
    constexpr static const float SQUARE_MAX_AMP = 0.05;
};

/*
 Class WaveTable:
   Holds a wave table for audio synthesis, and can populate
   itself with certain waveforms
     * sine wave
     * pseudo square wave
     * custom wave (user defined)
*/
class WaveTable : public WaveTableConstants {
public:
    float *table; // table for wavetable synthesis
    int *harmonic_amplitudes; // for custom synth
    WaveTable();
    ~WaveTable();
    // wave table methods
    void sine_wave();
    void square_wave();
    void custom_wave();
};

#endif /* wavetable_h */
