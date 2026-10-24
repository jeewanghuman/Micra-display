// Shell for the Waveshare ESP32-P4-WIFI6-Touch-LCD-3.4C (round, 800x800).
//
// Two printed parts plus 4 x M2.5x6 screws:
//   SHELL - a round cup. The board drops in from the back and its glass rests
//           on a thin front lip that overlaps the glass's black border (the
//           display itself is 87.6 mm across; the glass is 115 mm, so a 2.5 mm
//           lip never covers the picture or the touch area).
//   COVER - a plug that closes the back, held by 4 radial M2.5 screws through
//           the shell wall. Its flat back carries the SAME mounting interface
//           as Apollo 2's shells: 2 x M4 holes, 75 mm apart, so the
//           tondo-magnet-mount.3mf (Micra top corner) and tondo-stand.3mf
//           (counter-top) bolt straight on with 2 x M4x8.
//
// A "chin" slot at the bottom of the wall clears the USB-C plug(s) and lets
// the paddle cable out.
//
// MEASURE BEFORE PRINTING. Waveshare publishes only the 115 mm outline; the
// numbers marked MEASURE below are best guesses. Print `part = "gauge"` first
// (a 4 mm slice, ~10 min) to check the glass fit, then set the rest.
//
// Export:  openscad -D 'part="shell"' -o shell.stl esp32-p4-3.4c-shell.scad
//          (part = "shell" | "cover" | "gauge" | "assembly")

part = "assembly";

/* [Board - MEASURE] */
glass_d     = 115.0;  // glass diameter (Waveshare: 115 x 115 mm outline)
body_depth  = 14.0;   // MEASURE: front of the glass to the deepest point at the back
                      //   (connectors, header pins, SD card included)
usb_angle   = 270;    // MEASURE: where the USB-C port(s) face, degrees, 270 = bottom
usb_from_front = 9.0; // MEASURE: front of the glass to the CENTRE of the USB-C port
chin_w      = 34;     // chin slot width: fits both USB-C plugs + the paddle cable
chin_h      = 9;      // chin slot height (a USB-C overmould is ~6.5 mm)

/* [Fit] */
fit   = 0.5;   // diametral clearance around the glass (0.25 mm a side)
wall  = 3.0;   // shell wall
lip   = 2.5;   // how far the front lip overlaps the glass edge
lip_t = 1.6;   // front lip thickness
plug_t = 7.0;  // cover thickness (depth it sits inside the shell)
plug_fit = 0.3;  // diametral clearance cover-in-shell
back_pocket = 3.0; // cover recess for the board's backside parts (0 = flat)

/* [Mounting - same as Apollo 2's shells] */
mount_spacing = 75;   // tondo-magnet-mount / -stand hole spacing
m4_pilot   = 3.7;     // printed pilot for an M4 screw threading into plastic
m4_depth   = 6.5;
m25_clear  = 2.9;     // M2.5 clearance in the shell wall
m25_pilot  = 2.3;     // M2.5 pilot in the cover rim (self-threading)

$fn = 160;

bore    = glass_d + fit;
od      = bore + 2 * wall;
shell_h = lip_t + body_depth + plug_t;   // cover sits flush with the back edge
plug_d  = bore - plug_fit;
screw_z = shell_h - plug_t / 2;          // radial screws through the cover rim
screw_angles = [45, 135, 225, 315];      // clear of the chin at 270

module shell() {
  difference() {
    cylinder(d = od, h = shell_h);
    // glass + body bore, from the lip back
    translate([0, 0, lip_t]) cylinder(d = bore, h = shell_h);
    // window through the lip
    translate([0, 0, -1]) cylinder(d = glass_d - 2 * lip, h = lip_t + 2);
    // soften the front edge
    translate([0, 0, -0.01]) difference() {
      cylinder(d = od + 2, h = 1.2);
      cylinder(d1 = od - 2.4, d2 = od + 0.01, h = 1.21);
    }
    // chin: USB-C + cable exit, open to the back edge so the board (cables
    // already plugged in) slides in from behind
    rotate([0, 0, usb_angle])
      translate([od / 2 - wall - 1, -chin_w / 2, lip_t + usb_from_front - chin_h / 2])
        cube([wall + 2, chin_w, shell_h]);
    // radial cover screws
    for (a = screw_angles)
      rotate([0, 0, a]) translate([0, 0, screw_z]) rotate([0, 90, 0])
        cylinder(d = m25_clear, h = od);
    // countersink-ish head seats so the screw heads sit flush-ish
    for (a = screw_angles)
      rotate([0, 0, a]) translate([od / 2 - 0.8, 0, screw_z]) rotate([0, 90, 0])
        cylinder(d = 5.2, h = 2);
  }
}

// The cover in its INSTALLED orientation: z = 0 is the inside face (against
// the board), z = plug_t the outside face that bolts to the mount. "cover"
// export flips it outside-face-down for printing (a real rotation, so the
// TOP mark and the chin keep their places).
module cover_installed() {
  difference() {
    cylinder(d = plug_d, h = plug_t);
    // pocket for the board's backside parts (inside face)
    if (back_pocket > 0)
      translate([0, 0, -1]) cylinder(d = plug_d - 2 * 6, h = back_pocket + 1);
    // the chin continues through the cover rim
    rotate([0, 0, usb_angle])
      translate([plug_d / 2 - 8, -chin_w / 2, -1]) cube([10, chin_w, plug_t + 2]);
    // radial M2.5 pilots (the shell-wall screws bite here), in the solid rim
    for (a = screw_angles)
      rotate([0, 0, a]) translate([plug_d / 2 - 9, 0, screw_z - (shell_h - plug_t)])
        rotate([0, 90, 0]) cylinder(d = m25_pilot, h = 10);
    // vents: a ring of slots so the P4 doesn't cook in a closed cup
    for (i = [0 : 11])
      rotate([0, 0, i * 30 + 15])
        translate([22, -1.2, -1]) cube([14, 2.4, plug_t + 2]);
    // "TOP" mark opposite the chin, debossed on the outside face
    rotate([0, 0, usb_angle + 180]) translate([44, 0, plug_t - 0.6])
      rotate([0, 0, -90]) linear_extrude(1)
        text("TOP", size = 5, halign = "center", valign = "center");
  }
  // M4 mounting bosses, 75 mm apart, horizontal (perpendicular to the chin)
  for (s = [-1, 1])
    rotate([0, 0, usb_angle + 90]) translate([s * mount_spacing / 2, 0, 0])
      difference() {
        cylinder(d = 10, h = plug_t);
        translate([0, 0, plug_t - m4_depth]) cylinder(d = m4_pilot, h = m4_depth + 0.01);
        translate([0, 0, plug_t - 0.6]) cylinder(d1 = m4_pilot, d2 = m4_pilot + 1.2, h = 0.61);
      }
}

module cover() {  // print orientation: outside (mounting) face down
  rotate([0, 180, 0]) translate([0, 0, -plug_t]) cover_installed();
}

// A 4 mm slice of the shell's front (lip + first few mm of bore): print it,
// press the glass in from the back, and adjust `fit` before the full print.
module gauge() {
  intersection() {
    shell();
    cylinder(d = od + 1, h = lip_t + 2.5);
  }
}

module assembly() {
  color("DimGray") shell();
  color("Silver", 0.9) translate([0, 0, shell_h - plug_t]) cover_installed();
}

if (part == "shell") shell();
else if (part == "cover") cover();
else if (part == "gauge") gauge();
else assembly();
