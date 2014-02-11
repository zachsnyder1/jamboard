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

// define the struct used to pass user data into the callback function
typedef struct
{
	float sine[TABLE_SIZE]; //table for wavetable synthesis
	float left_phase; //left stereo channel data type
	float right_phase; //right stereo channel data type
	float pitchIncrementer; // used to set increment value in the callback function
}
paTestData;

// This is the callback function which is called repeatedly by portaudio,
// it is what actually drives the audio stream.  Every time it is called
// it sends another set of frames to the output buffer.  When the buffer
// filled it is called again, until something stops the stream.
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
		*out++ = data->sine[(int)data->left_phase];
		*out++ = data->sine[(int)data->right_phase];
		// Advances the wavetable index for left channel by 2 each time
		// the loop executes.  This essentially doubles the frequency
		// of the wavetable sample being read.
		data->left_phase += data->pitchIncrementer;
		// prevents loop from reading past the end of the sample in the wavetable
		if(data->left_phase >= TABLE_SIZE) data->left_phase -= TABLE_SIZE;
		// Same proceedures for right channel, except the frequency is triple
		// that of the wavetable sample.  This just adds a little more texture...
		data->right_phase += data->pitchIncrementer;
		if(data->right_phase >= TABLE_SIZE) data->right_phase -= TABLE_SIZE;
	}

	return paContinue;
}

int main(int argc, char *argv[])
{
	// set up some variables
	PaStreamParameters outputParameters; //struct for stream parameters
	PaStream *stream; // open stream
	PaError err; // err is used for error handling
	paTestData data; // create user data structure
	int x;
	int loop;
	char ch;
	int octave;

    // print info about program to terminal
    printf("\n\tJAMBOARD WAVETABLE SYNTHESIS");
    printf("\ntype 'z' and hit RETURN for operating info");
    printf("\ntype 'x' and hit RETURN to exit program");

	// sine wave table created
	for(x=0; x<TABLE_SIZE; x++)
		{
			data.sine[x] = (float) sin( ((double)x/(double)TABLE_SIZE) * M_PI * 2.);
		}

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

    // start the stream with a note to avoid a segmentation fault
    data.pitchIncrementer = 2.0275;
    // print indication that program is initiated
    printf("\n\nSTREAM OPEN\nJAM AWAY\n\n");
    // set octave value for loop
	octave = 3;
    // set up loop value
	loop = 1;
    // this loop continues to ask for user input, and executes user commands
	while(loop) {
        // print the prompt
		printf(">> ");
		ch = getc(stdin);
    	getc(stdin); // eats <RETURN>
        // these 'if' statements cause note commands to switch to notes in
        // the current octave
    	if(octave == 3) {
            // These switch statements change pitchIncrementer based on user input.
            // This is how the pitch is changed (see jamboardCALLBACK())
            // They also change octave, print operating info, or exit based
            // on user input.
    		switch(ch) {
    			case '`':
    				data.pitchIncrementer = 1.903;
    				break;
    			case '1':
    				data.pitchIncrementer = 2.0275;
    				break;
    			case 'q':
    				data.pitchIncrementer = 2.12;
    				break;
    			case '2':
    				data.pitchIncrementer = 2.245;
    				break;
    			case '3':
    				data.pitchIncrementer = 2.375;
    				break;
    			case 'e':
    				data.pitchIncrementer = 2.53;
    				break;
    			case '4':
    				data.pitchIncrementer = 2.67;
    				break;
    			case 'r':
    				data.pitchIncrementer = 2.83;
    				break;
    			case '5':
    				data.pitchIncrementer = 2.995;
    				break;
    			case '6':
    				data.pitchIncrementer = 3.17;
    				break;
    			case 'y':
    				data.pitchIncrementer = 3.36;
    				break;
    			case '7':
    				data.pitchIncrementer = 3.56;
    				break;
    			case 'u':
    				data.pitchIncrementer = 3.77;
    				break;
    			case '8':
    				data.pitchIncrementer = 4;
    				break;
    			case '-':
    				octave--;
    				printf("Octave: %d\n", octave);
    				break;
    			case '=':
    				octave++;
    				printf("Octave: %d\n", octave);
    				break;
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
    	} else if(octave == 2) {
    		switch(ch) {
    			case '`':
    				data.pitchIncrementer = 0.943;
    				break;
    			case '1':
    				data.pitchIncrementer = 0.99;
    				break;
    			case 'q':
    				data.pitchIncrementer = 1.04;
    				break;
    			case '2':
    				data.pitchIncrementer = 1.11;
    				break;
    			case '3':
    				data.pitchIncrementer = 1.19;
    				break;
    			case 'e':
    				data.pitchIncrementer = 1.27;
    				break;
    			case '4':
    				data.pitchIncrementer = 1.35;
    				break;
    			case 'r':
    				data.pitchIncrementer = 1.43;
    				break;
    			case '5':
    				data.pitchIncrementer = 1.5;
    				break;
    			case '6':
    				data.pitchIncrementer = 1.6;
    				break;
    			case 'y':
    				data.pitchIncrementer = 1.7;
    				break;
    			case '7':
    				data.pitchIncrementer = 1.8;
    				break;
    			case 'u':
    				data.pitchIncrementer = 1.903;
    				break;
    			case '8':
    				data.pitchIncrementer = 2.0275;
    				break;
    			case '-':
    				printf("Already at lowest octave");
    				break;
    			case '=':
    				octave++;
    				printf("Octave: %d\n", octave);
    				break;
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
    	} else if(octave == 4) {
    		switch(ch) {
    			case '`':
    				data.pitchIncrementer = 3.77;
    				break;
    			case '1':
    				data.pitchIncrementer = 4;
    				break;
    			case 'q':
    				data.pitchIncrementer = 4.25;
    				break;
    			case '2':
    				data.pitchIncrementer = 4.5;
    				break;
    			case '3':
    				data.pitchIncrementer = 4.78;
    				break;
    			case 'e':
    				data.pitchIncrementer = 5.04;
    				break;
    			case '4':
    				data.pitchIncrementer = 5.325;
    				break;
    			case 'r':
    				data.pitchIncrementer = 5.68;
    				break;
    			case '5':
    				data.pitchIncrementer = 6;
    				break;
    			case '6':
    				data.pitchIncrementer = 6.33;
    				break;
    			case 'y':
    				data.pitchIncrementer = 6.71;
    				break;
    			case '7':
    				data.pitchIncrementer = 7.1;
    				break;
    			case 'u':
    				data.pitchIncrementer = 7.54;
    				break;
    			case '8':
    				data.pitchIncrementer = 8;
    				break;
    			case '-':
    				octave--;
    				printf("Octave: %d\n", octave);
    				break;
    			case '=':
    				printf("Already at highest octave");
    				break;
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
