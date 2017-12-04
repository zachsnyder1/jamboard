//
//  ui.cpp
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "ui.h"

char UserInterface::get_input() {
    char command;
    
    std::cout << ">> "; // print the prompt
    command = getc(stdin); // get command char
    getc(stdin); // eat newline
    return command;
}

void UserInterface::custom_wave(int *harmonic_amplitudes) {
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

void UserInterface::salutation() {
    std::cout << "\n---------------------------------------------------------------";
    std::cout << "\n\t\tLITTLE-DAW WAVETABLE SYNTHESIS";
    std::cout << "\n---------------------------------------------------------------\n";
    std::cout << "\ntype 'z' and hit RETURN for operating info";
    std::cout << "\ntype 'x' and hit RETURN to exit program";
    std::cout << "\nSTREAM OPEN\n\n";
}

void UserInterface::farewell() {
    std::cout << "\nSTREAM CLOSED\nGOODBYE\n";
}

void UserInterface::help() {
    std::cout << "\nTo enter a command, type its letter and hit ";
    std::cout << "RETURN. Each command is\none symbol long.  Don't ";
    std::cout << "include quotations or brackets.\n\n\n";
    std::cout << "      PARTIAL SCHEMATIC OF KEYBOARD:\n\n";
    std::cout << "---------------- KEYS: -----------------------|\n";
    std::cout << "                                              |\n";
    std::cout << "[`][1]   [2] [3]   [4]   [5] [6]   [7]   [8]  |\n";
    std::cout << "      [q]       [e]   [r]       [y]   [u]     |\n";
    std::cout << "                                              |\n";
    std::cout << "----------- CORRESPONDING NOTES: -------------|\n";
    std::cout << "                                              |\n";
    std::cout << "[g#][a]   [b] [c]   [d]   [e] [f]   [g]   [a] |\n";
    std::cout << "       [a#]      [c#]  [d#]      [f#]  [g#]   |\n";
    std::cout << "______________________________________________|\n";
    std::cout << "\n\n   OTHER COMMANDS:\n   --------------\n";
    std::cout << "     -   --->  Go down an octave\n";
    std::cout << "     =   --->  Go up an octave\n";
    std::cout << "     A   --->  Timbre = sine wave\n";
    std::cout << "     S   --->  Timbre = square wave (default)\n";
    std::cout << "     C   --->  Timbre = custom waveform\n";
    std::cout << "     z   --->  Print help\n";
    std::cout << "     x   --->  EXIT PROGRAM\n\n";
}

void UserInterface::info(const char *msg) {
    std::cout << "[Info] " << msg << "\n";
}

void UserInterface::info(const char *msg, int arg) {
    std::cout << "[Info] ";
    printf(msg, arg);
    std::cout << "\n";
}

void UserInterface::error(const char *msg) {
    std::cerr << "[Error] " << msg << "\n";
}

void UserInterface::error(const char *msg, void *args) {
    std::cout << "[Error] ";
    printf(msg, args);
    std::cout << "\n";
}
