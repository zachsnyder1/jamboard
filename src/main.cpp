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
    // init correct engine
    if(continuous) {
        Engine<InfiniteEnvelope> engine = Engine<InfiniteEnvelope>(1);
        engine.run(); // go
    } else {
        Engine<FiniteEnvelope> engine = Engine<FiniteEnvelope>();
        engine.run(); // go
    }
    return 0;
}
