//
//  littledaw.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//
#include "littledaw.h"
#include "engine.h"
#include "envelope.h"


int main(int argc, char *argv[]) {
    Engine *engine = new Engine();
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

    
    engine->add_instrument(synth);
    engine->add_controller(shell);
    engine->map_controller(shell, synth);
    engine->run(); // go


    delete engine;
    delete synth;
    delete shell;
    
    return 0;
}
