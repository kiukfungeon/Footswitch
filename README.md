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
