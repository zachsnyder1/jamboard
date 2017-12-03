/*
 Jamboard, running MonophonicEngine.  Enjoy.
   -Z
*/
#include "jamboard.h"
#include "engine.h"
#include "envelope.h"

/*
 Make an engine object, run it.
*/
int main(int argc, char *argv[]) {
    Engine *engine;
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
        engine = new Engine(1);
    } else {
        engine = new Engine();
    }
    engine->run(); // go
    delete engine;
    
    return 0;
}
