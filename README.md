Feuille64 GRID Firmware
===================================

This is the firmware to work Feuille64 as a monome grid (https://monome.org/docs/grid/) clone.

This will work with the [node-serialoscd](https://github.com/szymonkaliski/node-serialoscd).   
It does not work with the official serialosc.

work in progress

## How to build 
AVR development environment is required. If you already installed QMK Firmware, you should be able to build this too.

```
(clone this repo. )

git submodule update --init --recursive

make
```

## License
MIT license (c) 2021, Takuya Urakawa (hsgw)

Many parts of this are based on QMK Firmware.
