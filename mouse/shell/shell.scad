// top shell. hex cut most of it away, whats left is the rails the
// switches sit on and a spine down the middle

$fn = 48;

L  = 118;
W  = 62;
H  = 38;
T  = 0.8;      // wall. 0.6 printed fine and flexed under a click
HEX = 7;       // flat to flat
GAP = 1.4;     // web between holes, thinner than this and it tears off the bed

sensor_x = 60;  // from the front, roughly under where the palm ends up
pcb_z = 6;

module body() {
    hull() {
        translate([0, 0, 2]) resize([L, W, 1]) cylinder(h = 1, d = 100);
        translate([L*0.42, 0, H-14]) resize([L*0.72, W*0.86, 1]) sphere(d = 60);
    }
}

module hexgrid(l, w, h) {
    dx = HEX + GAP;
    dy = (HEX + GAP) * sin(60);
    for (j = [-w/dy : w/dy])
        for (i = [-l/dx : l/dx])
            translate([i*dx + (j % 2 ? dx/2 : 0), j*dy, 0])
                cylinder(h = h, d = HEX / cos(30), $fn = 6);
}

module keepouts() {
    // switch pads, cant have holes under these
    translate([L*0.30, -W*0.22, H-6]) cube([26, 16, 20], center = true);
    translate([L*0.30,  W*0.22, H-6]) cube([26, 16, 20], center = true);
    // spine
    translate([0, 0, H/2]) cube([L, 9, H], center = true);
    // sensor boss
    translate([sensor_x - L/2, 0, 0]) cylinder(h = 12, d = 22);
    // front lip, the seam is here and it needs the material
    translate([-L/2 + 4, 0, H/2]) cube([8, W, H], center = true);
}

difference() {
    difference() { body(); offset_shell(); }
    // holes everywhere except the keepouts
    difference() {
        translate([0, 0, -5]) rotate([0, 0, 0]) hexgrid(L, W, H + 20);
        keepouts();
    }
}

module offset_shell() {
    // hollow. minkowski was 40 minutes and looked the same as this
    scale([(L - 2*T)/L, (W - 2*T)/W, 1]) translate([0, 0, -1]) body();
}

// bottom plate, never printed it
// translate([0, 0, -6]) linear_extrude(1.2) offset(r = -T) projection() body();
