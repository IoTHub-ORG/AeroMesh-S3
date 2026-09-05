// AeroMesh S3 - 35mm Elevated TPU Landing Skid
// Material: TPU (Flexible 95A)
// Mount: Mark4 Motor Arm Extrusion

$fn = 60;

skid_height = 35;
arm_width = 16;
arm_length = 20;
wall_thickness = 3;

difference() {
    // Outer landing foot body
    cube([arm_width, arm_length, skid_height], center = false);

    // Motor arm slot
    translate([wall_thickness, -1, skid_height - 10])
        cube([arm_width - (2 * wall_thickness), arm_length + 2, 11], center = false);

    // Zip-tie fastening channel
    translate([-1, arm_length / 2, skid_height - 5])
        rotate([0, 90, 0])
        cylinder(h = arm_width + 2, r = 1.5, center = false);
}