Arduino cat feeder which is supposed to open the lid, let you select a time, close the lid, run a countdown, then open lid again.
This was writte entirely with gpt and this code sucks.
I'm having issues with this, not even sure if hardware or software.

components:
arduino nano
9g servo
encoder knob with press function
0,9" i²c oled

pinout:
gnd:    shared ground
5v:     shared 5v power
a4, a5: oled i²c
d2:     encoder knob up
d3:     encoder knob down
d4:      encoder knob press
d5:     9g servo
