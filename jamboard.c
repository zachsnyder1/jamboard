/* NOTE:   Below I refer to 'wavetable samples', by which I mean the one-period-long
           waveforms that are represented by the wavetables.  This is different
           from the individual amplitude values that the digital signal is
           represented by.  */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "portaudio.h"

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (192)
#define TABLE_SIZE (400)
#define HIGHEST_HARMONIC (10)
#define VOICES (6)

#ifndef M_PI
#define M_PI (3.14159265)
#endif

/* Define base pitches so that the pitch can be calculated for any note in any
   octave using the relatively simple switch board in each engine. */
static float baseHZ[] = {27.5, 29.14, 30.87, 32.7, 34.65, 36.71, 38.89, 41.2,
    43.65, 46.25, 49, 51.91};

// define the struct used to pass user data into the callback function
typedef struct
{
    float table[TABLE_SIZE]; // table for wavetable synthesis
    float table_position_L; // left stereo channel table position
    float table_position_R; // right stereo channel table position
    float pitchIncrementer; // used to set increment value in the callback function
    float amplitudeScaling; // used to create envelope
    float harm_amps[HIGHEST_HARMONIC]; // used to hold harmonic amplitudes for synth
}
paUserData;

/* 
   ---------------------------------------------------------------
                        PORT AUDIO CALLBACK
   ---------------------------------------------------------------
*/

/* This is the callback function which is called repeatedly by portaudio,
   it is what actually drives the audio stream. */
static int monophonyCallback(const void *inputBuffer, void *outputBuffer,
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
        // Advances the left channel wavetable index by pitchIncrementer
        data->table_position_L += data->pitchIncrementer;
        // prevents loop from reading past the end of the sample in the wavetable
        if(data->table_position_L >= TABLE_SIZE) data->table_position_L -= TABLE_SIZE;
        // same for right channel:
        data->table_position_R += data->pitchIncrementer;
        if(data->table_position_R >= TABLE_SIZE) data->table_position_R -= TABLE_SIZE;
    }

    return paContinue;
}

/* 
   ---------------------------------------------------------------
                         HELPER FUNCTIONS
   ---------------------------------------------------------------
*/

void error(PaError err, paUserData *data) {
	Pa_Terminate();
	free(data);
	fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
	exit(0);
}

void printTableFloat(float *table, int size, char *msg) {
    FILE *f = fopen("table.csv", "a");
    fprintf(f, "[%d] %s", size, msg);
    for(int i = 0; i < size; i++) {
        fprintf(f, "%f,", table[i]);
    }
    fprintf(f, "\n");
    fclose(f);
}

void printTableInt(int *table, int size, char *msg) {
    FILE *f = fopen("table.csv", "a");
    fprintf(f, "[%d] %s", size, msg);
    for(int i = 0; i < size; i++) {
        fprintf(f, "%d,", table[i]);
    }
    fprintf(f, "\n");
    fclose(f);
}

void fadeSignalOut(paUserData *data) {
    for(; data->amplitudeScaling > 0; data->amplitudeScaling -= 0.000005) {
        Pa_Sleep(0);}
}

void fadeSignalIn(paUserData *data) {
    for(; data->amplitudeScaling < 1; data->amplitudeScaling += 0.000005) {
        Pa_Sleep(0);
    }
}

/* 
   ---------------------------------------------------------------
                   WAVE TABLE SYNTHESIS FUNCTIONS
   ---------------------------------------------------------------
*/

void makeSine(paUserData *data) {
    for(int x=0; x<TABLE_SIZE; x++) {
        data->table[x] = (float) sin( ((double)x/(double)TABLE_SIZE) * M_PI * 2.);
    }
}

void makeSquare(paUserData *data) {
    for(int x=0; x < TABLE_SIZE; x++) {
        if(x < 10) {
            data->table[x] = 0.005 * (0 + (x * 0.1));
        }
        if(9 < x && x < 190) {
            data->table[x] = 0.005;
        }
        if(189 < x && x < 200) {
            data->table[x] = 0.005 * (1 - ((x-190) * 0.1));
        }
        if(199 < x && x < 210) {
            data->table[x] = 0.005 * (0 - ((x-200) * 0.1));
        }
        if(209 < x && x < 390) {
            data->table[x] = -0.005;
        }
        if(389 < x && x < TABLE_SIZE) {
            data->table[x] = 0.005 * (-1 + ((x-390) * 0.1));
        }
    }
}

/* 
   ---------------------------------------------------------------
                         CUSTOM SYTHESIS (broken)
   ---------------------------------------------------------------
*/

void makeCustom(paUserData *data) {
    float amplitude_scale;
    float amplitude_total;
    float temp_table[TABLE_SIZE];
    int target_index;
    int i, x;
    
    amplitude_scale = 0.0;
    for(i = 0; i < HIGHEST_HARMONIC; i++) {
        printf("%f\n", data->harm_amps[i]);
        amplitude_scale += data->harm_amps[i];
    }

    // harmonics are added together, one wavetable index at a time
    for(i = 0; i < TABLE_SIZE; i++) {
        amplitude_total = 0.0;
        for(x = 0; x < (HIGHEST_HARMONIC); x++) {
        	target_index = (int)(i * pow((double)2, x));
        	target_index = target_index % TABLE_SIZE;
            amplitude_total += data->table[target_index] * 
                                (data->harm_amps[x] / amplitude_scale);
            //printf("%f\n", amplitude_total);
        }
        temp_table[i] = amplitude_total;
    }
    printTableFloat(temp_table, TABLE_SIZE, "TEMP TABLE: ");

    // main wavetable is replaced with data stored in the temporary table
    for(i = 0; i < TABLE_SIZE; i++) {
        data->table[i] = temp_table[i];
    }
    printTableFloat(data->table, TABLE_SIZE, "WAVE TABLE: ");
}

// create a custom timbre based on user input
void makeCustomUI(paUserData *data)
{
    // setup local variables
    char command_check;
    int loop = 1;
    int x = 0;

    // print info to terminal
    printf("\n\t\tSYNTHESIZE A CUSTOM TIMBRE\n\n");
    printf("type 'z' and hit RETURN for custom synthesis info\n");
    printf("type 'x' and hit RETURN to abort custom timbre and return to jam\n");
    printf("type 's' and hit RETURN to synthesize timbre and return to jam\n\n");
    printf("Enter amplitude of each harmonic:");

    /* PROMPT FOR, PROCESS, AND STORE USER INPUT */
    while(loop) {
        // prompt for and then store input value
        if(x == 0) {
            // prompt
            printf("\n  Fundamental: ");
            // store input value
            fscanf(stdin, "%s", &command_check);
        } else {
            // prompt
            printf("  Hamonic %d: ", x);
            // store input value
            fscanf(stdin, "%s", &command_check);
        }

        // process input (commands and errors)
        if(command_check == 'z') { // COMMAND: print info
            printf("\n\nINFO:\n\n");
            printf("The 'synthesize custom timbre' function allows you to synthesize a new sound by");
            printf("\nsetting the relative amplitude for each harmonic, up to the 31st harmonic.");
            printf("\nRelative amplitudes are values between 0 and 100.  Enter a value for each");
            printf("\nharmonic as you are promted.  A value of 0 means the harmonic is not");
            printf("\nincluded in the sound.  When you have entered a value for the highest");
            printf("\nharmonic that you want to include, enter 's' (without quotations) to");
            printf("\nsynthesize the sound\n\n");
            printf("  LOCAL COMMANDS:\n");
            printf("  --------------\n");
            printf("    z   --->  Print custom synthesis info\n");
            printf("    s   --->  Synthesize waveform\n");
            printf("    x   --->  Abort custom timbre synthesis and return to jam\n");
        } else if(command_check == 'x') { // COMMAND: abort custom timbre synthesis
            printf("\n\nCUSTOM TIMBRE SYNTHESIS ABORTED\n\n");
            // eat enter
            getc(stdin);
            // returns to engine loop
            return;
        } else if(command_check == 's' && x == 0) { // ERROR: need more user info
            printf("\n\tERROR: Enter amplitude for one or more frequency\n");
        } else if(command_check == 's' || x >= HIGHEST_HARMONIC-1) { // COMMAND: synthesize
            printf("\n");
            loop = 0;
        } else if(atoi(&command_check) > 100) { // ERROR: invalid amplitude value
            printf("\n\tERROR: Amplitude needs to be 0<=X<=100\n");
        } else { // STORE USER INPUT, PROMPT FOR NEXT HARMONIC
            // set array value to input value
            printf("%f\n", (float)atoi(&command_check));
            data->harm_amps[x] = (float)atoi(&command_check);
            x++;
        }
    }
    for(int i = 0; i < HIGHEST_HARMONIC; i++) {
        printf("Index %d: %f\n", i, data->harm_amps[i]);
    }
    getc(stdin); // eat enter

    return;
}

/* 
   ---------------------------------------------------------------
                              ENGINES
   ---------------------------------------------------------------
*/

void Engine_Monophonic(paUserData *data)
{
    // set up some local variables
    char ch;
    char waveform = 'a';
    int loop = 1;
    int octave = 3;

    // print engine info
    printf("ENGINE INITIATED: monophonic (continuous signal)\nPROCEED TO JAM\n\n");
    fadeSignalIn(data);

    // this loop continues to ask for user input, and executes user commands
    while(loop) {
        // print the prompt
        printf(">> ");
        ch = getc(stdin); // gets command letter
        getc(stdin); // eats newline

        // user command switchboard
        switch(ch) {
            /* NOTE COMMANDS: Pitch is calculated based on the octave and
                              the frequencies found in the baseHZ array.
                              Then, incomming_note is changed to 1,
                              causing the polyphony operator child
                              process to execute the note with
                              envelope.
            */
            case '`':
                data->pitchIncrementer = (baseHZ[11] *
                    pow((double)2, (double)(octave-1))) / 110;
                break;
            case '1':
                data->pitchIncrementer = (baseHZ[0] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case 'q':
                data->pitchIncrementer = (baseHZ[1] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case '2':
                data->pitchIncrementer = (baseHZ[2] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case '3':
                data->pitchIncrementer = (baseHZ[3] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case 'e':
                data->pitchIncrementer = (baseHZ[4] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case '4':
                data->pitchIncrementer = (baseHZ[5] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case 'r':
                data->pitchIncrementer = (baseHZ[6] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case '5':
                data->pitchIncrementer = (baseHZ[7] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case '6':
                data->pitchIncrementer = (baseHZ[8] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case 'y':
                data->pitchIncrementer = (baseHZ[9] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case '7':
                data->pitchIncrementer = (baseHZ[10] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case 'u':
                data->pitchIncrementer = (baseHZ[11] *
                    pow((double)2, (double)octave)) / 110;
                break;
            case '8':
                data->pitchIncrementer = (baseHZ[0] *
                    pow((double)2, (double)(octave+1))) / 110;
                break;
            /* OCTAVE CHANGE COMMANDS: Octave is used to calculate the
                                       pitch incrementer for a note in a
                                       given octave (see above).  Octave
                                       value ranges from 2 to 5.
            */
            case '-':
                if(octave == 2) {
                    printf("Already at lowest octave\n");
                } else {
                    octave--;
                    printf("Octave: %d\n", octave);
                }
                break;
            case '=':
                if(octave == 5) {
                    printf("Already at highest octave\n");
                } else {
                    octave++;
                    printf("Octave: %d\n", octave);
                }
                break;
            /* TIMBRE COMMANDS: Wavetable is rewritten to create a new timbre */
            case 'A': // CHANGE TO SINE WAVE
                fadeSignalOut(data);
                makeSine(data);
                fadeSignalIn(data);
                waveform = 'a';
                break;
            case 'S': // CHANGE TO PSEUDO SQUARE WAVE
                fadeSignalOut(data);
                // redefine table as square wave
                makeSquare(data);
                fadeSignalIn(data);
                // change waveform indicator
                waveform = 's';
                break;
/*
            case 'C': // CREATE CUSTOM TIMBRE
                fadeSignalOut(data);
                // change table to sinewave for reference
                if(waveform != 'a') {
                    makeSine(data);
                }
                makeCustomUI(data);
                makeCustom(data);
                // change waveform indicator
                waveform = 'c';
                // fade signal in again
                fadeSignalIn(data);
                break;
*/
            case 'z': // PRINT OPERATING INFO TO TERMINAL
                printf("\nTo enter a command, type its letter and hit ");
                printf("RETURN. Each command is\none symbol long.  Don't");
                printf("include quotations or brackets.\n\n\n");
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
                printf("______________________________________________|\n");
                printf("\n\n   OTHER COMMANDS:\n   --------------\n");
                printf("     -   --->  Go down an octave\n");
                printf("     =   --->  Go up an octave (if it were '+' you");
                printf("would have to type shift...)\n");
                printf("     A   --->  Timbre = sine wave (default)\n");
                printf("     S   --->  Timbre = square wave\n");
                //printf("     C   --->  Synthesize custom timbre\n");
                printf("     z   --->  Print operation info to terminal\n");
                printf("     x   --->  EXIT PROGRAM\n\n");
                break;
            case 'p':
                printTableFloat(data->table, TABLE_SIZE, "WAVE TABLE: ");
                break;
            case 'x':
                // stops the while loop
                loop=0;
                fadeSignalOut(data);
                break;
            default:
                printf("NOT A NOTE!\n");
        }
    }

    return;
}

/* 
   ---------------------------------------------------------------
                              MAIN
   ---------------------------------------------------------------
*/

int main(int argc, char *argv[])
{
    // set up some variables
    PaStreamParameters outputParameters; //struct for stream parameters
    PaStream *stream; // open stream
    PaError err; // err is used for error handling
    paUserData *data; // forward declaration of mapped paUserdata instance
    int x; // for looping

    data = malloc(sizeof(paUserData));
    if(data == MAP_FAILED) {
        printf("\nUser data failed to map.\n");
        exit(0);
    }
    
    // sine wave table created
    makeSine(data);
    
    // init amplitude and pitch:
    data->amplitudeScaling = 0; // start note has no amplitude
    data->pitchIncrementer = 2.0275; // starting pitch is A-220

    // print info about program to terminal
    printf("\n---------------------------------------------------------------");
    printf("\n\t\tJAMBOARD WAVETABLE SYNTHESIS");
    printf("\n---------------------------------------------------------------\n");
    printf("\ntype 'z' and hit RETURN for operating info");
    printf("\ntype 'x' and hit RETURN to exit program");

    // initializes PortAudio library
    err = Pa_Initialize();
    if(err != paNoError) error(err, data);

    // setup output parameters for Pa_OpenStream()
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = 2; // stereo
    outputParameters.sampleFormat = paFloat32; // 32 bit floating point samples
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->
        defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    // start stream
    err = Pa_OpenStream(&stream,
                        NULL,
                        &outputParameters,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff,
                        monophonyCallback,
                        data);
    if(err != paNoError) error(err, data);

    // Start stream
    err = Pa_StartStream(stream);
    if(err != paNoError) error(err, data);

    // print indication that stream is open
    printf("\n\nSTREAM OPEN\n");

    Engine_Monophonic(data);
    
    // stream is stopped
    err = Pa_StopStream(stream);
    if(err != paNoError) error(err, data);

    // stream is closed
    err = Pa_CloseStream(stream);
    if(err != paNoError) error(err, data);
    printf("\nSTREAM CLOSED\n\n");

    // PortAudio library is terminated
    err = Pa_Terminate();
    if(err != paNoError) error(err, data);

    // unmap mapped data structs
    free(data);
    return 0;
}
