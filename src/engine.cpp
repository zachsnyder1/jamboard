//
//  engine.cpp
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "engine.h"

/*
 Engine constructor
*/
template<class EnvelopeType>
Engine<EnvelopeType>::Engine(int num_v, float start_note) {
    // objects
    this->table = WaveTable();
    this->mixer = new Mixer<EnvelopeType>(num_v);
    this->ui = UserInterface();
    //this->outputParameters = PaStreamParameters();
    this->err = Pa_Initialize();
    if(this->err != paNoError) this->error();
    // setup output parameters for Pa_OpenStream()
    this->outputParameters.device = Pa_GetDefaultOutputDevice();
    this->outputParameters.channelCount = 2; // stereo
    this->outputParameters.sampleFormat = paFloat32; // 32 bit floating point samples
    this->outputParameters.suggestedLatency = Pa_GetDeviceInfo(this->outputParameters.device)->
      defaultLowOutputLatency;
    this->outputParameters.hostApiSpecificStreamInfo = NULL;
    // open stream
    this->err = Pa_OpenStream(&(this->stream),
                              NULL,
                              &(this->outputParameters),
                              SAMPLE_RATE,
                              FRAMES_PER_BUFFER,
                              paClipOff,
                              this->callback,
                              this);
    if(this->err != paNoError) this->error();
    // start stream
    this->err = Pa_StartStream(this->stream);
    if(this->err != paNoError) this->error();
}

/*
 Calculates a note frequency
   TAKES:
     
*/
template<class EnvelopeType>
float Engine<EnvelopeType>::calculate_note(int base_index, int octave) {
    return (float)((BASE_HZ[base_index] * pow((double)2, (double)(octave))) / 110);
}

/*
 Clean exit
*/
template<class EnvelopeType>
void Engine<EnvelopeType>::error() {
    Pa_Terminate();
    this->ui.error(Pa_GetErrorText(this->err));
    exit(0);
}

/*
 Clean your room
*/
template<class EnvelopeType>
void Engine<EnvelopeType>::end() {
    // stream is stopped
    this->err = Pa_StopStream(this->stream);
    if(this->err != paNoError) this->error();
    // stream is closed
    this->err = Pa_CloseStream(this->stream);
    if(this->err != paNoError) this->error();
    // PortAudio library is terminated
    this->err = Pa_Terminate();
    if(this->err != paNoError) this->error();
    // farewell
    this->ui.farewell();
}

/*
 Run engine loop: get user commands, execute commands.
*/
template<class EnvelopeType>
void Engine<EnvelopeType>::run() {
    // salutation
    this->ui.salutation();
    // set up some local variables
    char command;
    bool loop = true;
    int octave = 3;
    
    // print engine info
    this->ui.info("ENGINE INITIATED: monophonic (continuous signal)");
    this->ui.info("PROCEED TO JAM\n");
    this->mixer->fade_in();
    // this loop continues to ask for user input, and executes user commands
    while(loop) {
        command = this->ui.get_input();
        switch(command) {
                /* NOTE COMMANDS: Pitch is calculated based on the octave and
                the frequencies found in the BASE_HZ array.
                Then, incomming_note is changed to 1,
                causing the polyphony operator child
                process to execute the note with
                envelope.
                */
            case '`':
                this->mixer->trigger_note(this->calculate_note(11, octave - 1));
                break;
            case '1':
                this->mixer->trigger_note(this->calculate_note(0, octave));
                break;
            case 'q':
                this->mixer->trigger_note(this->calculate_note(1, octave));
                break;
            case '2':
                this->mixer->trigger_note(this->calculate_note(2, octave));
                break;
            case '3':
                this->mixer->trigger_note(this->calculate_note(3, octave));
                break;
            case 'e':
                this->mixer->trigger_note(this->calculate_note(4, octave));
                break;
            case '4':
                this->mixer->trigger_note(this->calculate_note(5, octave));
                break;
            case 'r':
                this->mixer->trigger_note(this->calculate_note(6, octave));
                break;
            case '5':
                this->mixer->trigger_note(this->calculate_note(7, octave));
                break;
            case '6':
                this->mixer->trigger_note(this->calculate_note(8, octave));
                break;
            case 'y':
                this->mixer->trigger_note(this->calculate_note(9, octave));
                break;
            case '7':
                this->mixer->trigger_note(this->calculate_note(10, octave));
                break;
            case 'u':
                this->mixer->trigger_note(this->calculate_note(11, octave));
                break;
            case '8':
                this->mixer->trigger_note(this->calculate_note(0, octave + 1));
                break;
            /* OCTAVE CHANGE COMMANDS: Octave is used to calculate the
               pitch incrementer for a note in a
               given octave (see above).  Octave
               value ranges from 2 to 5. */
            case '-':
                if(octave == 2) {
                    this->ui.info("Already at lowest octave");
                } else {
                    octave--;
                    this->ui.info("Octave: %d",  octave);
                }
                break;
            case '=':
                if(octave == 5) {
                    this->ui.info("Already at highest octave");
                } else {
                    octave++;
                    this->ui.info("Octave: %d",  octave);
                }
                break;
            // TIMBRE COMMANDS: Wavetable is rewritten to create a new timbre
            case 'A': // SINE WAVE
                this->mixer->fade_out();
                this->table.sine_wave();
                this->mixer->fade_in();
                break;
            case 'S': // SQUARE WAVE
                this->mixer->fade_out();
                this->table.square_wave();
                this->mixer->fade_in();
                break;
            /* TODO: fix custom synth
            case 'C': // CREATE CUSTOM TIMBRE
                this->mixer->fade_out();
                this->ui.custom_wave(this->table.harmonic_amplitudes);
                this->table.custom_wave();
                this->mixer->fade_in();
                break;
            */
            case 'z': // PRINT OPERATING INFO TO TERMINAL
                this->ui.help();
                break;
            case 'x':
                // stops the while loop
                loop = false;
                this->mixer->fade_out();
                break;
            default:
                this->ui.info("NOT A NOTE!\n");
        }
    }
    this->end();
    return;
}

/*
 Callback used by PortAudio
 */
template<class EnvelopeType>
int Engine<EnvelopeType>::callback(const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData)
{
    Engine *engine = (Engine*)userData;
    float *out = (float*)outputBuffer;
    int i; //for looping use...
    // casting the unused arguments as void to avoid 'unused' errors
    (void) timeInfo;
    (void) statusFlags;
    (void) inputBuffer;
    
    // write each frame to the output buffer
    for(i=0; i<framesPerBuffer; i++)
    {
        // frames alternate between left and right channels
        *out++ = engine->mixer->mix(0, &engine->table);
        *out++ = engine->mixer->mix(1, &engine->table);
        // Advance system
        engine->mixer->advance();
    }
    return paContinue;
}

template class Engine<InfiniteEnvelope>;
template class Engine<FiniteEnvelope>;
