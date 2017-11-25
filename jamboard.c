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
#define HIGHEST_HARMONIC (31)
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
    float voicePosition[VOICES]; // used to keep track of individual voices when in polyphony mode
    float voiceIncrementer[VOICES]; // used to increment individual voices
    float voiceAmpScaling[VOICES]; // used to apply envelope to individual voices
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

// define the struct used to hold data for the polyphony operator
typedef struct
{
    int engine_running;
    int incomming_note;
    int channel;
    float incomming_pitchIncrementer;
}
polyphonyData;

/* This is the callback function which is called repeatedly by portaudio,
   it is what actually drives the audio stream.  Every time it is called
   it sends another set of frames to the output buffer.  When the buffer
   filled it is called again, until something stops the stream. */
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

/* This is the callback function which is called repeatedly by portaudio,
   it is what actually drives the audio stream.  Every time it is called
   it sends another set of frames to the output buffer.  When the buffer
   filled it is called again, until something stops the stream. */
static int polyphonyCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{
    // setting up arguments for use
    paUserData *data = (paUserData*)userData;
    float *out = (float*)outputBuffer;
    unsigned long i; //for looping use...
    int x; // for looping...

    // casting the unused arguments as void to avoid 'unused' errors 
    (void) timeInfo;
    (void) statusFlags;
    (void) inputBuffer;

    // the loop that writes each frame to the output buffer
    for(i=0; i<framesPerBuffer; i++)
    {
        float added_voice_amplitudes = 0;
        // voices are added together
        for(x = 0; x < VOICES; x++) {
            added_voice_amplitudes += (data->voiceAmpScaling[x] *
                data->table[(int)data->voicePosition[x]]) / VOICES;
        }
        // frames alternate between left and right channels, thereby
        // multiplexing stereo information into the serial buffer
        // currently its effectively a mono stream...
        *out++ = added_voice_amplitudes;
        *out++ = added_voice_amplitudes;
        // table position for each voice is incremented
        for(x = 0; x < VOICES; x++) {
            data->voicePosition[x] += data->voiceIncrementer[x];
        }
        // table positions are limited to TABLE_SIZE
        for(x = 0; x < VOICES; x++) {
            if(data->voicePosition[x] >= TABLE_SIZE) data->voicePosition[x] -= TABLE_SIZE;
        }
    }

    return paContinue;
}

/* These INIT functions initiate variables that will be used in a specific mode of the program */

// initiates envelope settings based on command line args; only called when in mode 'e'
int Init_EnvelopeSettings(envelopeData *envelope_settings, int argc, char *argv[])
{
    // check that envelope parameters are in correct range
    if(atoi(argv[2]) > 100 || atoi(argv[3]) > 100 || atoi(argv[4]) > 100) {
        printf("\nERROR: Envelope amplitude settings (first 3 args) ");
        printf("need to be 0 <= X <= 100\n\n");
        return 1;
    }
    if(atoi(argv[2]) < 0 || atoi(argv[3]) < 0 || atoi(argv[4]) < 0) {
        printf("\nERROR: Envelope amplitude settings (first 3 args) ");
        printf("need to be 0 <= X <= 100\n\n");
        return 1;
    }
    if(atoi(argv[5]) > 6 || atoi(argv[6]) > 6 || atoi(argv[7]) > 6) {
        printf("\nERROR: Envelope duration settings (last 3 args) ");
        printf("need to be 1 <= X <= 6\n\n");
        return 1;
    }
    if(atoi(argv[5]) < 1 || atoi(argv[6]) < 1 || atoi(argv[7]) < 1) {
        printf("\nERROR: Envelope duration settings (last 3 args) ");
        printf("need to be 1 <= X <= 6\n\n");
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

// finds an open polyphony channel for the polyphony operator child process
int find_OpenChanel(paUserData *data)
{
    int i;
    // find a channel that has a zero amplitude signal, return that channel
    for(i = 0; i < VOICES; i++) {
        if(data->voiceAmpScaling[i] == 0) return i;
    }
    // returns -1 if no channel is open
    return -1;
}

/* Forks to execute a note with envelope, returning control to the polyphony operator
   so that it can process the next incomming note. */
void voiceFork(paUserData *data,
    envelopeData *envelope_settings,
    polyphonyData *polyData)
{
    // set up local variables
    int channel_local = polyData->channel;
    double attackLocal = envelope_settings->attack;
    double decayLocal = envelope_settings->decay;
    double releaseLocal = envelope_settings->release;

    // to distinguish parent and child processes
    pid_t parentPID = getpid();
    // fork, child PID
    pid_t childPID = fork();

/* ---------------------(CHILD PROCESS)-----------------
   ------------------ ENVELOPE EXECUTION --------------- */
    if(getpid() != parentPID) {
        // execute attack
        for(data->voiceAmpScaling[channel_local] = envelope_settings->initial;
            data->voiceAmpScaling[channel_local] < envelope_settings->peak;
            data->voiceAmpScaling[channel_local] += attackLocal) {
            Pa_Sleep(0);
        }
        // execute decay
        for(data->voiceAmpScaling[channel_local] = envelope_settings->peak;
            data->voiceAmpScaling[channel_local] > envelope_settings->sustain;
            data->voiceAmpScaling[channel_local] -= decayLocal) {
            Pa_Sleep(0);
        }
        // execute release
        for(data->voiceAmpScaling[channel_local] = envelope_settings->sustain;
            data->voiceAmpScaling[channel_local] > 0;
            data->voiceAmpScaling[channel_local] -= releaseLocal) {
            Pa_Sleep(0);
        }

        // make sure scaling factor is exactly 0
        data->voiceAmpScaling[channel_local] = 0;

        // exit this process when envelope has finished executing
        _exit(0);
    }

    return;
}

// create a custom timbre based on user input
void create_CustomTimbre(paUserData *data)
{
    // setup local variables
    int harm_amplitudes[HIGHEST_HARMONIC+2] = {0}; // array for amplitude values of harmonics
    char command_check;
    int loop = 1;
    int x = 0;
    int i;
    float addedAmps_scaling;
    float added_amplitudes;
    float temp_table[TABLE_SIZE];

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
        } else if(command_check == 's' || x >= HIGHEST_HARMONIC) { // COMMAND: synthesize
            printf("\n");
            loop = 0;
        } else if(atoi(&command_check) > 100) { // ERROR: invalid amplitude value
            printf("\n\tERROR: Amplitude needs to be 0<=X<=100\n");
        } else { // STORE USER INPUT, PROMPT FOR NEXT HARMONIC
            // set array value to input value
            harm_amplitudes[x+1] = atoi(&command_check);
            x++;
        }
    }

    /* CREATE NEW WAVE TABLE FROM USER INPUT
       Overall amplitude of the synthesized waveform is scaled
       to avoid an arithmetic overflow.
       NOTE: first item in array kept getting corrupted, so it is
       ignored in these loops... */
    for(i = 1; i < (HIGHEST_HARMONIC+1); i++) {
        addedAmps_scaling += harm_amplitudes[i];
    }

    // harmonics are added together, one wavetable index at a time
    for(i = 0; i < TABLE_SIZE; i++) {
        added_amplitudes = 0;
        for(x = 1; x < (HIGHEST_HARMONIC+1); x++) {
            added_amplitudes += (((float)harm_amplitudes[x] / 100) *
                data->table[((x) * i) % TABLE_SIZE]) / ((float)addedAmps_scaling / 100);
        }
        temp_table[i] = added_amplitudes;
    }

    // main wavetable is replaced with data stored in the temporary table
    for(i = 0; i < TABLE_SIZE; i++) {
        data->table[i] = temp_table[i];
    }

    // eat enter; needed for some reason
    getc(stdin);

    return;
}

/* The engines prompt for and process user commands. */

// This engine is used if envelope arguments were included on the command line
void Engine_Polyphonic(paUserData *data,
    envelopeData *envelope_settings,
    polyphonyData *polyData)
{
    // set up some local variables
    char ch;
    char waveform = 'a';
    int loop = 1;
    int octave = 3;
    int x = 0;
    float total_output;

    // print engine info
    printf("ENGINE INITIATED: polyphonic (with envelope)\nPROCEED TO JAM\n\n");

    // this loop continues to ask for user input, and executes user commands
    // note commands are fed to the polyphony operator child process
    while(loop) {
        // print the prompt
        printf(">> ");
        ch = getc(stdin); // gets command character
        getc(stdin); // eats <RETURN>

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
                polyData->incomming_pitchIncrementer = (baseHZ[11] *
                    pow((double)2, (double)(octave-1))) / 110;
                polyData->incomming_note = 1;
                break;
            case '1':
                polyData->incomming_pitchIncrementer = (baseHZ[0] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case 'q':
                polyData->incomming_pitchIncrementer = (baseHZ[1] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case '2':
                polyData->incomming_pitchIncrementer = (baseHZ[2] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case '3':
                polyData->incomming_pitchIncrementer = (baseHZ[3] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case 'e':
                polyData->incomming_pitchIncrementer = (baseHZ[4] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case '4':
                polyData->incomming_pitchIncrementer = (baseHZ[5] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case 'r':
                polyData->incomming_pitchIncrementer = (baseHZ[6] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case '5':
                polyData->incomming_pitchIncrementer = (baseHZ[7] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case '6':
                polyData->incomming_pitchIncrementer = (baseHZ[8] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case 'y':
                polyData->incomming_pitchIncrementer = (baseHZ[9] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case '7':
                polyData->incomming_pitchIncrementer = (baseHZ[10] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case 'u':
                polyData->incomming_pitchIncrementer = (baseHZ[11] *
                    pow((double)2, (double)octave)) / 110;
                polyData->incomming_note = 1;
                break;
            case '8':
                polyData->incomming_pitchIncrementer = (baseHZ[0] *
                    pow((double)2, (double)(octave+1))) / 110;
                polyData->incomming_note = 1;
                break;
            /* OCTAVE CHANGE COMMANDS: Octave is used to calculate the
                                       pitch incrementer for a note in a
                                       given octave (see above).  Octave
                                       value ranges from 2 to 5.
            */
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
            /* TIMBRE COMMANDS: Wavetable is rewritten to create a new timbre */
            case 'A': // CHANGE TO SINE WAVE
                // wait for all polyphony channels to reach zero amplitude
                total_output = 1;
                while(total_output) {
                    total_output = 0;
                    for(x=0; x<VOICES; x++) {
                        total_output += data->voiceAmpScaling[x];
                    }
                }
                // redefine table as sine wave
                for(x=0; x<TABLE_SIZE; x++)
                    {
                        data->table[x] = (float)sin(((double)x/(double)TABLE_SIZE) * M_PI * 2.);
                    }
                // change waveform indicator
                waveform = 'a';
                break;
            case 'S': // CHANGE TO PSEUDO SQUARE WAVE
                // wait for all polyphony channels to reach zero amplitude
                total_output = 1;
                while(total_output) {
                    total_output = 0;
                    for(x=0; x<VOICES; x++) {
                        total_output += data->voiceAmpScaling[x];
                    }
                }

                // redefine table as square wave
                for(x=0; x<TABLE_SIZE; x++)
                    {
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

                // change waveform indicator
                waveform = 's';
                break;
            case 'C': // CREATE CUSTOM TIMBRE
                // wait for all polyphony channels to reach zero amplitude
                total_output = 1;
                while(total_output) {
                    total_output = 0;
                    for(x=0; x<VOICES; x++) {
                        total_output += data->voiceAmpScaling[x];
                    }
                }

                // change table to sinewave for reference, if not already a sinewave
                if(waveform != 'a') {
                    // redefine table as sine wave
                    for(x=0; x<TABLE_SIZE; x++)
                        {
                            data->table[x] = (float) sin( ((double)x/(double)TABLE_SIZE) * M_PI * 2.);
                        }
                }

                create_CustomTimbre(data);

                // change waveform indicator
                waveform = 'c';
                break;
            case 'z': // PRINT OPERATING INFO TO TERMINAL
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
                printf("     C   --->  Synthesize custom timbre\n");
                printf("     z   --->  Print operation info to terminal\n     x   --->  EXIT PROGRAM\n\n");
                break;
            case 'x': // EXIT PROGRAM
                // stop engine loop
                loop=0;
                // stop polyphony operator loop
                polyData->engine_running=0;

                // exit engine only when all polyphony channels reach zero amplitude
                int i = 0;
                total_output = 1;
                while(total_output) {
                    total_output = 0;
                    // signal output is calculated
                    for(x=0; x<VOICES; x++) {
                        total_output += data->voiceAmpScaling[x];
                    }
                    // if signal was not dead at second loop, print:
                    if(i == 1) printf("\nWAITING FOR SIGNAL TO DIE...\n");
                    i++;
                }
                break;

            default:
                printf("NOT A NOTE!\n");
        }
    }

    return;
}

void Engine_Monophonic(paUserData *data)
{
    // set up some local variables
    char ch;
    char waveform = 'a';
    int loop = 1;
    int octave = 3;
    int x = 0;

    // print engine info
    printf("ENGINE INITIATED: monophonic (continuous signal)\nPROCEED TO JAM\n\n");

    // this loop continues to ask for user input, and executes user commands
    while(loop) {
        // print the prompt
        printf(">> ");
        ch = getc(stdin); // gets command letter
        getc(stdin); // eats <RETURN>

        // used to fade in signal on first note, in order to prevent damage to speakers
        if(x == 0) {
            data->amplitudeScaling = 0;
        }

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
            /* TIMBRE COMMANDS: Wavetable is rewritten to create a new timbre */
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

                // change waveform indicator
                waveform = 'a';
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

                // fade signal in again
                for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling < 1; data->amplitudeScaling += 0.000005) {
                    Pa_Sleep(0);
                }

                // change waveform indicator
                waveform = 's';
                break;
            case 'C': // CREATE CUSTOM TIMBRE
                // fade out signal
                for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling > 0; data->amplitudeScaling -= 0.000005) {
                    Pa_Sleep(0);
                }

                // change table to sinewave for reference, if not already a sinewave
                if(waveform != 'a') {
                    // redefine table as sine wave
                    for(x=0; x<TABLE_SIZE; x++)
                        {
                            data->table[x] = (float) sin( ((double)x/(double)TABLE_SIZE) * M_PI * 2.);
                        }
                }

                create_CustomTimbre(data);

                // change waveform indicator
                waveform = 'c';
                // fade signal in again
                for(data->amplitudeScaling = data->amplitudeScaling; data->amplitudeScaling < 1; data->amplitudeScaling += 0.000005) {
                    Pa_Sleep(0);
                }
                break;
            case 'z': // PRINT OPERATING INFO TO TERMINAL
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
                printf("     C   --->  Synthesize custom timbre\n");
                printf("     z   --->  Print operation info to terminal\n     x   --->  EXIT PROGRAM\n\n");
                
                if(x == 0) x = -1; // prevents signal from fading in if 'z' is first command
                break;
            case 'x':
                // stops the while loop
                loop=0;

                // fades out amplitude to avoid damage to speakers
                for(data->amplitudeScaling = data->amplitudeScaling;
                    data->amplitudeScaling > 0;
                    data->amplitudeScaling -= 0.000005) {
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
            for(data->amplitudeScaling = data->amplitudeScaling;
                data->amplitudeScaling < 1;
                data->amplitudeScaling += 0.000005) {
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
    paUserData *data; // forward declaration of mapped paUserdata instance
    envelopeData *envelope_settings; // forward declaration of mapped envelopeData instance
    polyphonyData *polyData; // forward declaration of mapped polyphonyData instance
    int x; // for looping

    // map user data so that child process can use it (see below)
    paUserData *protoData = malloc(sizeof(paUserData)); // create user data structure
    data = (paUserData*)mmap(NULL, sizeof(paUserData),
        PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
    // check that mapping succeeded
    if(data == MAP_FAILED) {
        printf("\nUser data failed to map.\n");
        goto ARGerror;
    }

    // copy protoData into mapped 'data' and free protoData
    memcpy(data, protoData, sizeof(paUserData));
    free(protoData);

    // sine wave table created
    for(x=0; x<TABLE_SIZE; x++)
        {
            data->table[x] = (float) sin( ((double)x/(double)TABLE_SIZE) * M_PI * 2.);
        }

    /* DETERMINE MODE, SKIP TO APPROPRIATE JUMP POINT */
    if(argc < 2) {
        printf("\nERROR:  MODE ARGUMENT MISSING\n\n");
        printf("OPTIONS:\n\n");
        printf(" ./jamboard c\t---> continuous signal mode (monophonic) <---\n\n");
        printf(" ./jamboard e\t---> envelope control mode (polyphonic) <---\n\n");
        goto ARGerror;
    } else if(*argv[1] == 'e') {
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
    } else {
        printf("\nERROR:  INCORRECT MODE ARGUMENT\n\n");
        printf("OPTIONS:\n\n");
        printf(" ./jamboard c\t---> continuous signal mode (monophonic) <---\n\n");
        printf(" ./jamboard e\t---> envelope control mode (polyphonic) <---\n\n");
        goto ARGerror;
    }

/* SET ARGUMENTS FOR ENVELOPE CONTROL IF THEY WERE INCLUDED */
EnvelopeInit:

    printf(""); // For some reason I need a function here or the compiler yells at me

    /* ----- ALLOCATE, MAP, COPY, AND FREE HEAP MEMORY FOR 'e' MODE ----- */
    envelopeData *protoEnvelope_settings = malloc(sizeof(envelopeData)); // create envelope data structure
    // map envelope data so that child process can use it
    envelope_settings = (envelopeData*)mmap(NULL, sizeof(envelopeData),
        PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
    // check mapping success
    polyphonyData *protopolyData = malloc(sizeof(polyphonyData)); // create polyphony data structure
    // map polyphony data so that child process can use it
    polyData = (polyphonyData*)mmap(NULL, sizeof(polyphonyData),
        PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
    // check mapping success
    if(envelope_settings == MAP_FAILED || polyData == MAP_FAILED) {
        printf("\nUser data failed to map.\n");
        goto ARGerror;
    }
    // copy protoEnvelope_settings into mapped 'envelope_settings'
    memcpy(envelope_settings, protoEnvelope_settings, sizeof(envelopeData));
    free(protoEnvelope_settings); // free protoEnvelope_settings
    // same for polyData
    memcpy(polyData, protopolyData, sizeof(polyphonyData));
    free(protopolyData);

    /* ----- INITIATE NECESSARY VARIABLES FOR 'e' MODE -----*/
    if(Init_EnvelopeSettings(envelope_settings, argc, argv) == 1) return 0;
    // initiate polyphony data
    polyData->engine_running = 1;
    polyData->incomming_note = 0;
    // initiate some userData
    for(x=0; x<VOICES; x++) {
        data->voiceAmpScaling[x] = 0;
    }

/* Fork to create a child process that facilitates polyphony: */

    // to distinguish parent and child processes
    pid_t parentPID = getpid();
    // fork, child PID
    pid_t childPID = fork();

/* ---------------------(CHILD PROCESS)-----------------
   ------------------ POLYPHONY OPERATOR --------------- */
    if(getpid() != parentPID) {
        // operator keeps running until the engine terminates the loop (see Engine_Polyphonic())
        while(polyData->engine_running) {
            // Remains inactive until the engine signals 'incomming_note = 1'
            if(polyData->incomming_note == 1) {
                // gets an empty polyphony channel
                polyData->channel = find_OpenChanel(data);
                // if no channels are empty, prints...
                if(polyData->channel == -1) {
                    printf("\nMaximum number of voices\n");
                // else it calls voiceFork(), which executes the note with envelope
                } else {
                    data->voiceIncrementer[polyData->channel] =
                        polyData->incomming_pitchIncrementer;
                    voiceFork(data, envelope_settings, polyData);
                }
            // resets itself to wait for next incomming_note signal from engine
            polyData->incomming_note = 0;
            }
        }
        // when engine exits program, operator unmaps data struct instances
        munmap(data, sizeof(paUserData));
        munmap(envelope_settings, sizeof(envelopeData));
        munmap(polyData, sizeof(polyphonyData));
        return 0;
    }
/* ----------------------------------------------------- */

    goto postModeInit;

/* SKIP TO HERE AFTER MODE INITIATION FUNCTION IS CALLED (if any...) */
postModeInit:

    // Amplitude and pitch are set to avoid a segmentation fault when stream starts:
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
    if(err != paNoError) goto PAerror;

    // setup output parameters for Pa_OpenStream()
    outputParameters.device = Pa_GetDefaultOutputDevice(); // sends audio to default output
    outputParameters.channelCount = 2; // stereo
    outputParameters.sampleFormat = paFloat32; // 32 bit floating point samples
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->
        defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    // start stream; use polyphonyCallback if in mode 'e', else use monophonyCallback
    if(*argv[1] == 'e') {
        // Open stream
        err = Pa_OpenStream(
                    &stream,
                    NULL,
                    &outputParameters,
                    SAMPLE_RATE,
                    FRAMES_PER_BUFFER,
                    paClipOff,
                    polyphonyCallback,
                    data);
    } else {
        // Open stream
        err = Pa_OpenStream(
                    &stream,
                    NULL,
                    &outputParameters,
                    SAMPLE_RATE,
                    FRAMES_PER_BUFFER,
                    paClipOff,
                    monophonyCallback,
                    data);
    }
    if(err != paNoError) goto PAerror;

    // Start stream
    err = Pa_StartStream(stream);
    if(err != paNoError) goto PAerror;

    // print indication that stream is open
    printf("\n\nSTREAM OPEN\n");

    // Start correct engine
    if(*argv[1] == 'e') {
        Engine_Polyphonic(data, envelope_settings, polyData);
    } else if(*argv[1] == 'c') {
        Engine_Monophonic(data);
    }
    
    // stream is stopped
    err = Pa_StopStream(stream);
    if(err != paNoError) goto PAerror;

    // stream is closed
    err = Pa_CloseStream(stream);
    if(err != paNoError) goto PAerror;
    printf("\nSTREAM CLOSED\n\n");

    // PortAudio library is terminated
    err = Pa_Terminate();
    if(err != paNoError) goto PAerror;

    // unmap mapped data structs
    munmap(data, sizeof(paUserData));
    if(*argv[1] == 'e') munmap(envelope_settings, sizeof(envelopeData));
    if(*argv[1] == 'e') munmap(polyData, sizeof(polyphonyData));

    // program exits
    return err;
// PA error handling
PAerror:
    Pa_Terminate();

    // unmap mapped data structs
    munmap(data, sizeof(paUserData));
    if(*argv[1] == 'e') munmap(envelope_settings, sizeof(envelopeData));
    if(*argv[1] == 'e') munmap(polyData, sizeof(polyphonyData));

    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return err;
// command line arg error handling
ARGerror:
    // unmap mapped paUserData instance
    munmap(data, sizeof(paUserData));
    return 0;
}