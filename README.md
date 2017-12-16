# LITTLE-DAW

## THE VISION

A light-weight, headless audio workstation with basic synthesis, processing,
and sampling utilities, small enough to run on Raspberry Pi.

## CURRENT STATUS

So far little-daw is a minimal skeleton, with one wavetable synthesizer 
that allows you to play a melody using the keys on your keyboard.  The timbre 
can be a sine wave, square wave, or custom waveform built based on the 
amplitudes of the fundamental and the first X harmonics.


## COMPILING

Compile from the /src directory.  You may need to explicitly link to the
'portaudio' library. If you use g++, compilation may look like this:

	      g++ *.cpp -lportaudio -o littledaw


## INVOKING

In (default) polyphonic mode:

        ./littledaw

In continuous signal mode:

        ./littledaw -c


## COMMANDS

When the program initiates it provides a prompt ('>> ').  This is where you
enter all commands, including note changes.  To enter a command, type it and
then hit RETURN.

Here is a schematic showing spatial arrangement of keys
and their associated note values:
```
--------------------- KEYS: ----------------|
                                            |
     [`][1][2][3][4][5][6][7][8]            |
          [q]   [e][r]   [y][u]             |
            [a][s][d][f][g][h][j][k]        |
              [z]   [c][v]   [n][m]         |
                                            |
------------ CORRESPONDING NOTES: ----------|
                                            |
     [g#][a][b][c][d][e][f][g][a]           |
          [a#]  [c#][d#] [f#][g#]           |
            [a][b][c][d][e][f][g][a]        |
              a#]  [c#][d#] [f#][g#]        |
____________________________________________|
```


## PORTAUDIO DEPENDENCY

This program is written using PortAudio, so you will need to download and install
it if you are building this project.  On OSX, with brew, it is easy:

        brew install portaudio

Otherwise, you can download a release from the PortAudio website:

        http://portaudio.com/download.html


## LICENSE

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files
 (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.