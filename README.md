[![Build Status](https://github.com/rgerganov/footswitch/workflows/CI/badge.svg)](https://github.com/rgerganov/footswitch/actions)

Footswitch
----------

Command line utlities for programming [PCsensor][1] and [Scythe](https://www.scythe-eu.com/en/products/pc-accessory/usb-foot-switch-ii.html) foot switches.

The following list of devices are supported:

| vendorId | productId | Program       |
| ---------|---------- | ------------- |
| 0c45     | 7403      | `footswitch`  |
| 0c45     | 7404      | `footswitch`  |
| 413d     | 2107      | `footswitch`  |
| 1a86     | e026      | `footswitch`  |
| 3553     | b001      | `footswitch`  |
| 0426     | 3011      | `scythe`      |
| 055a     | 0998      | `scythe2`     |
| 5131     | 2019      | `footswitch1p`|

You can find the `vendorId` and `productId` of your device using the `lsusb` command on Linux.

The same kind of foot switches are used for building the popular [VIM Clutch][2].

Building
--------

Debian
------

On a debian machine, provided you have `devscripts` and `equivs` installed:

```bash
mk-build-deps -i
dpkg-buildpackage -us -uc -b
```

Or, you can build the package in a Docker container:

```bash
docker build .
```
Archlinux
---------

Footswitch is available in the [AUR](https://aur.archlinux.org/packages/footswitch-git) and can be installed using your favourite AUR helper, for example:

- `yay -S footswitch-git`
- `paru -S footswitch-git`


Other systems
-------------

The programs are using the [hidapi][3] library and should work on Linux and OSX. To build on Linux:

    sudo apt-get install libhidapi-dev
    git clone https://github.com/rgerganov/footswitch.git
    cd footswitch
    make
    sudo make install

The `install` target installs udev rules on Linux which allow running the programs without root.
You may need to use `sudo` otherwise.

To build on OSX:

    brew tap rgerganov/footswitch https://github.com/rgerganov/footswitch.git
    brew install --HEAD footswitch

Usage
-----
    footswitch [-123] [-r] [-s <string>] [-S <raw_string>] [-ak <key>] [-m <modifier>] [-b <button>] [-xyw <XYW>]
       -r          - read all pedals
       -1          - program the first pedal
       -2          - program the second pedal (default)
       -3          - program the third pedal
       -s string   - append the specified string
       -S rstring  - append the specified raw string (hex numbers delimited with spaces)
       -a key      - append the specified key
       -k key      - write the specified key
       -m modifier - ctrl|shift|alt|win
       -b button   - mouse_left|mouse_middle|mouse_right
       -x X        - move the mouse cursor horizontally by X pixels
       -y Y        - move the mouse cursor vertically by Y pixels
       -w W        - move the mouse wheel by W

    You cannot mix -sSa options with -kmbxyw options for one and the same pedal
_

    scythe [-123] [-r] [-a <key>] [-m <modifier>] [-b <button>]
       -r          - read all pedals
       -1          - program the first pedal
       -2          - program the second pedal (default)
       -3          - program the third pedal
       -a key      - append the specified key
       -m modifier - ctrl|shift|alt|win
       -b button   - mouse_left|mouse_double|mouse_right

    You cannot mix -a and -m options with -b option for one and the same pedal

Examples for PCsensor
--------
    footswitch -r
        read the persisted function in each pedal and print it on the console
    footswitch -k a
        program the second pedal to print the letter 'a' (also work for single pedal devices);
        as a general rule you don't need to specify -1, -2 or -3 if you have only one pedal
    footswitch -1 -k a -2 -k b -3 -k c
        program the first pedal to print 'a', second pedal to print 'b' and third pedal to print 'c'
    footswitch -1 -k esc -2 -k enter
        program the first pedal as Escape key and the second pedal as Enter key
    footswitch -1 -m ctrl -k a -3 -m alt -k f4
        program the first pedal as Ctrl+a and the third pedal as Alt+F4
    footswitch -1 -m ctrl -k a -3 -m shift -m alt -k s
        program the first pedal as Ctrl+a and the third pedal as Shift+Alt+s (Wikipedia "save")
    footswitch -m ctrl -b mouse_middle
        program the second pedal as Ctrl+<middle_mouse_click>
    footswitch -s 'hello world'
        program the second pedal to print 'hello world'
    footswitch -s 'hello' -s ' ' -s 'world'
        this will also program the second pedal to print 'hello world';
        you can specify multiple -s options and each option will append to the resulting string
