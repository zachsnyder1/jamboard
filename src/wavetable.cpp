//
//  wavetable.cpp
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "wavetable.h"

/*
 WaveTable constructor: make table, populate with pseudo square wave
*/
WaveTable::WaveTable() {
    this->table = new float[WaveTable::TABLE_SIZE];
    this->harmonic_amplitudes = new int[WaveTable::HIGHEST_HARMONIC]; // for custom synth
    for(int i = 0; i < WaveTable::HIGHEST_HARMONIC; i++) {
        this->harmonic_amplitudes[i] = 0;
    }
    square_wave();
}

/*
 WaveTable destructor
*/
WaveTable::~WaveTable() {
    delete [] this->table;
    delete [] this->harmonic_amplitudes;
}

/*
 Populate wave table with a sine waveform
*/
void WaveTable::sine_wave() {
    int x;
    float calculated_sample;
    double sin_input;
    
    for(x = 0; x < WaveTable::TABLE_SIZE; x++) {
        sin_input = ((double)x/(double)WaveTable::TABLE_SIZE) * M_PI * 2.0;
        calculated_sample = (float) sin(sin_input);
        this->table[x] = WaveTable::SINE_MAX_AMP * calculated_sample;
    }
}

/*
 Populate wave table with a pseudo square waveform
 */
void WaveTable::square_wave() {
    int x;
    for(x = 0; x < WaveTable::TABLE_SIZE; x++) {
        if(x < 10) {
            this->table[x] = (WaveTable::SQUARE_MAX_AMP * (0 + (x * 0.1)));
        }
        if(9 < x && x < 190) {
            this->table[x] = WaveTable::SQUARE_MAX_AMP;
        }
        if(189 < x && x < 200) {
            this->table[x] = (WaveTable::SQUARE_MAX_AMP * (1 - ((x-190) * 0.1)));
        }
        if(199 < x && x < 210) {
            this->table[x] = (WaveTable::SQUARE_MAX_AMP * (0 - ((x-200) * 0.1)));
        }
        if(209 < x && x < 390) {
            this->table[x] = -WaveTable::SQUARE_MAX_AMP;
        }
        if(389 < x && x < WaveTable::TABLE_SIZE) {
            this->table[x] = (WaveTable::SQUARE_MAX_AMP * (-1 + ((x-390) * 0.1)));
        }
    }
}

/*
 Populate wave table with a custom waveform
 */
void WaveTable::custom_wave() {
    float amplitude_scale = 0.0;
    float amplitude_total = 0.0;
    float temp_table[WaveTable::TABLE_SIZE];
    int target_index;
    int i, x;
    
    // set table to reference sine wave
    this->sine_wave();
    // add harmonic amplitudes
    for(i = 0; i < WaveTable::HIGHEST_HARMONIC; i++) {
        amplitude_scale += this->harmonic_amplitudes[i];
    }
    // harmonics waveforms are added together, one wavetable index at a time
    for(i = 0; i < WaveTable::TABLE_SIZE; i++) {
        for(x = 0; x < (WaveTable::HIGHEST_HARMONIC); x++) {
            target_index = (int)(i * pow((double)2, x));
            target_index = target_index % WaveTable::TABLE_SIZE;
            amplitude_total += this->table[target_index] *
              (this->harmonic_amplitudes[x] / amplitude_scale) *
              WaveTable::CUSTOM_MAX_AMP;
        }
        temp_table[i] = amplitude_total;
    }
    // main wavetable is replaced with data stored in the temporary table
    for(i = 0; i < WaveTable::TABLE_SIZE; i++) {
        this->table[i] = temp_table[i];
    }
}
