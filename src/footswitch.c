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
#include <errno.h>
#include <unistd.h>
#include <hidapi.h>
#include "common.h"
#include "debug.h"

hid_device *dev = NULL;

typedef struct pedal_data
{
    unsigned char header[8];
    unsigned char data[48];
    int data_len;
} pedal_data;

typedef struct pedal_protocol
{
    unsigned char start[8];
    pedal_data pedals[3];
} pedal_protocol;

pedal_protocol pd = {{0}};
pedal_data *curr_pedal = &pd.pedals[1]; // start at the second pedal

// KEY and MOUSE types can be combined
#define KEY_TYPE    1
#define MOUSE_TYPE    2
// STRING must be by itself
#define STRING_TYPE    4

void usage() {
    fprintf(stderr, "Usage: footswitch [-123] [-r] [-s <string>] [-S <raw_string>] [-ak <key>] [-m <modifier>] [-b <button>] [-xyw <XYW>]\n"
        "   -r          - read all pedals\n"
        "   -1          - program the first pedal\n"
        "   -2          - program the second pedal (default)\n"
        "   -3          - program the third pedal\n"
        "   -s string   - append the specified string\n"
        "   -S rstring  - append the specified raw string (hex numbers delimited with spaces)\n"
        "   -a key      - append the specified key\n"
        "   -k key      - write the specified key\n"
        "   -m modifier - (l_,r_)ctrl|shift|alt|win\n"
        "   -b button   - mouse_left|mouse_middle|mouse_right\n"
        "   -x X        - move the mouse cursor horizontally by X pixels\n"
        "   -y Y        - move the mouse cursor vertically by Y pixels\n"
        "   -w W        - move the mouse wheel by W\n\n"
        "You cannot mix -sSa options with -kmbxyw options for one and the same pedal\n");
    exit(1);
}

void init_pid(unsigned short vid, unsigned short pid) {
    struct hid_device_info *info = NULL, *ptr = NULL;
    hid_init();
    // Config protocol is on interface 1. Interface 0 is the keyboard, which
    // macOS won't open, so we can't just hid_open() by vid/pid here.
    info = hid_enumerate(vid, pid);
    ptr = info;
    while (ptr != NULL) {
        if (ptr->interface_number == 1) {
            dev = hid_open_path(ptr->path);
            break;
        }
        ptr = ptr->next;
    }
    hid_free_enumeration(info);
#ifdef OSX
    // Older hidapi (<0.14) doesn't report interface_number on macOS, so the
    // loop finds nothing -- fall back to opening by vid/pid.
    if (dev == NULL) {
        dev = hid_open(vid, pid, NULL);
    }
#endif
}

void init() {
    unsigned short vid_pid[][2] = {
        {0x0c45, 0x7403},
        {0x0c45, 0x7404},
        {0x413d, 0x2107},
        {0x1a86, 0xe026},
        {0x3553, 0xb001},
    };
    int i = 0;
    for (i = 0 ; i < sizeof(vid_pid) / sizeof(vid_pid[0]) ; i++) {
        init_pid(vid_pid[i][0], vid_pid[i][1]);
        if (dev != NULL) {
            break;
        }
    }
    if (dev == NULL) {
        fatal("Cannot find footswitch with one of the supported VID:PID.\nCheck that the device is connected and that you have the correct permissions to access it.");
    }
}

void init_pedal(pedal_data *p, int num) {
    unsigned char default_header[8] = {0x01, 0x81, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char default_data[8] = {0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    memcpy(p->header, default_header, 8);
    p->header[3] = num + 1;

    memset(p->data, 0, sizeof(p->data));
    memcpy(p->data, default_data, 8);

    p->data_len = 8;
}

void init_pedals() {
    unsigned char start[8] = {0x01, 0x80, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};

    memcpy(pd.start, start, 8);
    init_pedal(&pd.pedals[0], 0);
    init_pedal(&pd.pedals[1], 1);
    init_pedal(&pd.pedals[2], 2);
}
