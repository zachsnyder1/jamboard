//
//  instrument.cpp
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "instrument.h"

/*
 Instrument constructor
*/
Instrument::Instrument(int num_c, int num_v) {
    this->curr_voice = 0;
    this->num_channels = num_c;
    this->envelope = new Envelope();
    for(int i = 0; i < num_v; i++) {
        Voice *v = new Voice();
        this->voices.push_back(v);
    }
}

/*
 WaveTableSynth destructor
*/
Instrument::~Instrument() {
    delete this->envelope;
    for(int i = 0; i < this->voices.size(); i++) {
        delete this->voices[i];
    }
}

/*
 Trigger a note
   TAKES:
     note --> the note to trigger
*/
int Instrument::trigger(const int note_const) {
    if(this->voices[this->curr_voice]->is_triggered()) {
        return 1; // No free voices
    } else {
        this->trigger_template(note_const);
        this->voices[this->curr_voice]->trigger();
        this->curr_voice++;
        this->curr_voice %= this->voices.size();
        return 0;
    }
}

/*
 Advance all voices.
*/
void Instrument::advance() {
    int i;
    this->advance_template();
    // advance voices
    for(i = 0; i < this->voices.size(); i++) {
        this->voices[i]->advance(this->envelope->length);
    }
}

/*
 Calculates a note frequency
   TAKES:
     
*/
float WaveTableSynth::calculate_note(const int note_const) {
    double base_hz = this->BASE_HZ[note_const % 12];
    double octave = (double)(note_const / (int)12);
    return (float)((base_hz * pow((double)2, octave)) / 110);
}

/*
 WaveTableSynth default constructor
*/
WaveTableSynth::WaveTableSynth(int num_c, int num_v) : 
Instrument::Instrument(num_c, num_v) {
    int i;
    this->pitch_incrementers = new float[this->voices.size()];
    this->wavetable_positions = new float[(this->voices.size()*this->num_channels)];
    for(i = 0; i < this->voices.size(); i++) {
        this->pitch_incrementers[i] = START_NOTE;
    }
    for(int i = 0; i < (this->voices.size()*this->num_channels); i++){
        this->wavetable_positions[i] = 0.0;
    }
}

/*
 WaveTableSynth destructor
*/
WaveTableSynth::~WaveTableSynth() {
    delete [] this->pitch_incrementers;
    delete [] this->wavetable_positions;
}

/*
 WaveTableSynth override of trigger_template
*/
void WaveTableSynth::trigger_template(const int note_const) {
    this->pitch_incrementers[this->curr_voice] = this->calculate_note(note_const);
}

/*
 WaveTableSynth override of advance_template
*/
void WaveTableSynth::advance_template() {
    int v, c, x;
    // advance channel positions
    for(v = 0; v < this->voices.size(); v++) {
        for(c = 0; c < this->num_channels; c++) {
            x = (v*this->num_channels) + c;
            this->wavetable_positions[x] += this->pitch_incrementers[v];
            if(this->wavetable_positions[x] > WaveTable::TABLE_SIZE) {
                this->wavetable_positions[x] -=  WaveTable::TABLE_SIZE;
            }
        }
    }
}

/*
 Combine all voices on specified channel
   TAKES:
     chann --> int channel to mix
   RETURNS:
     float added voices
*/
float WaveTableSynth::output(int chann) {
    float out = 0.0;
    int i, wt_index;
    float voice_signal;
    float envelope_signal;
    
    for(i = 0; i < this->voices.size(); i++) {
        wt_index = (int)(this->wavetable_positions[(i*this->num_channels)+chann]);
        voice_signal = this->table.table[wt_index];
        envelope_signal = this->envelope->calculate(this->voices[i]->envelope_pos, 
                                                    this->voices[i]->is_triggered());
        out += (voice_signal * envelope_signal);
    }
    return out;
}

/*
 WaveTableSynth-specific command processing
   TAKES:
     command --> const int command code
     data    --> void * any data passed with command
*/
void WaveTableSynth::command(const int command, void *data) {
    switch(command) {
        case COMMAND_SINE_WAVE:
            this->table.sine_wave();
            break;
        case COMMAND_SQUARE_WAVE:
            this->table.square_wave();
            break;
        case COMMAND_CUSTOM_WAVE:
            int *d = (int*)data;
            for(int i=0; i < WaveTable::HIGHEST_HARMONIC; i++) {
                this->table.harmonic_amplitudes[i] = d[i];
            }
            this->table.custom_wave();
            break;
    }
}
