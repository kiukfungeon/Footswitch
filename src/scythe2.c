#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <hidapi.h>
#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "debug.h"

#define MAX_KEYS 255

hid_device *dev = NULL;

enum event_type {
    NONE = 0,
    SINGLE_KEY_REPEAT = 0x10,
    SINGLE_KEY_NOREPEAT = 0x20,
    MULTIPLE_KEYS = 0x30,
} event_type;

typedef struct key_data {
    uint8_t mod;
    uint8_t code;
} key_data;

typedef struct pedal_data
{
    enum event_type type;
    int count;
    key_data keys[MAX_KEYS];
} pedal_data;

pedal_data pedals[6];
int curr_pedal = 0;

void usage()
{
    fprintf(stderr, "Usage: scythe2 [-123456] [-r] [-k <key>] [-a <key>] [-m <modifier>] [-b <button>]\n"
        "   -r          - read all pedals\n"
        "   -1          - program the first pedal\n"
        "   -2          - program the second pedal (default)\n"
        "   -3          - program the third pedal\n"
        "   -4          - program the fourth pedal\n"
        "   -5          - program the fifth pedal\n"
        "   -6          - program the sixth pedal\n"
        "   -s string   - append the specified string\n"
        "   -a key      - write the specified key (no repeat)\n"
        "   -k key      - write the specified key (repeat)\n"
        "   -m modifier - ctrl|shift|alt|win\n"
        "   -b button   - mouse_left|mouse_middle|mouse_right|mouse_double\n");
    exit(1);
}

void init()
{
    hid_init();
    dev = hid_open(0x055a, 0x0998, NULL);
    if (dev == NULL) {
        fatal("Cannot find Scythe pedal with VID:PID=055a:0998.\nCheck that a Scythe device is connected and that you have the correct permissions to access it.");
    }
}

void deinit()
{
    hid_close(dev);
    hid_exit();
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

// BXKBSettingLib.dll + 0x10B0
void checksum(uint8_t *data, int len)
{
    uint8_t sum = 0;
    data[7] = 0;
    for (int i = 0; i < len; i++) {
        sum += data[i];
    }
    data[7] = sum;
}

// BXKBSettingLib.dll + 0x10E0
void SetUpdateEx(uint8_t *data, int len)
{
    data[0] = 0x05;
    data[1] = 0x96;
    data[2] = 0xa5;
    checksum(data, len);
    send_report(data, len);
}
