//
//  daw.cpp
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "daw.h"

/*
 Mapping struct for Instruments
*/
struct Mapping {
    Controller *controller;
    Instrument *instrument;
};

/*
 Daw constructor
*/
Daw::Daw() {
    // objects
    this->mixer = new Mixer;
    this->outputParameters = new PaStreamParameters;
    this->err = Pa_Initialize();
    if(this->err != paNoError) this->error();
    // setup output parameters for Pa_OpenStream()
    this->outputParameters->device = Pa_GetDefaultOutputDevice();
    this->outputParameters->channelCount = 2; // stereo
    this->outputParameters->sampleFormat = paFloat32; // 32 bit floating point samples
    this->outputParameters->suggestedLatency = Pa_GetDeviceInfo(
                                               this->outputParameters->device)->
      defaultLowOutputLatency;
    this->outputParameters->hostApiSpecificStreamInfo = NULL;
    // open stream
    this->err = Pa_OpenStream(&(this->stream),
                              NULL,
                              this->outputParameters,
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
 Daw destructor
*/
Daw::~Daw() {
  delete this->mixer;
  delete this->outputParameters;
  for(int i = 0; i < this->mappings.size(); i++) {
      delete this->mappings[i];
  }
}

/*
 Register an instrument with the daw
*/
void Daw::add_instrument(Instrument *instrument) {
    this->instruments.push_back(instrument);
}

/*
 Register a controller with the daw
*/
void Daw::add_controller(Controller *controller) {
    this->controllers.push_back(controller);
}

/*
 Map a controller to an instrument
*/
void Daw::map_controller(Controller *c, Instrument *i) {
    Mapping *m = new Mapping();
    m->controller = c;
    m->instrument = i;
    this->mappings.push_back(m);
}

/*
 Clean exit
*/
void Daw::error() {
    Pa_Terminate();
    for(int i = 0; i < this->controllers.size(); i++) {
        this->controllers[i]->error(Pa_GetErrorText(this->err));
    }
    exit(0);
}

/*
 Clean your room
*/
void Daw::end() {
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
    for(int i = 0; i < this->controllers.size(); i++) {
        this->controllers[i]->farewell();
    }
}

/*
 Run daw loop: get user commands, execute commands.
*/
void Daw::run() {
    int i;
    Mapping *m;
    bool loop = true;
    
    this->mixer->fade_in();
    for(i = 0; i < this->controllers.size(); i++) {
        this->controllers[i]->salutation();
    }
    // get input from all controllers, process
    while(loop) {
        for(i = 0; i < this->mappings.size(); i++) {
            // call controller callback
            m = this->mappings[i];
            m->controller->input_loop(&loop, this, m->instrument);
        }
    }
    this->mixer->fade_out();
    this->end();
    return;
}

/*
 Callback used by PortAudio
 */
int Daw::callback(const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData)
{
    Daw *e = (Daw*)userData;
    float *out = (float*)outputBuffer;
    int i, x; //for looping use...
    // casting the unused arguments as void to avoid 'unused' errors
    (void) timeInfo;
    (void) statusFlags;
    (void) inputBuffer;
    
    // write each frame to the output buffer
    for(i = 0; i<framesPerBuffer; i++)
    {
        // add each channel
        for(x = 0; x < NUM_CHANNELS; x++) {
            *out++ = e->mixer->mix(x, e->instruments);
        }
        // Advance system
        e->mixer->advance();
        for(x = 0; x < e->instruments.size(); x++) {
            e->instruments[x]->advance();
        }
    }
    return paContinue;
}
