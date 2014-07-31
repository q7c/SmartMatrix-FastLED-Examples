/*

Modified by Jason Coon for Smart Matrix
from https://gist.github.com/anonymous/876f908333cd95315c35
removed/commented out audio examples until I can get a MSGEQ7 spectrum analyser wired up...

Funky Clouds Compendium (alpha version)
by Stefan Petrick

An ever growing list of examples, tools and toys
for creating one- and twodimensional LED effects.

Dedicated to the users of the FastLED v2.1 library
by Daniel Garcia and Mark Kriegsmann.

Provides basic and advanced helper functions.
Contains many examples how to creatively combine them.

Tested only @ATmega2560 (runs propably NOT on an Uno
or anything with less than 4kB RAM)
*/

#include "SmartMatrix.h"

SmartMatrix matrix;

const rgb24 COLOR_BLACK = { 0, 0, 0 };

// set master brightness 0-255 here to adjust power consumption
// and light intensity
#define BRIGHTNESS 255

//// MSGEQ7 wiring on spectrum analyser shield
//#define MSGEQ7_STROBE_PIN 4
//#define MSGEQ7_RESET_PIN 5
//#define AUDIO_LEFT_PIN 0
//#define AUDIO_RIGHT_PIN 1


// Matrix dimensions

const uint8_t WIDTH = 32;
const uint8_t HEIGHT = 32;

// number of LEDs based on fixed calculation matrix size
// do not touch
#define NUM_LEDS (WIDTH * HEIGHT)

// the rendering buffer (32*32)
rgb24 *leds;

// the oscillators: linear ramps 0-255
byte osci[4];

// sin8(osci) swinging between 0 - 15
byte p[4];

// store the 7 10Bit (0-1023) audio band values in these 2 arrays
int left[7];
int right[7];

/*
-------------------------------------------------------------------
Init Inpus and Outputs: LEDs and MSGEQ7
-------------------------------------------------------------------
*/
void setup() {
    // Initialize 32x32 LED Matrix
    matrix.begin();
    matrix.setBrightness(BRIGHTNESS);
    matrix.setColorCorrection(cc24);

    // Clear screen
    matrix.fillScreen(COLOR_BLACK);
    matrix.swapBuffers();

    leds = matrix.backBuffer();

    // just for debugging:
    //Serial.begin(9600);

    // InitMSGEQ7();
}

/*
-------------------------------------------------------------------
The main program
-------------------------------------------------------------------
*/
void loop()
{
    AutoRun();

    // Comment AutoRun out and test examples seperately here
    // SlowMandala3();

    // For discovering parameters of examples I reccomend to
    // tinker with a renamed copy ...
}

/*
-------------------------------------------------------------------
Basic Helper functions:

XY translate 2 dimensional coordinates into an index
Line draw a line
Pixel draw a pixel
ClearAll empty the screenbuffer
MoveOscillators increment osci[] and calculate p[]=sin8(osci)
InitMSGEQ7 activate the MSGEQ7
ReadAudio get data from MSGEQ7 into left[7] and right[7]

-------------------------------------------------------------------
*/

// translates from x, y into an index into the LED array and
// finds the right index for our matrix
int XY(int x, int y) {
    if (y > HEIGHT) { y = HEIGHT; }
    if (y < 0) { y = 0; }
    if (x > WIDTH) { x = WIDTH; }
    if (x < 0) { x = 0; }

    return (y * WIDTH) + x;
}

// Bresenham line algorythm based on 2 coordinates
void Line(int x0, int y0, int x1, int y1, byte color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    for (;;) {
        leds[XY(x0, y0)] = CHSV(color, 255, 255);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > dy) { err += dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

// write one pixel with HSV color to coordinates
void Pixel(int x, int y, byte color) {
    leds[XY(x, y)] = CHSV(color, 255, 255);
}

// delete the screenbuffer
void ClearAll()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = { 0, 0, 0 };
    }
}

/*
Oscillators and Emitters
*/

// set the speeds (and by that ratios) of the oscillators here
void MoveOscillators() {
    osci[0] = osci[0] + 5;
    osci[1] = osci[1] + 2;
    osci[2] = osci[2] + 3;
    osci[3] = osci[3] + 4;
    for (int i = 0; i < 4; i++) {
        p[i] = map(sin8(osci[i]), 0, 255, 0, WIDTH - 1); //why? to keep the result in the range of 0-WIDTH (matrix size)
    }
}

//// wake up the MSGEQ7
//void InitMSGEQ7() {
//    pinMode(MSGEQ7_RESET_PIN, OUTPUT);
//    pinMode(MSGEQ7_STROBE_PIN, OUTPUT);
//    digitalWrite(MSGEQ7_RESET_PIN, LOW);
//    digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
//}

// get the data from the MSGEQ7
// (still fucking slow...)
void ReadAudio() {
    //digitalWrite(MSGEQ7_RESET_PIN, HIGH);
    //digitalWrite(MSGEQ7_RESET_PIN, LOW);
    //for (byte band = 0; band < 7; band++) {
    //    digitalWrite(MSGEQ7_STROBE_PIN, LOW);
    //    delayMicroseconds(30);
    //    left[band] = analogRead(AUDIO_LEFT_PIN);
    //    right[band] = analogRead(AUDIO_RIGHT_PIN);
    //    digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
    //}
}

/*
-------------------------------------------------------------------
Functions for manipulating existing data within the screenbuffer:

DimAll scales the brightness of the screenbuffer down
Caleidoscope1 mirror one quarter to the other 3 (and overwrite them)
Caleidoscope2 rotate one quarter to the other 3 (and overwrite them)
Caleidoscope3 useless bullshit?!
Caleidoscope4 rotate and add the complete screenbuffer 3 times
Caleidoscope5 copy a triangle from the first quadrant to the other half
Caleidoscope6
SpiralStream stream = give it a nice fading tail
HorizontalStream
VerticalStream
VerticalMove move = just move it as it is one line down
Copy copy a rectangle
RotateTriangle copy + rotate a triangle (in 8*8)
MirrorTriangle copy + mirror a triangle (in 8*8)
RainbowTriangle static draw for debugging

-------------------------------------------------------------------
*/

// scale the brightness of the screenbuffer down
void DimAll(byte value)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i].nscale8(value);
    }
}

/*
Caleidoscope1 mirrors from source to A, B and C

y

| |
| B | C
|_______________
| |
|source | A
|_______________ x

*/
void Caleidoscope1() {
    for (int x = 0; x < WIDTH / 2; x++) {
        for (int y = 0; y < HEIGHT / 2; y++) {
            leds[XY(WIDTH - 1 - x, y)] = leds[XY(x, y)]; // copy to A
            leds[XY(x, HEIGHT - 1 - y)] = leds[XY(x, y)]; // copy to B
            leds[XY(WIDTH - 1 - x, HEIGHT - 1 - y)] = leds[XY(x, y)]; // copy to C
        }
    }
}

/*
Caleidoscope2 rotates from source to A, B and C

y

| |
| C | B
|_______________
| |
|source | A
|_______________ x

*/
void Caleidoscope2() {
    for (int x = 0; x < WIDTH / 2; x++) {
        for (int y = 0; y < HEIGHT / 2; y++) {
            leds[XY(WIDTH - 1 - x, y)] = leds[XY(y, x)]; // rotate to A
            leds[XY(WIDTH - 1 - x, HEIGHT - 1 - y)] = leds[XY(x, y)]; // rotate to B
            leds[XY(x, HEIGHT - 1 - y)] = leds[XY(y, x)]; // rotate to C
        }
    }
}

// adds the color of one quarter to the other 3
void Caleidoscope3() {
    for (int x = 0; x < WIDTH / 2; x++) {
        for (int y = 0; y < HEIGHT / 2; y++) {
            leds[XY(WIDTH - 1 - x, y)] += leds[XY(y, x)]; // rotate to A
            leds[XY(WIDTH - 1 - x, HEIGHT - 1 - y)] += leds[XY(x, y)]; // rotate to B
            leds[XY(x, HEIGHT - 1 - y)] += leds[XY(y, x)]; // rotate to C
        }
    }
}

// add the complete screenbuffer 3 times while rotating
void Caleidoscope4() {
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            leds[XY(WIDTH - 1 - x, y)] += leds[XY(y, x)]; // rotate to A
            leds[XY(WIDTH - 1 - x, HEIGHT - 1 - y)] += leds[XY(x, y)]; // rotate to B
            leds[XY(x, HEIGHT - 1 - y)] += leds[XY(y, x)]; // rotate to C
        }
    }
}

// rotate, duplicate and copy over a triangle from first sector into the other half
// (crappy code)
void Caleidoscope5() {
    int halfWidth = WIDTH / 2;
    int halfWidthMinus1 = halfWidth - 1;

    int j = halfWidthMinus1;
    int k = 0;

    for (int i = 1; i < halfWidth; i++) {
        for (int x = i; x < halfWidth; x++) {
            leds[XY(halfWidthMinus1 - x, j)] += leds[XY(x, k)];
        }

        j--;
        k++;
    }

    //for (int x = 1; x < halfWidth; x++) {
    //    leds[XY(7 - x, 7)] += leds[XY(x, 0)];
    //} //a
    //for (int x = 2; x < halfWidth; x++) {
    //    leds[XY(7 - x, 6)] += leds[XY(x, 1)];
    //} //b
    //for (int x = 3; x < halfWidth; x++) {
    //    leds[XY(7 - x, 5)] += leds[XY(x, 2)];
    //} //c
    //for (int x = 4; x < halfWidth; x++) {
    //    leds[XY(7 - x, 4)] += leds[XY(x, 3)];
    //} //d
    //for (int x = 5; x < halfWidth; x++) {
    //    leds[XY(7 - x, 3)] += leds[XY(x, 4)];
    //} //e
    //for (int x = 6; x < halfWidth; x++) {
    //    leds[XY(7 - x, 2)] += leds[XY(x, 5)];
    //} //f
    //for (int x = 7; x < halfWidth; x++) {
    //    leds[XY(7 - x, 1)] += leds[XY(x, 6)];
    //} //g
}


void Caleidoscope6() {
    for (int x = 1; x < WIDTH / 2; x++) {
        leds[XY(7 - x, 7)] = leds[XY(x, 0)];
    } //a
    for (int x = 2; x < WIDTH / 2; x++) {
        leds[XY(7 - x, 6)] = leds[XY(x, 1)];
    } //b
    for (int x = 3; x < WIDTH / 2; x++) {
        leds[XY(7 - x, 5)] = leds[XY(x, 2)];
    } //c
    for (int x = 4; x < WIDTH / 2; x++) {
        leds[XY(7 - x, 4)] = leds[XY(x, 3)];
    } //d
    for (int x = 5; x < WIDTH / 2; x++) {
        leds[XY(7 - x, 3)] = leds[XY(x, 4)];
    } //e
    for (int x = 6; x < WIDTH / 2; x++) {
        leds[XY(7 - x, 2)] = leds[XY(x, 5)];
    } //f
    for (int x = 7; x < WIDTH / 2; x++) {
        leds[XY(7 - x, 1)] = leds[XY(x, 6)];
    } //g
}

// create a square twister
// x and y for center, r for radius
void SpiralStream(int x, int y, int r, byte dimm) {
    for (int d = r; d >= 0; d--) { // from the outside to the inside
        for (int i = x - d; i <= x + d; i++) {
            leds[XY(i, y - d)] += leds[XY(i + 1, y - d)]; // lowest row to the right
            leds[XY(i, y - d)].nscale8(dimm);
        }
        for (int i = y - d; i <= y + d; i++) {
            leds[XY(x + d, i)] += leds[XY(x + d, i + 1)]; // right colum up
            leds[XY(x + d, i)].nscale8(dimm);
        }
        for (int i = x + d; i >= x - d; i--) {
            leds[XY(i, y + d)] += leds[XY(i - 1, y + d)]; // upper row to the left
            leds[XY(i, y + d)].nscale8(dimm);
        }
        for (int i = y + d; i >= y - d; i--) {
            leds[XY(x - d, i)] += leds[XY(x - d, i - 1)]; // left colum down
            leds[XY(x - d, i)].nscale8(dimm);
        }
    }
}

// give it a linear tail to the side
void HorizontalStream(byte scale)
{
    for (int x = 1; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            leds[XY(x, y)] += leds[XY(x - 1, y)];
            leds[XY(x, y)].nscale8(scale);
        }
    }
    for (int y = 0; y < HEIGHT; y++)
        leds[XY(0, y)].nscale8(scale);
}

// give it a linear tail downwards
void VerticalStream(byte scale)
{
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 1; y < HEIGHT; y++) {
            leds[XY(x, y)] += leds[XY(x, y - 1)];
            leds[XY(x, y)].nscale8(scale);
        }
    }
    for (int x = 0; x < WIDTH; x++)
        leds[XY(x, 0)].nscale8(scale);
}

// just move everything one line down
void VerticalMove() {
    for (int y = HEIGHT - 1; y > 0; y--) {
        for (int x = 0; x < WIDTH; x++) {
            leds[XY(x, y)] = leds[XY(x, y - 1)];
        }
    }
}

// copy the rectangle defined with 2 points x0, y0, x1, y1
// to the rectangle beginning at x2, x3
void Copy(byte x0, byte y0, byte x1, byte y1, byte x2, byte y2) {
    for (int y = y0; y < y1 + 1; y++) {
        for (int x = x0; x < x1 + 1; x++) {
            leds[XY(x + x2 - x0, y + y2 - y0)] = leds[XY(x, y)];
        }
    }
}

// rotate + copy triangle (WIDTH / 2*WIDTH / 2)
void RotateTriangle() {
    for (int x = 1; x < WIDTH / 2; x++) {
        for (int y = 0; y < x; y++) {
            leds[XY(x, 7 - y)] = leds[XY(7 - x, y)];
        }
    }
}

// mirror + copy triangle (WIDTH / 2*WIDTH / 2)
void MirrorTriangle() {
    for (int x = 1; x < WIDTH / 2; x++) {
        for (int y = 0; y < x; y++) {
            leds[XY(7 - y, x)] = leds[XY(7 - x, y)];
        }
    }
}
// draw static rainbow triangle pattern (WIDTH / 2xWIDTH / 2)
// (just for debugging)
void RainbowTriangle() {
    for (int i = 0; i < WIDTH / 2; i++) {
        for (int j = 0; j <= i; j++) {
            Pixel(7 - i, j, i*j * 4);
        }
    }
}

/*
-------------------------------------------------------------------
Examples how to combine functions in order to create an effect

...or: how to visualize some of the following data
osci[0] ... osci[3] (0-255) triangle
p[0] ... p[3] (0-15) sinus
left[0] ... left[6] (0-1023) values of 63Hz, 160Hz, ...
right[0] ... right[6] (0-1023)

effects based only on oscillators (triangle/sine waves)

AutoRun shows everything that follows
SlowMandala red slow
Dots1 2 arround one
Dots2 stacking sines
SlowMandala2 just nice and soft
SlowMandala3 just nice and soft
Mandala8 copy one triangle all over

effects based on audio data (could be also linked to oscillators)

MSGEQtest colorfull 2 chanel 7 band analyzer
MSGEQtest2 2 frequencies linked to dot emitters in a spiral mandala
MSGEQtest3 analyzer 2 bars
MSGEQtest4 analyzer x 4 (as showed on youtube)
AudioSpiral basedrum/snare linked to red/green emitters
MSGEQtest5 one channel 7 band spectrum analyzer (spiral fadeout)
MSGEQtest6 classic analyzer, slow falldown
MSGEQtest7 spectrum mandala, color linked to low frequencies
MSGEQtest8 spectrum mandala, color linked to osci
MSGEQtest9 falling spectogram
CopyTest
Audio1
Audio2
Audio3
Audio4
CaleidoTest1
Caleidotest2
Audio5
Audio6
-------------------------------------------------------------------
*/

// all examples together
void AutoRun() {
    // all oscillator based:
    for (int i = 0; i < 300; i++) { Dots1(); }
    for (int i = 0; i < 300; i++) { Dots2(); }
    SlowMandala();
    SlowMandala2();
    SlowMandala3();
    for (int i = 0; i < 300; i++) { Mandala8(); }
    //// all MSGEQ7 based:
    //for (int i = 0; i < 500; i++) { MSGEQtest(); }
    //for (int i = 0; i < 500; i++) { MSGEQtest2(); }
    //for (int i = 0; i < 500; i++) { MSGEQtest3(); }
    //for (int i = 0; i < 500; i++) { MSGEQtest4(); }
    //for (int i = 0; i < 500; i++) { AudioSpiral(); }
    //for (int i = 0; i < 500; i++) { MSGEQtest5(); }
    //for (int i = 0; i < 500; i++) { MSGEQtest6(); }
    //for (int i = 0; i < 500; i++) { MSGEQtest7(); }
    //for (int i = 0; i < 500; i++) { MSGEQtest8(); }
    //for (int i = 0; i < 500; i++) { MSGEQtest9(); }
    //for (int i = 0; i < 500; i++) { CopyTest(); }
    //for (int i = 0; i < 500; i++) { Audio1(); }
    //for (int i = 0; i < 500; i++) { Audio2(); }
    //for (int i = 0; i < 500; i++) { Audio3(); }
    //for (int i = 0; i < 500; i++) { Audio4(); }
    //for (int i = 0; i < 500; i++) { CaleidoTest1(); }
    //for (int i = 0; i < 500; i++) { CaleidoTest2(); }
    //for (int i = 0; i < 500; i++) { Audio5(); }
    //for (int i = 0; i < 500; i++) { Audio6(); }
}

// red, 4 spirals, one dot emitter
// demonstrates SpiralStream and Caleidoscope
// (psychedelic)
void SlowMandala() {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            Pixel(i, j, 1);
            SpiralStream(8, 8, 8, 127);
            Caleidoscope1();
            ShowFrame();
            delay(50);
        }
    }
}

// 2 oscillators flying arround one ;)
void Dots1() {
    MoveOscillators();
    //2 lissajous dots red
    leds[XY(p[0], p[1])] = CHSV(1, 255, 255);
    leds[XY(p[2], p[3])] = CHSV(1, 255, 150);
    //average of the coordinates in yellow
    Pixel((p[2] + p[0]) / 2, (p[1] + p[3]) / 2, 50);
    ShowFrame();
    delay(20);
    HorizontalStream(125);
}

// x and y based on 3 sine waves
void Dots2() {
    MoveOscillators();
    Pixel((p[2] + p[0] + p[1]) / 3, (p[1] + p[3] + p[2]) / 3, osci[3]);
    ShowFrame();
    delay(20);
    HorizontalStream(125);
}

// beautifull but periodic
void SlowMandala2() {
    for (int i = 1; i < WIDTH / 2; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            MoveOscillators();
            Pixel(j, i, (osci[0] + osci[1]) / 2);
            SpiralStream(8, 8, 8, 127);
            Caleidoscope2();
            ShowFrame();
            delay(20);
        }
    }
}

// same with a different timing
void SlowMandala3() {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            MoveOscillators();
            Pixel(j, j, (osci[0] + osci[1]) / 2);
            SpiralStream(8, 8, 8, 127);
            Caleidoscope2();
            ShowFrame();
            delay(20);
        }
    }
}

// 2 lissajou dots *2 *4
void Mandala8() {
    MoveOscillators();
    Pixel(p[0] / 2, p[1] / 2, osci[2]);
    Pixel(p[2] / 2, p[3] / 2, osci[3]);
    Caleidoscope5();
    Caleidoscope2();
    HorizontalStream(110);
    ShowFrame();
}

// colorfull 2 chanel 7 band analyzer
void MSGEQtest() {
    ReadAudio();
    for (int i = 0; i < 7; i++) {
        Pixel(i, 16 - left[i] / 64, left[i] / 4);
    }
    for (int i = 0; i < 7; i++) {
        Pixel(8 + i, 16 - right[i] / 64, right[i] / 4);
    }
    ShowFrame();
    VerticalStream(120);
}

// 2 frequencies linked to dot emitters in a spiral mandala
void MSGEQtest2() {
    ReadAudio();
    if (left[0]>500) {
        Pixel(0, 0, 1);
        Pixel(1, 1, 1);
    }
    if (left[2] > 200) { Pixel(2, 2, 100); }
    if (left[6] > 200) { Pixel(5, 0, 200); }
    SpiralStream(4, 4, 4, 127);
    Caleidoscope1();
    ShowFrame();
}

// analyzer 2 bars
void MSGEQtest3() {
    ReadAudio();
    for (int i = 0; i < 8; i++) {
        Pixel(i, 16 - left[0] / 64, 1);
    }
    for (int i = 8; i < 16; i++) {
        Pixel(i, 16 - left[4] / 64, 100);
    }
    ShowFrame();
    VerticalStream(120);
}
// analyzer x 4 (as showed on youtube)
void MSGEQtest4() {
    ReadAudio();
    for (int i = 0; i < 7; i++) {
        Pixel(7 - i, 8 - right[i] / 128, i * 10);
    }
    Caleidoscope2();
    ShowFrame();
    DimAll(240);
}

// basedrum/snare linked to red/green emitters
void AudioSpiral() {
    MoveOscillators();
    SpiralStream(7, 7, 7, 130);
    SpiralStream(4, 4, 4, 122);
    SpiralStream(11, 11, 3, 122);
    ReadAudio();
    if (left[1] > 500) { leds[2, 1] = CHSV(1, 255, 255); }
    if (left[4] > 500) { leds[XY(random(15), random(15))] = CHSV(100, 255, 255); }
    ShowFrame();
    DimAll(250);
}

// one channel 7 band spectrum analyzer (spiral fadeout)
void MSGEQtest5() {
    ReadAudio();
    for (int i = 0; i < 7; i++) {
        Line(2 * i, 16 - left[i] / 64, 2 * i, 15, i * 10);
        Line(1 + 2 * i, 16 - left[i] / 64, 1 + 2 * i, 15, i * 10);
    }
    ShowFrame();
    SpiralStream(7, 7, 7, 120);
}

// classic analyzer, slow falldown
void MSGEQtest6() {
    ReadAudio();
    for (int i = 0; i < 7; i++) {
        Line(2 * i, 16 - left[i] / 64, 2 * i, 15, i * 10);
        Line(1 + 2 * i, 16 - left[i] / 64, 1 + 2 * i, 15, i * 10);
    }
    ShowFrame();
    VerticalStream(170);
}
// geile Schei�e
// spectrum mandala, color linked to 160Hz band
void MSGEQtest7() {
    MoveOscillators();
    ReadAudio();
    for (int i = 0; i < 7; i++) {
        Pixel(7 - i, 8 - right[i] / 128, i * 10 + right[1] / 8);
    }
    Caleidoscope5();
    Caleidoscope1();
    ShowFrame();
    DimAll(240);
}

// spectrum mandala, color linked to osci
void MSGEQtest8() {
    MoveOscillators();
    ReadAudio();
    for (int i = 0; i < 7; i++) {
        Pixel(7 - i, 8 - right[i] / 128, i * 10 + osci[1]);
    }
    Caleidoscope5();
    Caleidoscope2();
    ShowFrame();
    DimAll(240);
}

// falling spectogram
void MSGEQtest9() {
    ReadAudio();
    for (int i = 0; i < 7; i++) {
        leds[XY(i * 2, 0)] = CHSV(i * 27, 255, right[i] / 3); // brightness should be divided by 4
        leds[XY(1 + i * 2, 0)] = CHSV(i * 27, 255, left[i] / 3);
    }
    leds[XY(14, 0)] = { 0, 0, 0 };
    leds[XY(15, 0)] = { 0, 0, 0 };
    ShowFrame();
    VerticalMove();
}

// 9 analyzers
void CopyTest() {
    ReadAudio();
    for (int i = 0; i < 5; i++) {
        Line(i, 4 - left[i] / 256, i, 4, i * 10);
    }
    Copy(0, 0, 4, 4, 5, 0);
    Copy(0, 0, 4, 4, 10, 0);
    Copy(0, 0, 14, 4, 0, 5);
    Copy(0, 0, 14, 4, 0, 10);
    ShowFrame();
    DimAll(200);
}

// test scale
// NOT WORKING as intended YET!
void CopyTest2() {
    ReadAudio();
    for (int i = 0; i < 5; i++) {
        Line(i * 2, 4 - left[i] / 128, i * 2, 4, i * 10);
    }
    Scale(0, 0, 4, 4,
        7, 7, 15, 15);
    ShowFrame();
    DimAll(200);
}

// rechtangle 0-1 -> 2-3
// NOT WORKING as intended YET!
void Scale(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
    for (int y = y2; y < y3 + 1; y++) {
        for (int x = x2; x < x3 + 1; x++) {
            leds[XY(x, y)] = leds[XY(
                x0 + ((x * (x1 - x0)) / (x3 - x1)),
                y0 + ((y * (y1 - y0)) / (y3 - y1)))];
        }
    }
}

// line spectogram mandala
void Audio1() {
    ReadAudio();
    for (int i = 0; i < 5; i++) {
        Line(3 * i, 16 - left[i] / 64, 3 * (i + 1), 16 - left[i + 1] / 64, 255 - i * 15);
    }
    Caleidoscope4();
    ShowFrame();
    DimAll(10);
}

// line analyzer with stream
void Audio2() {
    ReadAudio();
    for (int i = 0; i < 5; i++) {
        Line(3 * i, 16 - left[i] / 64, 3 * (i + 1), 16 - left[i + 1] / 64, 255 - i * 15);
    }
    ShowFrame();
    HorizontalStream(120);
}

void Audio3() {
    ReadAudio();
    for (int i = 0; i < 7; i++) {
        leds[XY(6 - i, right[i] / 128)] = CHSV(i * 27, 255, right[i]);
    } // brightness should be divided by 4
    Caleidoscope6();
    Caleidoscope2();
    ShowFrame();
    DimAll(255);
}

void Audio4() {
    ReadAudio();
    for (int i = 0; i < 5; i++) {
        Line(3 * i, 8 - left[i] / 128, 3 * (i + 1), 8 - left[i + 1] / 128, i*left[i] / 32);
    }
    Caleidoscope4();
    ShowFrame();
    DimAll(12);
}

void CaleidoTest1() {
    ReadAudio();
    for (int i = 0; i < 7; i++) {
        Line(i, left[i] / 256, i, 0, left[i] / 32);
    }
    RotateTriangle();
    Caleidoscope2(); //copy + rotate
    ShowFrame();
    DimAll(240);
}

void CaleidoTest2() {
    MoveOscillators();
    ReadAudio();
    for (int i = 0; i < 7; i++) {
        Line(i, left[i] / 200, i, 0, (left[i] / 16) + 150);
    }
    MirrorTriangle();
    Caleidoscope1(); //mirror + rotate
    ShowFrame();
    DimAll(240);
}

void Audio5() {
    ReadAudio();
    for (int i = 0; i < 5; i++) {
        Line(
            3 * i, 8 - left[i] / 128, // from
            3 * (i + 1), 8 - left[i + 1] / 128, // to
            i * 30);
    } // color
    Caleidoscope4();
    ShowFrame();
    DimAll(9);
}

void Audio6() {
    ReadAudio();
    for (int i = 0; i < 5; i++) {
        Line(
            3 * i, 8 - left[i] / 128, // from
            3 * (i + 1), 8 - left[i + 1] / 128, // to
            i * 10); // lolor
        Line(
            15 - (3 * i), 7 + left[i] / 128, // from
            15 - (3 * (i + 1)), 7 + left[i + 1] / 128, // to
            i * 10); // color
    }
    ShowFrame();
    DimAll(200);
    //ClearAll();
}


/*
-------------------------------------------------------------------
Testcode for mapping the 16*16 calculation buffer to your
matrix size
-------------------------------------------------------------------
*/

// describe your matrix layout here:
// P.S. If you use not a 8*8 just remove the */ and /*
void RenderCustomMatrix() {
    /*
    for(int x = 0; x < CUSTOM_WIDTH; x++) {
    for(int y = 0; y < CUSTOM_HEIGHT; y++) {
    // position in the custom array
    leds2[x + x * y] =
    // positions(s) in the source 16*16
    // in this example it interpolates between just 2 diagonal touching pixels
    (leds[XY(x*2, y*2)] + // first point added to
    leds[XY(1+(x*2), 1+(y*2))]) // second point
    / 2; // divided by 2 to get the average color
    }
    }
    */
}

void ShowFrame() {
    // when using a matrix different than 16*16 use RenderCustomMatrix();
    //RenderCustomMatrix();
    matrix.swapBuffers();
    leds = matrix.backBuffer();
}

// HSV to RGB color conversion
// Input arguments
// hue in degrees (0 - 360.0)
// saturation (0.0 - 1.0)
// value (0.0 - 1.0)
// Output arguments
// red, green blue (0.0 - 1.0)
void hsvToRGB(float hue, float saturation, float value, float * red, float * green, float * blue) {

    int i;
    float f, p, q, t;

    if (saturation == 0) {
        // achromatic (grey)
        *red = *green = *blue = value;
        return;
    }
    hue /= 60;                  // sector 0 to 5
    i = floor(hue);
    f = hue - i;                // factorial part of h
    p = value * (1 - saturation);
    q = value * (1 - saturation * f);
    t = value * (1 - saturation * (1 - f));
    switch (i) {
        case 0:
            *red = value;
            *green = t;
            *blue = p;
            break;
        case 1:
            *red = q;
            *green = value;
            *blue = p;
            break;
        case 2:
            *red = p;
            *green = value;
            *blue = t;
            break;
        case 3:
            *red = p;
            *green = q;
            *blue = value;
            break;
        case 4:
            *red = t;
            *green = p;
            *blue = value;
            break;
        default:
            *red = value;
            *green = p;
            *blue = q;
            break;
    }
}

#define MAX_COLOR_VALUE     255

// Create a HSV color
rgb24 createHSVColor(float hue, float saturation, float value) {

    float r, g, b;
    rgb24 color;

    hsvToRGB(hue, saturation, value, &r, &g, &b);

    color.red = r * MAX_COLOR_VALUE;
    color.green = g * MAX_COLOR_VALUE;
    color.blue = b * MAX_COLOR_VALUE;

    return color;
}

rgb24 CHSV(int _h, int _s, int _v) {
    int h = map(_h, 0, 255, 0, 360);
    float s = (float) map(_s, 0, 255, 0, 1000) / 1000.0;
    float v = (float) map(_v, 0, 255, 0, 1000) / 1000.0;

    return createHSVColor(h, s, v);
}