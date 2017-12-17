//
//  controller.cpp
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "controller.h"
#include "daw.h"
#include "instrument.h"

void ShellController::salutation() {
    std::cout << "\n---------------------------------------------------------------";
    std::cout << "\n\t\tLITTLE-DAW WAVETABLE SYNTHESIS";
    std::cout << "\n---------------------------------------------------------------\n";
    std::cout << "\ntype 'Z' for operating info";
    std::cout << "\ntype 'X' to exit program\n\n";
}

void ShellController::farewell() {
    std::cout << "\nSTREAM CLOSED\nGOODBYE\n";
}

void ShellController::help() {
    std::cout << "\nTo enter a command, type its letter and hit ";
    std::cout << "RETURN. Each command is\none symbol long.  Don't ";
    std::cout << "include quotations or brackets.\n\n\n";
    std::cout << "      PARTIAL SCHEMATIC OF KEYBOARD:\n\n";
    std::cout << "--------------------- KEYS: ----------------|\n";
    std::cout << "                                            |\n";
    std::cout << "     [`][1][2][3][4][5][6][7][8]            |\n";
    std::cout << "          [q]   [e][r]   [y][u]             |\n";
    std::cout << "            [a][s][d][f][g][h][j][k]        |\n";
    std::cout << "              [z]   [c][v]   [n][m]         |\n";
    std::cout << "                                            |\n";
    std::cout << "------------ CORRESPONDING NOTES: ----------|\n";
    std::cout << "                                            |\n";
    std::cout << "     [g#][a][b][c][d][e][f][g][a]           |\n";
    std::cout << "          [a#]  [c#][d#] [f#][g#]           |\n";
    std::cout << "            [a][b][c][d][e][f][g][a]        |\n";
    std::cout << "              a#]  [c#][d#] [f#][g#]        |\n";
    std::cout << "____________________________________________|\n";
    std::cout << "\n\n   OTHER COMMANDS:\n   --------------\n";
    std::cout << "     A   --->  Timbre = sine wave\n";
    std::cout << "     S   --->  Timbre = square wave (default)\n";
    std::cout << "     C   --->  Timbre = custom waveform\n";
    std::cout << "     Z   --->  Print help\n";
    std::cout << "     X   --->  EXIT PROGRAM\n\n";
}

void ShellController::info(const char *msg) {
    std::cout << "[Info] " << msg << "\n";
}

void ShellController::info(const char *msg, int arg) {
    std::cout << "[Info] ";
    printf(msg, arg);
    std::cout << "\n";
}

void ShellController::error(const char *msg) {
    std::cerr << "[Error] " << msg << "\n";
}

void ShellController::error(const char *msg, void *args) {
    std::cerr << "[Error] ";
    printf(msg, args);
    std::cerr << "\n";
}

void ShellController::custom_wave(int *harmonic_amplitudes) {
    // setup local variables
    std::string command_str;
    int command = 0;
    int loop = 1;
    int x = 0;
    
    // print info to terminal
    std::cout << "\n\t\tSYNTHESIZE A CUSTOM TIMBRE\n\n";
    std::cout << "type 'z' and hit RETURN for custom synthesis info\n";
    std::cout << "type 'x' and hit RETURN to abort custom timbre and return to jam\n";
    std::cout << "type 's' and hit RETURN to synthesize timbre and return to jam\n\n";
    std::cout << "Enter amplitude of each harmonic:\n";
    // PROMPT FOR, PROCESS, AND STORE USER INPUT
    while(loop) {
        // prompt for and then store input value
        if(x == 0) {
            std::cout << "  Fundamental: ";
        } else {
            std::cout << "  Hamonic " << x << ": ";
        }
        // get command
        std::getline(std::cin, command_str);
        command = std::atoi(command_str.c_str());
        // validate not empty input
        if(command_str.length() < 1) {
            std::cout << "\n\tERROR: invalid command/value\n";
            std::cout << "\t       amplitudes are {0 <= a <= 100}\n";
            std::cout << "\t       enter 'z' for custom synthesis help\n\n";
        // COMMAND 'z': print help
        } else if(command_str[0] == 'z') {
            std::cout << "\n   Enter the amplitude {0 <= a <= 100} of each\n";
            std::cout << "   harmonic, starting with the fundamental.  You can\n";
            std::cout << "   synthesize at any point with the 's' command.\n\n";
            std::cout << "         COMMANDS:\n         --------\n";
            std::cout << "     s   --->  Synthesize waveform\n";
            std::cout << "     z   --->  Print help\n";
            std::cout << "     x   --->  Abort custom synthesis\n\n";
        // COMMAND 'x': abort custom timbre synthesis
        } else if(command_str[0] == 'x') {
            std::cout << "\nCUSTOM TIMBRE SYNTHESIS ABORTED\n";
            loop = 0;
            return;
        // COMMAND 's' without input: need more user info
        } else if(command_str[0] == 's' && x == 0) {
            std::cout << "\n\n\tERROR: Enter amplitude for one or more frequency\n\n";
        // COMMAND 's': synthesize
        } else if(command_str[0] == 's' || (x >= (HIGHEST_HARMONIC-1))) {
            std::cout << "\n";
            loop = 0;
        // STORE USER INPUT, PROMPT FOR NEXT HARMONIC
        } else if((command >= 0) && (command <= 100)) {
            harmonic_amplitudes[x] = command;
            x++;
        // ERROR: invalid input
        } else if((command < 0) || (command > 100)) {
            std::cout << "\tERROR: amplitudes are {0 <= X <= 100}\n";
        }
    }
    std::cin.clear();
    return;
}

void ShellController::input_loop(bool *loop, void *daw, void *instrument) {
    char command;
    int ha[HIGHEST_HARMONIC] = {0};
    Daw *e = (Daw*)daw;
    Instrument *inst = (Instrument*)instrument;
    
    std::cout << ">> "; // print the prompt
    command = getc(stdin); // get command char
    getc(stdin); // eat newline
    switch(command) {
            /* NOTE COMMANDS: Pitch is calculated based on the octave and
            the frequencies found in the BASE_HZ array.
            Then, incomming_note is changed to 1,
            causing the polyphony operator child
            process to execute the note with
            envelope.
            */
        case '`':
            inst->trigger(Instrument::GS2);
            break;
        case '1':
            inst->trigger(Instrument::A3);
            break;
        case 'q':
            inst->trigger(Instrument::AS3);
            break;
        case '2':
            inst->trigger(Instrument::B3);
            break;
        case '3':
            inst->trigger(Instrument::C3);
            break;
        case 'e':
            inst->trigger(Instrument::CS3);
            break;
        case '4':
            inst->trigger(Instrument::D3);
            break;
        case 'r':
            inst->trigger(Instrument::DS3);
            break;
        case '5':
            inst->trigger(Instrument::E3);
            break;
        case '6':
            inst->trigger(Instrument::F3);
            break;
        case 'y':
            inst->trigger(Instrument::FS3);
            break;
        case '7':
            inst->trigger(Instrument::G3);
            break;
        case 'u':
            inst->trigger(Instrument::GS3);
            break;
        case '8':
            inst->trigger(Instrument::A4);
            break;
        case 'a':
            inst->trigger(Instrument::A4);
            break;
        case 'z':
            inst->trigger(Instrument::AS4);
            break;
        case 's':
            inst->trigger(Instrument::B4);
            break;
        case 'd':
            inst->trigger(Instrument::C4);
            break;
        case 'c':
            inst->trigger(Instrument::CS4);
            break;
        case 'f':
            inst->trigger(Instrument::D4);
            break;
        case 'v':
            inst->trigger(Instrument::DS4);
            break;
        case 'g':
            inst->trigger(Instrument::E4);
            break;
        case 'h':
            inst->trigger(Instrument::F4);
            break;
        case 'n':
            inst->trigger(Instrument::FS4);
            break;
        case 'j':
            inst->trigger(Instrument::G4);
            break;
        case 'm':
            inst->trigger(Instrument::GS4);
            break;
        case 'k':
            inst->trigger(Instrument::A5);
            break;
        // TIMBRE COMMANDS: Wavetable is rewritten to create a new timbre
        case 'A': // SINE WAVE
            e->mixer->fade_out();
            inst->command(WaveTableSynth::COMMAND_SINE_WAVE, this);
            e->mixer->fade_in();
            break;
        case 'S': // SQUARE WAVE
            e->mixer->fade_out();
            inst->command(WaveTableSynth::COMMAND_SQUARE_WAVE, this);
            e->mixer->fade_in();
            break;
        case 'C': // CREATE CUSTOM TIMBRE
            e->mixer->fade_out();
            this->custom_wave(ha);
            inst->command(WaveTableSynth::COMMAND_CUSTOM_WAVE, ha);
            e->mixer->fade_in();
            break;
        case 'Z': // PRINT OPERATING INFO TO TERMINAL
            this->help();
            break;
        case 'X':
            // stops the while loop
            *loop = false;
            e->mixer->fade_out();
            break;
        default:
            this->info("NOT A NOTE!\n");
    }
}
