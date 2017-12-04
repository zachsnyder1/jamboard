//
//  littledaw.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef littledaw_h
#define littledaw_h

#define SQUARE_MAX_AMP (0.05)
#define CUSTOM_MAX_AMP (0.01)
#define SINE_MAX_AMP (0.5)
#define START_NOTE (2.0275)
#define SAMPLE_RATE (44100)
#define NUM_CHANNELS (2)
#define DEFAULT_NUM_VOICES (6)
#define FRAMES_PER_BUFFER (192)
#define HIGHEST_HARMONIC (10)
#define TABLE_SIZE (400)
#ifndef M_PI
#define M_PI (3.14159265)
#endif

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <math.h>
#include "portaudio.h"

#endif  // littledaw_h
