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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "common.h"

typedef struct keymap_entry
{
    const char *name;
    unsigned char value;
} keymap_entry;

// see http://www.freebsddiary.org/APC/usb_hid_usages.php
static const keymap_entry keymap[] =
{
    {"<00>",        0x00},
    {"<01>",        0x01},
    {"<02>",        0x02},
    {"<03>",        0x03},
    {"a",           0x04},
    {"b",           0x05},
    {"c",           0x06},
    {"d",           0x07},
    {"e",           0x08},
    {"f",           0x09},
    {"g",           0x0a},
    {"h",           0x0b},
    {"i",           0x0c},
    {"j",           0x0d},
    {"k",           0x0e},
    {"l",           0x0f},
    {"m",           0x10},
    {"n",           0x11},
    {"o",           0x12},
    {"p",           0x13},
    {"q",           0x14},
    {"r",           0x15},
    {"s",           0x16},
    {"t",           0x17},
    {"u",           0x18},
    {"v",           0x19},
    {"w",           0x1a},
    {"x",           0x1b},
    {"y",           0x1c},
    {"z",           0x1d},
    {"1",           0x1e},
    {"2",           0x1f},
    {"3",           0x20},
    {"4",           0x21},
    {"5",           0x22},
    {"6",           0x23},
    {"7",           0x24},
    {"8",           0x25},
    {"9",           0x26},
    {"0",           0x27},
    {"enter",       0x28},
    {"Return",      0x28},
    {"esc",         0x29},
    {"Escape",      0x29},
    {"backspace",   0x2a},
    {"tab",         0x2b},
    {" ",           0x2c},
    {"space",       0x2c},
    {"-",           0x2d},
    {"=",           0x2e},
    {"[",           0x2f},
    {"]",           0x30},
    {"\\",          0x31},
    {"\\",          0x32}, // yes a repeat
    {";",           0x33},
    {"\'",          0x34},
    {"`",           0x35},
    {",",           0x36},
    {".",           0x37},
    {"/",           0x38},
    {"capslock",    0x39},
    {"f1",          0x3a},
    {"f2",          0x3b},
    {"f3",          0x3c},
    {"f4",          0x3d},
    {"f5",          0x3e},
    {"f6",          0x3f},
    {"f7",          0x40},
    {"f8",          0x41},
    {"f9",          0x42},
    {"f10",         0x43},
    {"f11",         0x44},
    {"f12",         0x45},
    {"f13",         0x68},
    {"f14",         0x69},
    {"f15",         0x6A},
    {"f16",         0x6B},
    {"f17",         0x6C},
    {"f18",         0x6D},
    {"f19",         0x6E},
    {"f20",         0x6F},
    {"f21",         0x70},
    {"f22",         0x71},
    {"f23",         0x72},
    {"f24",         0x73},
    {"printscreen", 0x46},
    {"scrollock",   0x47},
    {"pause",       0x48},
    {"insert",      0x49},
    {"home",        0x4a},
    {"pageup",      0x4b},
    {"Prior",       0x4b},
    {"delete",      0x4c},
    {"end",         0x4d},
    {"pagedown",    0x4e},
    {"Next",        0x4e},
    {"right",       0x4f},
    {"down",        0x51},
    {"left",        0x50},
    {"up",          0x52},
    {"numlock",     0x53},
