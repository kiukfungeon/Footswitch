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

// BXKBSettingLib.dll + 0x1540
void UpdateSetting(uint8_t *data, int len)
{
    uint8_t buff[0x48] = {0};
    buff[3] = 0x2c;         // local_45
    buff[6] = 0x02;         // local_42
    SetUpdateEx(buff, 0x48);
    for (int offset = 0; offset < len; offset += 0x20) {
        int count = len - offset;
        if (count > 0x20) {
            count = 0x20;
        }
        buff[3] = 0x26;             // local_45
        buff[4] = offset >> 8;       // local_44
        buff[5] = offset;            // local_43
        buff[6] = count;            // local_42
        for (int i = 0; i < count; i++) {
            buff[8+i] = data[offset+i];
        }
        SetUpdateEx(buff, 0x48);
        SetUpdateEx(buff, 0x48);
    }
    buff[3] = 0x2b;             // local_45
    buff[4] = 0x14;             // local_44
    buff[5] = 0x23;             // local_43
    buff[6] = 0x00;             // local_42
    SetUpdateEx(buff, 0x48);
}

static bool set_pedal_type(enum event_type new_type)
{
    if (pedals[curr_pedal].type == NONE) {
        pedals[curr_pedal].type = new_type;
        return true;
    }
    return false;
}

void compile_string(const char *str)
{
    if (!set_pedal_type(MULTIPLE_KEYS)) {
        fprintf(stderr, "Invalid combination of options\n");
        usage();
    }
    int len = strlen(str);
    if (len > MAX_KEYS) {
        fprintf(stderr, "The string length exceeds %d\n", MAX_KEYS);
        exit(1);
    }
    pedals[curr_pedal].count = len;
    for (int i = 0; i < strlen(str); i++) {
        pedals[curr_pedal].keys[i].mod = 0xf0;
        uint8_t code = 0;
        encode_char(str[i], &code);
        pedals[curr_pedal].keys[i].code = code;
    }
}

void compile_key_norepeat(const char *key)
{
    if (!set_pedal_type(SINGLE_KEY_NOREPEAT)) {
        fprintf(stderr, "Invalid combination of options\n");
        usage();
    }
    uint8_t code = 0;
    if (!encode_key(key, &code)) {
        fprintf(stderr, "Cannot encode key '%s'\n", key);
        exit(1);
    }
    pedals[curr_pedal].count = 1;
    pedals[curr_pedal].keys[0].mod |= 0xf0;
    pedals[curr_pedal].keys[0].code = code;
}

void compile_key_repeat(const char *key)
{
    if (!set_pedal_type(SINGLE_KEY_REPEAT)) {
        fprintf(stderr, "Invalid combination of options\n");
        usage();
    }
    uint8_t code = 0;
    if (!encode_key(key, &code)) {
        fprintf(stderr, "Cannot encode key '%s'\n", key);
        exit(1);
    }
    pedals[curr_pedal].count = 1;
    pedals[curr_pedal].keys[0].mod |= 0xf0;
    pedals[curr_pedal].keys[0].code = code;
}

void compile_modifier(const char *mod_str)
{
    enum modifier mod;

    if (!parse_modifier(mod_str, &mod)) {
        fprintf(stderr, "Invalid modifier '%s'\n", mod_str);
        exit(1);
    }
    pedals[curr_pedal].keys[0].mod |= mod;
}
