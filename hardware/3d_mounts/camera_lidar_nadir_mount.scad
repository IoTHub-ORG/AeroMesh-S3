// AeroMesh S3 - Nadir Camera & VL53L0X LiDAR Mount
// Material: PLA / PETG / TPU
// Platform: 36x36mm Mark4 Bottom Plate Mount

$fn = 60;

plate_width = 36;
plate_length = 36;
plate_thickness = 3;
m3_hole_r = 1.6;

difference() {
    cube([plate_width, plate_length, plate_thickness], center = true);

    translate([15, 15, 0]) cylinder(h = plate_thickness +2, r = m3_hole_r, center = true);
    translate([-15, 15, 0]) cylinder(h = plate_thickness +2, r = m3_hole_r, center = true);
    translate([15, -15, 0]) cylinder(h = plate_thickness + 2, r = m3_hole_r, center = true);
    translate([-15, -15, 0]) cylinder(h = plate_thickness + 2, r = m3_hole_r, center = true);

    cylinder(h = plate_thickness +2, r = 5.0, center = true);

    translate([0, 10, 0]) cube([8, 12, plate_thickness +2], center = true);


}