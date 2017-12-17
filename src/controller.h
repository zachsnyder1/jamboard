//
//  controller.h
//  little-daw
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#ifndef controller_h
#define controller_h

#include <iostream>

// Controller abstract base class
class Controller {
public:
    // abstract interface
    virtual void input_loop(bool*, void*, void*) {}; // Engine*, Instrument*
    virtual void salutation() {};
    virtual void farewell() {};
    virtual void help() {};
    virtual void info(const char[]) {};
    virtual void info(const char[], int) {};
    virtual void error(const char[]) {};
    virtual void error(const char[], void*) {};
};

class ShellController : public Controller {
public:
    // Controller interface overrides
    void input_loop(bool*, void*, void*); // Engine*, Instrument*
    void salutation();
    void farewell();
    void help();
    void info(const char[]);
    void info(const char[], int);
    void error(const char[]);
    void error(const char[], void*);
    // ShellController specific methods
    void custom_wave(int[]);
};

#endif /* controller_h */
