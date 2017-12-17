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
    ShellController *shell = new ShellController();
    WaveTableSynth *synth = new WaveTableSynth();
    
    daw->add_instrument(synth);
    daw->add_controller(shell);
    daw->map_controller(shell, synth);
    daw->run(); // go


    delete daw;
    delete synth;
    delete shell;
    
    return 0;
}
