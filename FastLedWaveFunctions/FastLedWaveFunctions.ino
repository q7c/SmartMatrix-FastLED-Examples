// LED Wave Functions
// Modified by Jason Coon for Smart Matrix:
// 
// Funky Clouds 2014
// Toys For Matrix Effects
// www.stefan-petrick.de/wordpress_beta
// http://pastebin.com/5fmCZs3B
// https://www.youtube.com/watch?v=s2U99Zu-kYw

#include "SmartMatrix.h"

#define HAS_IR_REMOTE 1

#if (HAS_IR_REMOTE == 1)

#include "IRremote.h"

#define IR_RECV_CS     18

// IR Raw Key Codes for SparkFun remote
#define IRCODE_HOME  0x10EFD827   

IRrecv irReceiver(IR_RECV_CS);

#endif

SmartMatrix matrix;

const int DEFAULT_BRIGHTNESS = 255;

const rgb24 COLOR_BLACK = { 0, 0, 0 };

bool isOff = false;

#define FRAMES_PER_SECOND 60

// Matrix Size

const uint8_t WIDTH = 32;
const uint8_t HEIGHT = 32;

// LED Setup

#define NUM_LEDS (WIDTH * HEIGHT)

rgb24 *leds;

byte count;

rgb24 color0 = CHSV(0, 255, 255);
rgb24 color1 = CHSV(40, 255, 255);
rgb24 color2 = CHSV(80, 255, 255);
rgb24 color3 = CHSV(120, 255, 255);
rgb24 color4 = CHSV(160, 255, 255);

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

void setup() {
    // Setup serial interface
    Serial.begin(9600);

    // Initialize 32x32 LED Matrix
    matrix.begin();
    matrix.setBrightness(DEFAULT_BRIGHTNESS);
    matrix.setColorCorrection(cc24);

#if (HAS_IR_REMOTE == 1)

    // Initialize IR receiver
    irReceiver.enableIRIn();

#endif

    // Clear screen
    matrix.fillScreen(COLOR_BLACK);
    matrix.swapBuffers();

    randomSeed(analogRead(5));
}

// scale the brightness of the screenbuffer down
void DimmAll(byte value)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i].nscale8(value);
    }
}

// finds the right index for our matrix
int XY(int x, int y) {
    if (y > HEIGHT) { y = HEIGHT; }
    if (y < 0) { y = 0; }
    if (x > WIDTH) { x = WIDTH; }
    if (x < 0) { x = 0; }

    return (y * WIDTH) + x;
}

void loop()
{
#if (HAS_IR_REMOTE == 1)
    handleInput();

    if (isOff) {
        delay(100);
        return;
    }
#endif

    leds = matrix.backBuffer();

    count++;
    if (count == 255) count = 0;

    leds[XY(map(sin8(count), 0, 255, 0, 31), 0)] = color0;
    leds[XY(map(cos8(count), 0, 255, 0, 31), 2)] = color1;
    leds[XY(map(quadwave8(count), 0, 255, 0, 31), 4)] = color2;
    leds[XY(map(cubicwave8(count), 0, 255, 0, 31), 6)] = color3;
    leds[XY(map(triwave8(count), 0, 255, 0, 31), 8)] = color4;

    DimmAll(250);

    matrix.swapBuffers();

    // delay(1000 / FRAMES_PER_SECOND);
}

#if (HAS_IR_REMOTE == 1)

unsigned long handleInput() {
    unsigned long input = 0;

    decode_results results;

    results.value = 0;

    // Attempt to read an IR code ?
    if (irReceiver.decode(&results)) {
        input = results.value;

        // Prepare to receive the next IR code
        irReceiver.resume();
    }

    if (input == IRCODE_HOME) {
        isOff = !isOff;

        if (isOff){
            matrix.fillScreen(COLOR_BLACK);
            matrix.swapBuffers();
        }

        return input;
    }

    return input;
}

#endif