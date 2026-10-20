#!/usr/bin/env python3
"""Virtual pen tablet (uinput stylus with pressure) for testing pen pressure under XWayland.
The compositor sees it as a real tablet, so XWayland gets an "xwayland-tablet stylus" device,
which Wine's X11 driver reads (patch 0075). Coordinates are screen pixels; the screen size comes
from the X root window, or from PEN_SCREEN=WxH. Needs write access to /dev/uinput.
  tools/pen.py stroke X1 Y1 X2 Y2 [STEPS]   pen down, pressure 0 -> max -> 0 along the line, pen up
  tools/pen.py ramp X1 Y1 X2 Y2 [STEPS]     pressure rises 0 -> max along the line (light to heavy)
  tools/pen.py flat X1 Y1 X2 Y2 [STEPS] [P] constant pressure P (0..1, default 0.8)
  tools/pen.py hover X Y                    move the pen in proximity without touching
  tools/pen.py hold SECONDS                 keep the device alive (for xinput list / test-xi2)
Several commands can be chained with ' ; ' inside one argument string."""
import sys, time
from evdev import UInput, ecodes as e, AbsInfo

def screen_size():
    import os
    if os.environ.get('PEN_SCREEN'):
        return tuple(int(v) for v in os.environ['PEN_SCREEN'].lower().split('x'))
    try:
        from Xlib import display
        root = display.Display().screen().root.get_geometry()
        return root.width, root.height
    except Exception:
        return 2880, 1800


W, H = screen_size()
SCALE = 10                 # tablet units per screen pixel
PMAX = 8191                # pressure range of a Wacom pen
cap = {
    e.EV_KEY: [e.BTN_TOOL_PEN, e.BTN_TOUCH, e.BTN_STYLUS, e.BTN_STYLUS2],
    e.EV_ABS: [(e.ABS_X, AbsInfo(0, 0, W * SCALE - 1, 0, 0, 100)),   # resolution: units per mm
               (e.ABS_Y, AbsInfo(0, 0, H * SCALE - 1, 0, 0, 100)),
               (e.ABS_PRESSURE, AbsInfo(0, 0, PMAX, 0, 0, 0)),
               (e.ABS_TILT_X, AbsInfo(0, -64, 63, 0, 0, 57)),
               (e.ABS_TILT_Y, AbsInfo(0, -64, 63, 0, 0, 57))],
}
ui = UInput(cap, name='adobewine-virtual-pen stylus', vendor=0x056a, product=0x0357,  # Wacom vendor id
            input_props=[e.INPUT_PROP_DIRECT])
time.sleep(1.0)  # let the compositor pick the device up


def syn(): ui.syn(); time.sleep(0.008)
def pos(x, y):
    ui.write(e.EV_ABS, e.ABS_X, int(float(x) * SCALE)); ui.write(e.EV_ABS, e.ABS_Y, int(float(y) * SCALE))
def prox(on):
    ui.write(e.EV_KEY, e.BTN_TOOL_PEN, 1 if on else 0)


def line(x1, y1, x2, y2, steps, pressure_of):
    x1, y1, x2, y2 = map(float, (x1, y1, x2, y2))
    pos(x1, y1); prox(1); syn(); time.sleep(0.05)
    for i in range(steps + 1):
        t = i / steps
        pos(x1 + (x2 - x1) * t, y1 + (y2 - y1) * t)
        p = int(PMAX * pressure_of(t))
        ui.write(e.EV_ABS, e.ABS_PRESSURE, p)
        ui.write(e.EV_KEY, e.BTN_TOUCH, 1 if p > 0 or 0 < i < steps else 0)
        syn()
    ui.write(e.EV_ABS, e.ABS_PRESSURE, 0); ui.write(e.EV_KEY, e.BTN_TOUCH, 0); syn()
    time.sleep(0.05); prox(0); syn()


def run(cmd):
    a = cmd.split()
    op = a[0]
    if op in ('stroke', 'ramp', 'flat'):
        steps = int(a[5]) if len(a) > 5 else 120
        level = float(a[6]) if len(a) > 6 else 0.8
        curve = {'stroke': lambda t: 1 - abs(2 * t - 1), 'ramp': lambda t: t, 'flat': lambda t: level}[op]
        line(a[1], a[2], a[3], a[4], steps, lambda t: max(0.02, curve(t)))
    elif op == 'hover':
        pos(a[1], a[2]); prox(1); syn(); time.sleep(0.3); prox(0); syn()
    elif op == 'hold':
        time.sleep(float(a[1]))
    else:
        sys.exit('unknown command: ' + op)
    time.sleep(0.15)


for c in ' '.join(sys.argv[1:]).split(' ; '):
    if c.strip():
        run(c.strip())
ui.close()
