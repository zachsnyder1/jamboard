//
//  ui.h
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef ui_h
#define ui_h

#include "jamboard.h"

class UserInterface {
public:
    // in
    char get_input();
    void custom_wave(int[]);
    // out
    void salutation();
    void farewell();
    void help();
    void info(const char[]);
    void info(const char[], int);
    void error(const char[]);
    void error(const char[], void*);
};

#endif /* ui_h */
