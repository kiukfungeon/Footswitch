/*
Copyright (c) 2012 Radoslav Gerganov <rgerganov@gmail.com>
Copyright (c) 2012 Daniel Manjarres <danmanj@gmail.com>

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
#include <inttypes.h>
#include <unistd.h>
#include <hidapi.h>
#include "common.h"
#include "debug.h"

hid_device *dev = NULL;

#define PEDAL_PIN_3_P15 0x03

enum pedal_report_ids {
    REPORT_DEVICE_ID   = 0x22,
    REPORT_SET_CODE    = 0x10,
};

typedef struct pedal_data {
    union {
        struct {
            unsigned char report_id;
            unsigned char pin;
            unsigned char command;
            unsigned char size;
            unsigned char data[8];
        };
        unsigned char buffer[64];
    };
} pedal_data_t;

pedal_data_t pd = { 0 };

void usage() {
    fprintf(stderr, "Usage: footswitch1p [-r] [-k <key>] [-m <modifier>] [-b <button>] [-xyw <XYW>]\n"
        "   -r          - read all pedals\n"
        "   -k key      - write the specified key\n"
        "   -m modifier - (l_,r_)ctrl|shift|alt|win\n"
        "   -b button   - mouse_left|mouse_middle|mouse_right\n"
        "   -x X        - move the mouse cursor horizontally by X pixels\n"
        "   -y Y        - move the mouse cursor vertically by Y pixels\n"
        "   -w W        - move the mouse wheel by W\n\n"
        "You cannot mix -km options with -bxyw options.\n");
    exit(1);
}

void init_pid(unsigned short vid, unsigned short pid) {
#ifdef OSX
    hid_init();
    dev = hid_open(vid, pid, NULL);
#else
    struct hid_device_info *info = NULL, *ptr = NULL;
    hid_init();
    info = hid_enumerate(vid, pid);
    ptr = info;
    while (ptr != NULL) {
        if (ptr->interface_number == 3) {
            dev = hid_open_path(ptr->path);
            break;
        }
        ptr = ptr->next;
    }
    hid_free_enumeration(info);
#endif
}

void init() {
    static unsigned short vid_pid[][2] = {
        {0x5131, 0x2019},
    };

    for (size_t i = 0 ; i < sizeof(vid_pid) / sizeof(vid_pid[0]) ; i++) {
