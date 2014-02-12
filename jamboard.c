/* NOTE:   Below I refer to 'wavetable samples', by which I mean the one-period-long
           waveforms that are represented by the wavetables.  This is different
           from the individual amplitude values that the digital signal is
           represented by.  */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "portaudio.h"

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (192)
#define TABLE_SIZE (400)

#ifndef M_PI
#define M_PI (3.14159265)
#endif

/* Define base pitches so that the pitch can be calculated for any note in any octave
   using the relatively simple switch board in each engine. */
static float baseHZ[] = {27.5, 29.14, 30.87, 32.7, 34.65, 36.71, 38.89, 41.2, 43.65, 46.25, 49, 51.91};

// define the struct used to pass user data into the callback function
typedef struct
{
    float sine[TABLE_SIZE]; //table for wavetable synthesis
    float left_phase; //left stereo channel data type
    float pitchIncrementer; // used to set increment value in the callback function
    float amplitudeScaling; // used to create envelope
}
paTestData;

// define the struct used to hold envelope info
typedef struct
{
    // for scaling magnitude of the envelope transitions
    float initial;
    float peak;
    float sustain;
    // for temporally scaling the envelope transitions
    double attack;
    double decay;
    double release;
}
envelopeData;

/* This is the callback function which is called repeatedly by portaudio,
   it is what actually drives the audio stream.  Every time it is called
   it sends another set of frames to the output buffer.  When the buffer
   filled it is called again, until something stops the stream. */
static int jamboardCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{
    // setting up arguments for use
    paTestData *data = (paTestData*)userData;
    float *out = (float*)outputBuffer;
    unsigned long i; //for looping use...

    // casting the unused arguments as void to avoid 'unused' errors 
    (void) timeInfo;
    (void) statusFlags;
    (void) inputBuffer;

    // the loop that writes each frame to the output buffer
    for(i=0; i<framesPerBuffer; i++)
    {
        // frames alternate between left and right channels, thereby
        // multiplexing stereo information into the serial buffer
        *out++ = (data->amplitudeScaling * (data->sine[(int)data->left_phase]));
        *out++ = (data->amplitudeScaling * (data->sine[(int)data->left_phase]));
        // Advances the wavetable index for left channel by pitchImcrementer
        data->left_phase += data->pitchIncrementer;
        // prevents loop from reading past the end of the sample in the wavetable
        if(data->left_phase >= TABLE_SIZE) data->left_phase -= TABLE_SIZE;
        // Same proceedures for right channel
        data->left_phase += data->pitchIncrementer;
        if(data->left_phase >= TABLE_SIZE) data->left_phase -= TABLE_SIZE;
    }

    return paContinue;
}

int Init_EnvelopeSettings(envelopeData *envData, int argc, char *argv[])
{
    // set up pointers
    envelopeData *envelope_settings = envData;

    // check that envelope parameters are in correct range (0<arg<100)
    if(atoi(argv[1]) > 100 || atoi(argv[2]) > 100 || atoi(argv[3]) > 100) {
        printf("\nERROR: Envelope settings need to be 0 <= X <= 100\n\n");
        return 1;
    }
    if(atoi(argv[1]) < 0 || atoi(argv[2]) < 0 || atoi(argv[3]) < 0) {
        printf("\nERROR: Envelope settings need to be 0 <= X <= 100\n\n");
        return 1;
    }
    if(atoi(argv[4]) > 100 || atoi(argv[5]) > 100 || atoi(argv[6]) > 100) {
        printf("\nERROR: Envelope settings need to be 0 <= X <= 100\n\n");
        return 1;
    }
    if(atoi(argv[4]) < 0 || atoi(argv[5]) < 0 || atoi(argv[6]) < 0) {
        printf("\nERROR: Envelope settings need to be 0 <= X <= 100\n\n");
        return 1;
    }

    // set up envelope data from command line arguments
    envelope_settings->initial = ((float)(atoi(argv[1])) / 100);
    envelope_settings->peak = ((float)(atoi(argv[2])) / 100);
    envelope_settings->sustain = ((float)(atoi(argv[3])) / 100);
    envelope_settings->attack = 1 / (double)(atoi(argv[4]) * 500000);
    envelope_settings->decay = 1 / (double)(atoi(argv[5]) * 500000);
    envelope_settings->release = 1 / (double)(atoi(argv[6]) * 500000);

    return 0;
}

/* This function processes the envelope of each note by modulating the amplitude
   according to the envelope arguments. */
void Envelope(paTestData *userData, envelopeData *envData)
{
    // set up struct pointers
    paTestData *data = (paTestData*)userData;
    envelopeData *eData = (envelopeData*)envData;
    //set up amplitude increment value for looping
    double attackLocal = eData->attack;
    double decayLocal = eData->decay;
    double releaseLocal = eData->release;
    
    // execute attack
    for(data->amplitudeScaling = eData->initial; data->amplitudeScaling < eData->peak; data->amplitudeScaling += attackLocal) {
        Pa_Sleep(0);
    }
    // execute decay
    for(data->amplitudeScaling = eData->peak; data->amplitudeScaling > eData->sustain; data->amplitudeScaling -= decayLocal) {
        Pa_Sleep(0);
    }
    // execute release
    for(data->amplitudeScaling = eData->sustain; data->amplitudeScaling > 0; data->amplitudeScaling -= releaseLocal) {
        Pa_Sleep(0);
    }

    // make sure scaling factor is exactly 0
    data->amplitudeScaling = 0;

    return;
}

/* The engines prompt for and process user commands. */

// This engine is used if envelope arguments were included on the command line
void Engine_withEnvelope(paTestData *userData, envelopeData *envData)
{
    // set up struct pointers
    paTestData *data = userData;
    envelopeData *envelope_settings = envData;

    char ch;
    int loop = 1;
    int octave = 3;
    int x = 0;

    // print engine info
    printf("ENGINE INITIATED: with envelope\nJAM AWAY\n\n");
    // this loop continues to ask for user input, and executes user commands
    while(loop) {
        // print the prompt
        printf(">> ");
        ch = getc(stdin);
        getc(stdin); // eats <RETURN>
        switch(ch) {
            case '`':
                data->pitchIncrementer = (baseHZ[11] * pow((double)2, (double)(octave-1))) / 110;
                Envelope(data, envelope_settings);
                break;
            case '1':
                data->pitchIncrementer = (baseHZ[0] * pow((double)2, (double)octave)) / 110;
                 Envelope(data, envelope_settings);
                break;
            case 'q':
                data->pitchIncrementer = (baseHZ[1] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case '2':
                data->pitchIncrementer = (baseHZ[2] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case '3':
                data->pitchIncrementer = (baseHZ[3] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case 'e':
                data->pitchIncrementer = (baseHZ[4] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case '4':
                data->pitchIncrementer = (baseHZ[5] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case 'r':
                data->pitchIncrementer = (baseHZ[6] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case '5':
                data->pitchIncrementer = (baseHZ[7] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case '6':
                data->pitchIncrementer = (baseHZ[8] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case 'y':
                data->pitchIncrementer = (baseHZ[9] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case '7':
                data->pitchIncrementer = (baseHZ[10] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case 'u':
                data->pitchIncrementer = (baseHZ[11] * pow((double)2, (double)octave)) / 110;
                Envelope(data, envelope_settings);
                break;
            case '8':
                data->pitchIncrementer = (baseHZ[0] * pow((double)2, (double)(octave+1))) / 110;
                Envelope(data, envelope_settings);
                break;
            case '-':
                if(octave == 1) {
                    printf("Already at lowest octave");
                    break;
                } else {
                    octave--;
                    printf("Octave: %d\n", octave);
                    break;
                }
            case '=':
                if(octave == 5) {
                    printf("Already at highest octave");
                    break;
                } else {
                    octave++;
                    printf("Octave: %d\n", octave);
                    break;
                }
            case 'z':
                printf("To enter a command type its letter and hit RETURN. ");
                printf("Each command is one symbol long.  Don't include quotations.\n\n\n");
                printf("         SCHEMATIC OF KEYBOARD:\n\n");
                printf("---------------- KEYS: -----------------------|\n");
                printf("                                              |\n");
                printf("[`][1]   [2] [3]   [4]   [5] [6]   [7]   [8]  |\n");
                printf("      [q]       [e]   [r]       [y]   [u]     |\n");
                printf("                                              |\n");
                printf("----------- CORRESPONDING NOTES: -------------|\n");
                printf("                                              |\n");
                printf("[g#][a]   [b] [c]   [d]   [e] [f]   [g]   [a] |\n");
                printf("       [a#]      [c#]  [d#]      [f#]  [g#]   |\n");
                printf("______________________________________________|\n\n\n");
                printf("   OTHER COMMANDS:\n   --------------\n     -   --->  Go down an octave\n");
                printf("     =   --->  Go up an octave (if it were '+' you would have to type shift...)\n");
                printf("     z   --->  Print operation info to terminal\n     x   --->  EXIT PROGRAM\n\n");
                break;
            case 'x':
                loop=0;
                break;

            default:
                printf("NOT A NOTE!\n");
        }
    }

    return;
}

void Engine_withoutEnvelope(paTestData *userData, envelopeData *envData)
{
    // set up struct pointers
    paTestData *data = userData;
    envelopeData *eData = envData;

    char ch;
    int loop = 1;
    int octave = 3;
    int x = 0;

    // print engine info
    printf("ENGINE INITIATED: without envelope\nJAM AWAY\n\n");
    // this loop continues to ask for user input, and executes user commands
    while(loop) {
        // print the prompt
        printf(">> ");
        ch = getc(stdin);
        getc(stdin); // eats <RETURN>
        // used to fade in signal on first note, in order to prevent damage to speakers
        if(x == 0) {
            data->amplitudeScaling = 0;
        }
        switch(ch) {
            case '`':
                data->pitchIncrementer = (baseHZ[11] * pow((double)2, (double)(octave-1))) / 110;
                break;
            case '1':
                data->pitchIncrementer = (baseHZ[0] * pow((double)2, (double)octave)) / 110;
                break;
            case 'q':
                data->pitchIncrementer = (baseHZ[1] * pow((double)2, (double)octave)) / 110;
                break;
            case '2':
                data->pitchIncrementer = (baseHZ[2] * pow((double)2, (double)octave)) / 110;
                break;
            case '3':
                data->pitchIncrementer = (baseHZ[3] * pow((double)2, (double)octave)) / 110;
                break;
            case 'e':
                data->pitchIncrementer = (baseHZ[4] * pow((double)2, (double)octave)) / 110;
                break;
            case '4':
                data->pitchIncrementer = (baseHZ[5] * pow((double)2, (double)octave)) / 110;
                break;
            case 'r':
                data->pitchIncrementer = (baseHZ[6] * pow((double)2, (double)octave)) / 110;
                break;
            case '5':
                data->pitchIncrementer = (baseHZ[7] * pow((double)2, (double)octave)) / 110;
                break;
            case '6':
                data->pitchIncrementer = (baseHZ[8] * pow((double)2, (double)octave)) / 110;
                break;
            case 'y':
                data->pitchIncrementer = (baseHZ[9] * pow((double)2, (double)octave)) / 110;
                break;
            case '7':
                data->pitchIncrementer = (baseHZ[10] * pow((double)2, (double)octave)) / 110;
                break;
            case 'u':
                data->pitchIncrementer = (baseHZ[11] * pow((double)2, (double)octave)) / 110;
                break;
            case '8':
                data->pitchIncrementer = (baseHZ[0] * pow((double)2, (double)(octave+1))) / 110;
                break;
            case '-':
                if(octave == 1) {
                    printf("Already at lowest octave");
                    break;
                } else {
                    octave--;
                    printf("Octave: %d\n", octave);
                    break;
                }
            case '=':
                if(octave == 5) {
                    printf("Already at highest octave");
                    break;
                } else {
                    octave++;
                    printf("Octave: %d\n", octave);
                    break;
                }
            case 'z':
                printf("To enter a command type its letter and hit RETURN. ");
                printf("Each command is one symbol long.  Don't include quotations.\n\n\n");
                printf("         SCHEMATIC OF KEYBOARD:\n\n");
                printf("---------------- KEYS: -----------------------|\n");
                printf("                                              |\n");
                printf("[`][1]   [2] [3]   [4]   [5] [6]   [7]   [8]  |\n");
                printf("      [q]       [e]   [r]       [y]   [u]     |\n");
                printf("                                              |\n");
                printf("----------- CORRESPONDING NOTES: -------------|\n");
                printf("                                              |\n");
                printf("[g#][a]   [b] [c]   [d]   [e] [f]   [g]   [a] |\n");
                printf("       [a#]      [c#]  [d#]      [f#]  [g#]   |\n");
                printf("______________________________________________|\n\n\n");
                printf("   OTHER COMMANDS:\n   --------------\n     -   --->  Go down an octave\n");
                printf("     =   --->  Go up an octave (if it were '+' you would have to type shift...)\n");
                printf("     z   --->  Print operation info to terminal\n     x   --->  EXIT PROGRAM\n\n");
                break;
            case 'x':
                // stops the while loop
                loop=0;
                // fades out amplitude to avoid damage to speakers
                for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling > 0; data->amplitudeScaling -= 0.000005) {
                    Pa_Sleep(0);
                }
                x++;
                break;

            default:
                printf("NOT A NOTE!\n");
        }
        // fade-in on first note occurs here
        if(x == 0) {
            for(data->amplitudeScaling = 0; data->amplitudeScaling < 1; data->amplitudeScaling += 0.000005) {
                    Pa_Sleep(0);
                }
        }
        // increment x so that fade-in only occurs on first note
        x++;
    }

    return;
}

int main(int argc, char *argv[])
{
    // set up some variables
    PaStreamParameters outputParameters; //struct for stream parameters
    PaStream *stream; // open stream
    PaError err; // err is used for error handling
    paTestData data; // create user data structure
    int x;

    /* DETERMINE MODE, SKIP TO APPROPRIATE ROUTINE */
    if(argc == 7) {
        goto EnvelopeInit;
    } else if(argc == 1) {
        goto skipEnvelop;
    } else {
        printf("\t\t>>>INCORRECT NUMBER OF ARGUMENTS<<<\n\n");
        printf("OPTIONS:\n\n");
        printf("./jamboard\n\t---> continuous sine wave mode <---\n\n");
        printf("./jamboard (initial) (peak) (sustain) (attack) (decay) (release)\n\t");
        printf("---> envelope control mode <---\n\n\n");
        return 0;
    }

/* SET ARGUMENTS FOR ENVELOPE CONTROL IF THEY WERE INCLUDED */
EnvelopeInit:

    printf(""); // For some reason I need a function here or the compiler yells at me
    envelopeData envelope_settings; // create envelope data structure
    if(Init_EnvelopeSettings(&envelope_settings, argc, argv) == 1) return 0;

/* SKIP TO HERE IF ENVELOPE ARGUMENTS WERE NOT INCLUDED */
skipEnvelop:

    // Amplitude and pitch are set to avoid a segmentation fault:
    data.amplitudeScaling = 0; // start note has no amplitude
    data.pitchIncrementer = 2.0275; // starting pitch is A-220

    // sine wave table created
    for(x=0; x<TABLE_SIZE; x++)
        {
            data.sine[x] = (float) sin( ((double)x/(double)TABLE_SIZE) * M_PI * 2.);
        }

    // print info about program to terminal
    printf("\n\tJAMBOARD WAVETABLE SYNTHESIS");
    printf("\ntype 'z' and hit RETURN for operating info");
    printf("\ntype 'x' and hit RETURN to exit program");

    // initializes PortAudio library
    err = Pa_Initialize();
    if(err != paNoError) goto error;

    // setup output parameters for Pa_OpenStream()
    outputParameters.device = Pa_GetDefaultOutputDevice(); // sends audio to default output
    outputParameters.channelCount = 2; // stereo
    outputParameters.sampleFormat = paFloat32; // 32 bit floating point samples
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    // Open stream
    err = Pa_OpenStream(
                &stream,
                NULL,
                &outputParameters,
                SAMPLE_RATE,
                FRAMES_PER_BUFFER,
                paClipOff,
                jamboardCallback,
                &data);
    if(err != paNoError) goto error;

    // Start stream; no sound at this point because wavetable is empty
    err = Pa_StartStream(stream);
    if(err != paNoError) goto error;

    // print indication that program is initiated
    printf("\n\nSTREAM OPEN\n");
    // if statement checks mode, starts correct engine
    if(argc == 7) {
        Engine_withEnvelope(&data, &envelope_settings);
    } else if(argc == 1) {
        Engine_withoutEnvelope(&data, &envelope_settings);
    }
    
    // stream is stopped
    err = Pa_StopStream(stream);
    if(err != paNoError) goto error;

    // stream is closed
    err = Pa_CloseStream(stream);
    if(err != paNoError) goto error;

    // PortAudio library is terminated
    err = Pa_Terminate();
    if(err != paNoError) goto error;

    // program exits
    return err;
//error handling
error:
    Pa_Terminate();
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return err;
}