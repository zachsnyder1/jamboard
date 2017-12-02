//
//  ui.cpp
//  jamboard
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
    char command_check;
    int loop = 1;
    int x = 0;
    
    // print info to terminal
    std::cout << "\n\t\tSYNTHESIZE A CUSTOM TIMBRE\n\n";
    std::cout << "type 'z' and hit RETURN for custom synthesis info\n";
    std::cout << "type 'x' and hit RETURN to abort custom timbre and return to jam\n";
    std::cout << "type 's' and hit RETURN to synthesize timbre and return to jam\n\n";
    std::cout << "Enter amplitude of each harmonic:";
    // PROMPT FOR, PROCESS, AND STORE USER INPUT
    while(loop) {
        // prompt for and then store input value
        if(x == 0) {
            std::cout << "\n  Fundamental: ";
        } else {
            std::cout << "  Hamonic " << x << ": ";
        }
        // get a char
        fscanf(stdin, "%s", &command_check);
        // process input (commands and errors)
        if(command_check == 'z') { // COMMAND: print info
            this->help();
        } else if(command_check == 'x') { // COMMAND: abort custom timbre synthesis
            std::cout << "\n\nCUSTOM TIMBRE SYNTHESIS ABORTED\n\n";
            getc(stdin);  // eat enter
            return;
        } else if(command_check == 's' && x == 0) { // ERROR: need more user info
            std::cout << "\n\tERROR: Enter amplitude for one or more frequency\n";
        } else if(command_check == 's' || x >= HIGHEST_HARMONIC-1) { // COMMAND: synthesize
            std::cout << "\n";
            return;
        } else if(atoi(&command_check) > 100) { // ERROR: invalid amplitude value
            std::cout << "\n\tERROR: Amplitude needs to be 0<=X<=100\n";
        } else { // STORE USER INPUT, PROMPT FOR NEXT HARMONIC
            // store in amplitude array
            harmonic_amplitudes[x] = (float)atoi(&command_check);
            x++;
        }
    }
    getc(stdin); // eat enter
    return;
}

void UserInterface::salutation() {
    std::cout << "\n---------------------------------------------------------------";
    std::cout << "\n\t\tJAMBOARD WAVETABLE SYNTHESIS";
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
    std::cout << "RETURN. Each command is\none symbol long.  Don't";
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
    std::cout << "     =   --->  Go up an octave (if it were '+' you";
    std::cout << "would have to type shift...)\n";
    std::cout << "     A   --->  Timbre = sine wave\n";
    std::cout << "     S   --->  Timbre = square wave (default)\n";
    //std::cout << "     C   --->  Synthesize custom timbre\n";
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
