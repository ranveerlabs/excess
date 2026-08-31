// scratch. pmw3360 module footprint so i could check the lens clears the
// hex cut, the lens is the tall bit not the pcb
module pmw_mod() {
    color("green") cube([21, 21, 1.6], center = true);
    translate([0, 0, -3.5]) color("gray") cube([12, 9, 6], center = true);  // lens
}
pmw_mod();
