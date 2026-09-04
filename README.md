# excess

Bench stuff that never got its own repo. none of it is finished, some of it
doesnt build without a blob i cant commit, and nothing in here has anything to
do with anything else in here beyond living on the same desk

morphcpu is the one thats actually done, its elsewhere

## csi-presence

esp32-s3 that works out if theres a person in the room by watching how they
wreck the wifi channel. no camera no pir. the radio is already measuring the
room every packet, csi just hands you the per subcarrier estimate instead of
throwing it away

it connects as a station, spams udp at the gateway so theres traffic coming back
at something like 70hz, and takes the l1 distance of each frames amplitudes off
a slow baseline. rolling stdev of that distance is the score, two thresholds
with a hold so it doesnt chatter. `motion.c` is 70 lines and easier to read than
this paragraph

first version scored it as correlation against the empty room vector instead.
that worked, it also fired every time the AP shifted rate so i threw it out

the thresholds in there are whatever worked with the board on my shelf, theyre
not a constant of the universe. `tools/cap.py` dumps raw frames over uart, flip
`print_raw` in `csi_rx.c` first or you get nothing out of it, and `plot_csi.py`
runs the same maths offline so you can move the window without reflashing.
thats how id retune it somewhere else

things that ate time. the csi buffer is imag then real, not real then imag.
power save has to be off or the radio sleeps between beacons and you get
nothing. `manu_scale` makes the trace look much steadier and also makes
amplitudes incomparable across frames so its off. the dc bin and the guard
subcarriers are zeroed cuz leaving them in just adds variance that isnt a person

breathing at rest is supposedly in there somewhere, the fft for it is commented
out at the bottom of the plot script, i didnt get anything out of it

built against idf 5.x for the s3, the csi config struct is different on c6

## mouse

paused. it started as wanting to know how much of a mouse you can remove before
it stops being one

rp2040, pmw3360 on spi0, five switches straight on gpio with no matrix, tinyusb
boot mouse. `pmw3360.c` is the only interesting file and its interesting for
boring reasons, the sensor needs its srom blob uploaded in one long cs low
before it reports anything and that blob is pixarts so its gitignored, pull
`srom_0x04.h` out of any qmk tree. the timings in there are the datasheet ones,
tSRAD 160us and tSWW 180us, and shaving them made writes go missing

sensor sits rotated 90 in the shell so x and y are swapped in the report. theres
a comment saying dont fix it, thats why. `pmw_init` failing doesnt stop boot
either, it comes up dead maybe half the time on a cold plug and a dead sensor is
still a working set of buttons

`shell/shell.scad` is the top, a hex grid subtracted out of everything that
isnt a switch rail or the spine or the sensor boss. keepouts are the whole
design. 7mm flat to flat with a 1.4 web, thinner web than that and it tore off
the bed. wall is 0.8, 0.6 printed fine and flexed under a click. hollowing it
with minkowski took 40 minutes and came out looking the same as the scaled
copy its doing now. no bottom plate, that lines commented out at the end

no weights in here, i never wrote them down anywhere that survived

## tcbfp

pong written in brainfuck, tyler cowens head is the ball

`pong.bf` prints an ascii frame and stops at a `,` waiting for a byte, the page
runs it in a little interpreter that hands back whatever got printed each time
it hits one and draws that on a canvas. up and down are the paddle. the counter
at the bottom is time since the emergent ventures rejection landed

https://ranveerlabs.github.io/excess/tcbfp/
