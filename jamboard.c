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
#define HIGHEST_HARMONIC (15)

#ifndef M_PI
#define M_PI (3.14159265)
#endif

/* Define base pitches so that the pitch can be calculated for any note in any octave
   using the relatively simple switch board in each engine. */
static float baseHZ[] = {27.5, 29.14, 30.87, 32.7, 34.65, 36.71, 38.89, 41.2, 43.65, 46.25, 49, 51.91};

// define the struct used to pass user data into the callback function
typedef struct
{
    float table[TABLE_SIZE]; //table for wavetable synthesis
    float table_position_L; //left stereo channel data type
    float table_position_R;
    float pitchIncrementer; // used to set increment value in the callback function
    float amplitudeScaling; // used to create envelope
}
paUserData;

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
    paUserData *data = (paUserData*)userData;
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
        // currently its effectively a mono stream...
        *out++ = (data->amplitudeScaling * (data->table[(int)data->table_position_L]));
        *out++ = (data->amplitudeScaling * (data->table[(int)data->table_position_R]));
        // Advances the left channel wavetable index by pitchImcrementer
        data->table_position_L += data->pitchIncrementer;
        // prevents loop from reading past the end of the sample in the wavetable
        if(data->table_position_L >= TABLE_SIZE) data->table_position_L -= TABLE_SIZE;
        // same for right channel:
        data->table_position_R += data->pitchIncrementer;
        if(data->table_position_R >= TABLE_SIZE) data->table_position_R -= TABLE_SIZE;
    }

    return paContinue;
}

/* These INIT functions initiate variables that will be used in a specific mode of the program */

// initiates envelope settings based on command line args; only called when in mode 'e'
int Init_EnvelopeSettings(envelopeData *envelope_settings, int argc, char *argv[])
{
    // check that envelope parameters are in correct range
    if(atoi(argv[2]) > 100 || atoi(argv[3]) > 100 || atoi(argv[4]) > 100) {
        printf("\nERROR: Envelope amplitude settings (first 3 args) need to be 0 <= X <= 100\n\n");
        return 1;
    }
    if(atoi(argv[2]) < 0 || atoi(argv[3]) < 0 || atoi(argv[4]) < 0) {
        printf("\nERROR: Envelope amplitude settings (first 3 args) need to be 0 <= X <= 100\n\n");
        return 1;
    }
    if(atoi(argv[5]) > 6 || atoi(argv[6]) > 6 || atoi(argv[7]) > 6) {
        printf("\nERROR: Envelope duration settings (last 3 args) need to be 1 <= X <= 6\n\n");
        return 1;
    }
    if(atoi(argv[5]) < 1 || atoi(argv[6]) < 1 || atoi(argv[7]) < 1) {
        printf("\nERROR: Envelope duration settings (last 3 args) need to be 1 <= X <= 6\n\n");
        return 1;
    }

    // set up envelope data from command line arguments
    envelope_settings->initial = ((float)(atoi(argv[2])) / 100);
    envelope_settings->peak = ((float)(atoi(argv[3])) / 100);
    envelope_settings->sustain = ((float)(atoi(argv[4])) / 100);
    envelope_settings->attack = 1 / (double)(atoi(argv[5]) * 500000);
    envelope_settings->decay = 1 / (double)(atoi(argv[6]) * 500000);
    envelope_settings->release = 1 / (double)(atoi(argv[7]) * 500000);

    return 0;
}

// creates a custom wavetable based on the command line args; only called when in mode 'C'
int Init_CustomTimbre(paUserData *data, int argc, char *argv[])
{
    // create a temporary wavetable; set up some variables
    float temp_table[TABLE_SIZE];
    int i;
    int x;
    int addedAmps_scaling;

    // error handling for command line arguments
    for(i = 2; i < argc; i++) {
        if(atoi(argv[i]) > 100) {
            printf("\nERROR: Harmonic amplitudes need to be 0 <= X <= 100\n\n");
            return 1;
        }
        if(atoi(argv[i]) < 0) {
            printf("\nERROR: Harmonic amplitudes need to be 0 <= X <= 100\n\n");
            return 1;
        }
        // only up to the 32nd harmonic can be set
        if(argc > 34) {
            printf("\nERROR: TOO MANY ARGS  (maximum 31 harmonics above fundamental)\n\n");
            return 1;
        }
    }

    // used to scale the overall amplitude of the synthesized waveform
    // to avoid an arithmetic overflow
    for(i = 2; i < argc; i++) {
        addedAmps_scaling += atoi(argv[i]);
    }

    // harmonics are added together, one wavetable index at a time
    for(i = 0; i < TABLE_SIZE; i++) {
        float added_amplitudes = 0;
        for(x = 2; x < argc; x++) {
            added_amplitudes += (((float)atoi(argv[x]) / 100) * data->table[((x-1) * i) % TABLE_SIZE]) / ((float)addedAmps_scaling / 100);
        }
        temp_table[i] = added_amplitudes;
    }

    // wave table is replaced with data stored in temporary table
    for(i = 0; i < TABLE_SIZE; i++) {
        data->table[i] = temp_table[i];
    }

    return 0;
}

/* This function processes the envelope of each note by modulating the amplitude
   according to the envelope arguments. */
void Envelope(paUserData *data, envelopeData *eData)
{
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
void Engine_withEnvelope(paUserData *data, envelopeData *envelope_settings)
{
    // set up some variables
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
                if(octave == 2) {
                    printf("Already at lowest octave\n");
                    break;
                } else {
                    octave--;
                    printf("Octave: %d\n", octave);
                    break;
                }
            case '=':
                if(octave == 5) {
                    printf("Already at highest octave\n");
                    break;
                } else {
                    octave++;
                    printf("Octave: %d\n", octave);
                    break;
                }
            case 'A': // CHANGE TO SINE WAVE
                // fade out signal
                for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling > 0; data->amplitudeScaling -= 0.000005) {
                    Pa_Sleep(0);
                }
                // redefine table as sine wave
                for(x=0; x<TABLE_SIZE; x++)
                    {
                        data->table[x] = (float) sin( ((double)x/(double)TABLE_SIZE) * M_PI * 2.);
                    }
                break;
            case 'S': // CHANGE TO PSEUDO SQUARE WAVE
                // fade out signal
                for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling > 0; data->amplitudeScaling -= 0.000005) {
                    Pa_Sleep(0);
                }
                // redefine table as square wave
                for(x=0; x<TABLE_SIZE; x++)
                    {
                        if(x < 10) {
                            data->table[x] = 0.005 * (0 + (x * 0.1));
                        }
                        if(9 < x < 190) {
                            data->table[x] = 0.005;
                        }
                        if(189 < x < 200) {
                            data->table[x] = 0.005 * (1 - ((x-190) * 0.1));
                        }
                        if(199 < x < 210) {
                            data->table[x] = 0.005 * (0 - ((x-200) * 0.1));
                        }
                        if(209 < x < 390) {
                            data->table[x] = -0.005;
                        }
                        if(389 < x < TABLE_SIZE) {
                            data->table[x] = 0.005 * (-1 + ((x-390) * 0.1));
                        }
                    }
                break;
            case 'z':
                printf("\nTo enter a command, type its letter and hit RETURN. ");
                printf("Each command is\none symbol long.  Don't include quotations or brackets.\n\n\n");
                printf("      PARTIAL SCHEMATIC OF KEYBOARD:\n\n");
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
                printf("     A   --->  Timbre = sine wave (default)\n     S   --->  Timbre = square wave\n");
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

void Engine_withoutEnvelope(paUserData *data, envelopeData *eData)
{
    // set up some variables
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
                if(octave == 2) {
                    printf("Already at lowest octave\n");
                } else {
                    octave--;
                    printf("Octave: %d\n", octave);
                }
                if(x == 0) x = -1;
                break;
            case '=':
                if(octave == 5) {
                    printf("Already at highest octave\n");
                } else {
                    octave++;
                    printf("Octave: %d\n", octave);
                }
                if(x == 0) x = -1;
                break;
            case 'A': // CHANGE TO SINE WAVE
                // fade out signal
                for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling > 0; data->amplitudeScaling -= 0.000005) {
                    Pa_Sleep(0);
                }
                // redefine table as sine wave
                for(x=0; x<TABLE_SIZE; x++)
                    {
                        data->table[x] = (float) sin( ((double)x/(double)TABLE_SIZE) * M_PI * 2.);
                    }
                // fade signal in again
                for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling < 1; data->amplitudeScaling += 0.000005) {
                    Pa_Sleep(0);
                }
                break;
            case 'S': // CHANGE TO PSEUDO SQUARE WAVE
                // fade out signal
                for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling > 0; data->amplitudeScaling -= 0.000005) {
                    Pa_Sleep(0);
                }
                // redefine table as square wave
                for(x=0; x<TABLE_SIZE; x++)
                    {
                        if(x < 10) {
                            data->table[x] = 0.005 * (0 + (x * 0.1));
                        }
                        if(9 < x < 190) {
                            data->table[x] = 0.005;
                        }
                        if(189 < x < 200) {
                            data->table[x] = 0.005 * (1 - ((x-190) * 0.1));
                        }
                        if(199 < x < 210) {
                            data->table[x] = 0.005 * (0 - ((x-200) * 0.1));
                        }
                        if(209 < x < 390) {
                            data->table[x] = -0.005;
                        }
                        if(389 < x < TABLE_SIZE) {
                            data->table[x] = 0.005 * (-1 + ((x-390) * 0.1));
                        }
                    }
                // fade signal in again
                for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling < 1; data->amplitudeScaling += 0.000005) {
                    Pa_Sleep(0);
                }
                break;
            case 'z':
                printf("\nTo enter a command, type its letter and hit RETURN. ");
                printf("Each command is\none symbol long.  Don't include quotations or brackets.\n\n\n");
                printf("      PARTIAL SCHEMATIC OF KEYBOARD:\n\n");
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
                printf("     A   --->  Timbre = sine wave (default)\n     S   --->  Timbre = square wave\n");
                printf("     z   --->  Print operation info to terminal\n     x   --->  EXIT PROGRAM\n\n");
                if(x == 0) x = -1;
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
                if(x == 0) x = -1;
        }
        // fade-in on first note occurs here
        if(x == 0) {
            for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling < 1; data->amplitudeScaling += 0.000005) {
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
    paUserData *data = malloc(sizeof(paUserData)); // create user data structure
    int x;

    // sine wave table created
    for(x=0; x<TABLE_SIZE; x++)
        {
            data->table[x] = (float) sin( ((double)x/(double)TABLE_SIZE) * M_PI * 2.);
        }

    /* DETERMINE MODE, SKIP TO APPROPRIATE JUMP POINT */
    if(*argv[1] == 'e') {
        if(argc != 8) {
            printf("\nERROR:  INCORRECT NUMBER OF ARGUMENTS FOR MODE = e\n\n");
            printf(" ./jamboard e (initial) (peak) (sustain) (attack) (decay) (release)\n\n\n");
            goto ARGerror;
        }
        goto EnvelopeInit;
    } else if(*argv[1] == 'c') {
        if(argc != 2) {
            printf("\nERROR:  INCORRECT NUMBER OF ARGUMENTS FOR MODE = c\n\n");
            printf(" ./jamboard c\n\n\n");
            goto ARGerror;
        }
        goto postModeInit;
    } else if(*argv[1] == 'C') {
        if(argc <= 2) {
            printf("\nERROR:  INCORRECT NUMBER OF ARGUMENTS FOR MODE = C\n\n");
            printf(" ./jamboard C (amplitude of harmonic 1) (amplitude of harmonic 2) ...\n\n\n");
            goto ARGerror;
        }
        goto CustomTimbreInit;
    } else {
        printf("\nERROR:  INCORRECT MODE ARGUMENT\n\n");
        printf("OPTIONS:\n\n");
        printf(" ./jamboard c\t---> continuous sine wave mode <---\n\n");
        printf(" ./jamboard e\t---> envelope control mode <---\n\n");
        printf(" ./jamboard C\t---> custom timbre mode <---\n\n\n");
        goto ARGerror;
    }

/* SET ARGUMENTS FOR ENVELOPE CONTROL IF THEY WERE INCLUDED */
EnvelopeInit:

    printf(""); // For some reason I need a function here or the compiler yells at me
    envelopeData *envelope_settings = malloc(sizeof(envelopeData)); // create envelope data structure
    if(Init_EnvelopeSettings(envelope_settings, argc, argv) == 1) return 0;
    goto postModeInit;

CustomTimbreInit:
    
    printf(""); // For some reason I need a function here or the compiler yells at me
    if(Init_CustomTimbre(data, argc, argv) == 1) return 0;
    goto postModeInit;

/* SKIP TO HERE AFTER MODE INITIATION FUNCTION IS CALLED (if any...) */
postModeInit:

    // Amplitude and pitch are set to avoid a segmentation fault when stream starts:
    data->amplitudeScaling = 0; // start note has no amplitude
    data->pitchIncrementer = 2.0275; // starting pitch is A-220

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
                data);
    if(err != paNoError) goto error;

    // Start stream
    err = Pa_StartStream(stream);
    if(err != paNoError) goto error;

    // print indication that program is initiated
    printf("\n\nSTREAM OPEN\n");
    // this if statement checks the mode, then starts the correct engine
    if(*argv[1] == 'e') {
        Engine_withEnvelope(data, envelope_settings);
    } else if(*argv[1] == 'c' || *argv[1] == 'C') {
        Engine_withoutEnvelope(data, envelope_settings);
    }
    
    // stream is stopped
    err = Pa_StopStream(stream);
    if(err != paNoError) goto error;

    // stream is closed
    err = Pa_CloseStream(stream);
    if(err != paNoError) goto error;
    printf("\nSTREAM CLOSED\n\n");

    // PortAudio library is terminated
    err = Pa_Terminate();
    if(err != paNoError) goto error;

    // free heap memory
    if(data) free(data);
    if(*argv[1] == 'e') free(envelope_settings);

    // program exits
    return err;
// PA error handling
error:
    Pa_Terminate();
    if(data) free(data);
    if(*argv[1] == 'e') free(envelope_settings);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return err;
// command line arg error handling
ARGerror:
    if(data) free(data);
    return 0;
}