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
    if (dev == NULL) {
        fatal("Cannot find Scythe pedal with VID:PID=0426:3011.\nCheck that a Scythe device is connected and that you have the correct permissions to access it.");
    }
}

void deinit()
{
    hid_close(dev);
    hid_exit();
}

void print_mouse(unsigned char data[])
{
    switch (data[1]) {
        case 0x81:
            printf("mouse_left");
            break;
        case 0x82:
            printf("mouse_right");
            break;
        case 0x84:
            printf("mouse_middle");
            break;
        case 0x80:
            printf("mouse_double");
            break;
    }
}

void print_key(unsigned char data[])
{
    char combo[128] = {0};
    int ind = 3;
    if ((data[1] & CTRL) != 0) {
        strcat(combo, "ctrl+");
    }
    if ((data[1] & SHIFT) != 0) {
        strcat(combo, "shift+");
    }
    if ((data[1] & ALT) != 0) {
        strcat(combo, "alt+");
    }
    if ((data[1] & WIN) != 0) {
        strcat(combo, "win+");
    }
    while (data[ind] != 0 && ind <= 7) {
        const char *key = decode_byte(data[ind]);
        strcat(combo, key);
        strcat(combo, "+");
        ind++;
    }
    size_t len = strlen(combo);
    if (len > 0) {
        combo[len - 1] = 0; // remove the last +
    }
    printf("%s", combo);
}

void read_pedals()
{
    int r = 0, i = 0;
    unsigned char query[8] = {0x06, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char response[20];

    for (i = 0 ; i < 3 ; i++) {
        query[0] = 6;
        query[1] = 0xbb;
        query[2] = i + 1;
        r = hid_send_feature_report(dev, query, 8);
        if (r < 0) {
            fatal("error sending feature report (%ls)", hid_error(dev));
        }
        r = hid_get_feature_report(dev, response, 8);
        if (r < 0) {
            fatal("error getting feature report (%ls)", hid_error(dev));
        }
        //debug_arr(response, 8);

        printf("[switch %d]: ", i + 1);
        if ((response[1] >= 0x80 && response[1] <= 0x82) || response[1] == 0x84) {
            print_mouse(response);
        } else if (response[1] == 0xff) {
            printf("undefined");
        } else {
            print_key(response);
        }
        printf("\n");
    }
}

void compile_key_repeat(const char *key)
{
    unsigned char b = 0;
    int i;
    int indarr[5] = {6, 7, 9, 10, 11};

    if (pedals[curr_pedal].data_len == 12) {
        fprintf(stderr, "Invalid combination of options\n");
        usage();
    }
    if (pedals[curr_pedal].data_len == 0) {
        memcpy(pedals[curr_pedal].data, KEY_DATA, 13);
        pedals[curr_pedal].data_len = 13;
    }
    pedals[curr_pedal].data[1] = curr_pedal + 1;

    if (!encode_key(key, &b)) {
        fprintf(stderr, "Cannot encode key '%s'\n", key);
        exit(1);
    }
    for (i = 0 ; i < 5 ; i++) {
        int ind = indarr[i];
        if (pedals[curr_pedal].data[ind] == 0) {
            pedals[curr_pedal].data[ind] = b;
            return;
        }
    }
    fprintf(stderr, "Cannot write more than 5 keys\n");
    exit(1);
}

void compile_modifier(const char *mod_str)
{
    enum modifier mod;

    if (pedals[curr_pedal].data_len == 12) {
        fprintf(stderr, "Invalid combination of options\n");
        usage();
    }
    if (pedals[curr_pedal].data_len == 0) {
        memcpy(pedals[curr_pedal].data, KEY_DATA, 13);
        pedals[curr_pedal].data_len = 13;
    }
    pedals[curr_pedal].data[1] = curr_pedal + 1;

    if (!parse_modifier(mod_str, &mod)) {
        fprintf(stderr, "Invlalid modifier '%s'\n", mod_str);
        exit(1);
    }

    pedals[curr_pedal].data[4] |= mod;
}

void compile_mouse_button(const char *btn_str)
{
    enum mouse_button btn;

    if (!parse_mouse_button(btn_str, &btn)) {
        fprintf(stderr, "Invalid mouse button '%s'\n", btn_str);
        exit(1);
    }
    if (pedals[curr_pedal].data_len == 13) {
        fprintf(stderr, "Invalid combination of options\n");
        usage();
    }
    if (pedals[curr_pedal].data_len == 0) {
        memcpy(pedals[curr_pedal].data, MOUSE_DATA, 12);
        pedals[curr_pedal].data_len = 12;
    }
    pedals[curr_pedal].data[1] = curr_pedal + 1;
    switch (btn) {
        case MOUSE_LEFT:
            pedals[curr_pedal].data[4] = 0x81;
            break;
        case MOUSE_RIGHT:
            pedals[curr_pedal].data[4] = 0x82;
            break;
        case MOUSE_MIDDLE:
            pedals[curr_pedal].data[4] = 0x84;
            break;
        case MOUSE_DOUBLE:
            pedals[curr_pedal].data[4] = 0x80;
            break;
    }
}

void send_report(unsigned char *ptr, int len)
{
    int r;
    //debug_arr(ptr, len);
    r = hid_send_feature_report(dev, ptr, len);
    if (r < 0) {
        fprintf(stderr, "Error sending feature report\n");
    }
    usleep(200 * 1000);
}

void write_pedals() {
    int i = 0;
    unsigned char nop[8] = {0x06, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00};
    unsigned char end[8] = {0x06, 0xaa, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00};

