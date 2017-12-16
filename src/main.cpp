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
    WaveTableSynth *synth = new WaveTableSynth();
    ShellController *shell = new ShellController();
    
    engine->add_instrument(synth);
    engine->add_controller(shell);
    engine->map_controller(shell, synth);
    engine->run(); // go


    delete engine;
    delete synth;
    delete shell;
    
    return 0;
}
