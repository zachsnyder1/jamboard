//
//  instrument.cpp
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "instrument.h"

/*
 Calculates a note frequency
   TAKES:
     
*/
float Instrument::calculate_note(const int note_const) {
    double base_hz = BASE_HZ[note_const % 12];
    double octave = (double)(note_const / (int)12);
    return (float)((base_hz * pow((double)2, octave)) / 110);
}

/*
 WaveTableSynth default constructor
*/
WaveTableSynth::WaveTableSynth(int num_v) {
    this->curr_voice = 0;
    this->envelope = new Envelope((num_v == 1));
    for(int i = 0; i < num_v; i++) {
        Voice *v = new Voice();
        this->voices.push_back(v);
    }
}

/*
 WaveTableSynth destructor
*/
WaveTableSynth::~WaveTableSynth() {
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
int WaveTableSynth::trigger_note(const int note_const) {
    float note;
    if(this->voices[this->curr_voice]->is_triggered()) {
        return 1; // No free voices
    } else {
        note = this->calculate_note(note_const);
        this->voices[this->curr_voice]->trigger(note, this->voices.size());
        this->curr_voice++;
        this->curr_voice %= this->voices.size();
        return 0;
    }
}

/*
 Advance all voices.
*/
void WaveTableSynth::advance() {
    int i;
    
    // advance voices
    for(i = 0; i < this->voices.size(); i++) {
        this->voices[i]->advance(this->envelope->length);
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
    int i;
    float voice_signal;
    float envelope_signal;
    
    for(i = 0; i < this->voices.size(); i++) {
        voice_signal = this->table.table[(int)(this->voices[i]->wavetable_pos[chann])];
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
            this->table.harmonic_amplitudes = (int*)data;
            this->table.custom_wave();
            break;
    }
}
