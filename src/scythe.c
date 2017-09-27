/*
Copyright (c) 2018 Radoslav Gerganov <rgerganov@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <hidapi.h>
#include "common.h"
#include "debug.h"

hid_device *dev = NULL;

const unsigned char KEY_DATA[13] = {0x06, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00,
                                    0x06, 0x00, 0x00, 0x00, 0xff};
const unsigned char MOUSE_DATA[12] = {0x06, 0x00, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00,
                                      0x06, 0x00, 0x00, 0xff};

typedef struct pedal_data
{
    unsigned char data[16];
    int data_len;
} pedal_data;

pedal_data pedals[3];
int curr_pedal = 1;

void usage()
{
    fprintf(stderr, "Usage: scythe [-123] [-r] [-a <key>] [-m <modifier>] [-b <button>]\n"
        "   -r          - read all pedals\n"
        "   -1          - program the first pedal\n"
        "   -2          - program the second pedal (default)\n"
        "   -3          - program the third pedal\n"
        "   -a key      - append the specified key\n"
        "   -m modifier - ctrl|shift|alt|win\n"
        "   -b button   - mouse_left|mouse_middle|mouse_right|mouse_double\n\n"
        "You cannot mix -a and -m options with -b option for one and the same pedal\n");
    exit(1);
}

void init()
{
    hid_init();
    dev = hid_open(0x0426, 0x3011, NULL);
