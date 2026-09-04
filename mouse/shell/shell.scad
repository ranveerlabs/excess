$fn = 48;

L  = 118;
W  = 62;
H  = 38;
T  = 0.8;
HEX = 7;
GAP = 1.4;

sensor_x = 60;
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
    translate([L*0.30, -W*0.22, H-6]) cube([26, 16, 20], center = true);
    translate([L*0.30,  W*0.22, H-6]) cube([26, 16, 20], center = true);
    translate([0, 0, H/2]) cube([L, 9, H], center = true);
    translate([sensor_x - L/2, 0, 0]) cylinder(h = 12, d = 22);
    translate([-L/2 + 4, 0, H/2]) cube([8, W, H], center = true);
}

difference() {
    difference() { body(); offset_shell(); }
    difference() {
        translate([0, 0, -5]) rotate([0, 0, 0]) hexgrid(L, W, H + 20);
        keepouts();
    }
}

module offset_shell() {
    scale([(L - 2*T)/L, (W - 2*T)/W, 1]) translate([0, 0, -1]) body();
}

// translate([0, 0, -6]) linear_extrude(1.2) offset(r = -T) projection() body();
