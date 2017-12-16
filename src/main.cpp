//
//  littledaw.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//
#include "littledaw.h"
#include "daw.h"
#include "envelope.h"


int main(int argc, char *argv[]) {
    Daw *daw = new Daw();
    WaveTableSynth *synth;
    ShellController *shell = new ShellController();
    int continuous = 0;
    int option;

    // parse command line args
    while ((option = getopt(argc, argv, "c")) != -1) {
        switch (option) {
            case 'c':
                continuous = 1;
                break;
        }
    }
    
    if(continuous) {
        synth = new WaveTableSynth(1);
    } else {
        synth = new WaveTableSynth();
    }

    
    daw->add_instrument(synth);
    daw->add_controller(shell);
    daw->map_controller(shell, synth);
    daw->run(); // go


    delete daw;
    delete synth;
    delete shell;
    
    return 0;
}
