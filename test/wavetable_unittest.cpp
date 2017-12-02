//
//  wavetable_unittest.cpp
//  jamboard
//
//  Created by Zach Snyder on 11/26/17.
//  Copyright © 2017 Zach Snyder. All rights reserved.
//

#include "../src/wavetable.h"
#include <fftw3.h>
#include "gtest/gtest.h"

namespace wavetabletest {

TEST(WaveTableInit, InitAsSquare) {}

class WaveTableWaveForms : public ::testing::Test {
protected:
    WaveTable table;
};

TEST_F(WaveTableWaveForms, SineWave) {}
TEST_F(WaveTableWaveForms, SquareWave) {}
TEST_F(WaveTableWaveForms, CustomWave) {}

} // wavetabletest
