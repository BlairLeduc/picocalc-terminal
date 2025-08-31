# picocalc-terminal

A serial terminal for the PicoCalc.

# Features

- Connect via USB-C or GPIO pins (3.3V only!)
- Supports different baud rates
- Fixed to 8 data bits, 1 stop bit, no parity
- VT100 terminal emulation

# Usage

If you are connecting to a Unix/Linux system, you need to
configure your environment to work correctly with this
terminal app.

In bash:

``` bash
> stty columns 64 rows 32
> export LINES=32
> export COLUMNS=64
> export TERM=vt100
```

