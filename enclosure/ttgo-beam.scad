// -------------------------------------------------
// Settings
// -------------------------------------------------

$fn = 50;

// -------------------------------------------------
// Configuration
// -------------------------------------------------

device_width = 100.0;
device_depth = 36.0;
device_height = 34.0;
wall_thickness = 3.0;
board_thickness = 2.1;
device_front_depth = 16.0;
rack_depth = 3.0;
corner_radius = 8.0;
screw_length = 12.0;
screw_head = 2.5;
screw_offset = 3.5; // was 2.5
display_left = 12.5;
display_bottom = 12.5;
display_width = 27.0;
display_inner_width = 17.0;
display_height = 15.0;
display_guide = 4.0;
antenna_radius = 4.0;
antenna_y = -7;
xamfra = 2;

// -------------------------------------------------

board_y = -device_depth/2 + device_front_depth - board_thickness / 2;
box_width = device_width + wall_thickness;
box_depth = device_depth + wall_thickness;
box_height = device_height + wall_thickness;

// -------------------------------------------------
// Modules
// -------------------------------------------------

module base() {
    linear_extrude(wall_thickness) {
        difference() {
            minkowski() {
                square([box_width-2*xamfra, box_depth-2*xamfra], true);
                circle(xamfra, true);
            }
            translate([8, board_y-3]) square([40,10], true);
        }
    }
}

module lid() {
    difference() {
        linear_extrude(wall_thickness-1) {
            difference() {
                square([box_width-wall_thickness/2-0.5,box_depth-wall_thickness/2-0.5], true);
                translate([0, board_y + antenna_y]) circle(antenna_radius, true);
                translate([box_width/2-screw_offset, box_depth/2-screw_offset ]) circle(screw_head/2,true);
                translate([box_width/2-screw_offset, -box_depth/2+screw_offset ]) circle(screw_head/2,true);
                translate([-box_width/2+screw_offset, box_depth/2-screw_offset ]) circle(screw_head/2,true);
                translate([-box_width/2+screw_offset, -box_depth/2+screw_offset ]) circle(screw_head/2,true);
            }
        }
        translate([0,0,1]) linear_extrude(10) {
                translate([box_width/2-screw_offset, box_depth/2-screw_offset ]) circle(screw_head,true);
                translate([box_width/2-screw_offset, -box_depth/2+screw_offset ]) circle(screw_head,true);
                translate([-box_width/2+screw_offset, box_depth/2-screw_offset ]) circle(screw_head,true);
                translate([-box_width/2+screw_offset, -box_depth/2+screw_offset ]) circle(screw_head,true);

        }
    }
}

module walls() {
    difference() {
        union() {
            linear_extrude(box_height + wall_thickness - 1) {
                difference() {
                    minkowski() {
                        square([box_width+wall_thickness/2-2*xamfra,box_depth+wall_thickness/2-2*xamfra], true);
                        circle(xamfra, true);
                    }
                    square([box_width-wall_thickness/2,box_depth-wall_thickness/2], true);
                }
            }
            translate([
                -box_width/2+wall_thickness/2+display_left+display_width/2,
                -(box_depth-wall_thickness)/2+display_guide
            ]) linear_extrude(display_bottom) {
                difference() {
                    square([display_width, 2], true);
                    square([display_inner_width, 2], true);
                }
            }
        }
        translate([
            -box_width/2+wall_thickness/2+display_left+display_width/2,
            -(box_depth-wall_thickness)/2,
            display_bottom+display_height/2+wall_thickness
        ]) display();
    }
}

module rack() {
    linear_extrude(box_height) {
        translate([0,board_y+board_thickness/2]) 
            union() {
                translate([(box_width-wall_thickness)/2,
                    (board_thickness+rack_depth)/2]) {
                    square([rack_depth/2,rack_depth], true);
                }
                translate([(box_width-wall_thickness)/2,
                    -(board_thickness+rack_depth)/2]) {
                    square([rack_depth/2,rack_depth], true);
                }
                translate([-(box_width-wall_thickness)/2,
                    (board_thickness+rack_depth)/2]) {
                    square([rack_depth/2,rack_depth], true);
                }
                translate([-(box_width-wall_thickness)/2,
                    -(board_thickness+rack_depth)/2]) {
                    square([rack_depth/2,rack_depth], true);
                }
            }
    }
}

module corners() {
    union() {
        translate([-box_width/2,-box_depth/2,box_height]) corner();
        translate([box_width/2,-box_depth/2,box_height]) rotate([0,0,90]) corner();
        translate([box_width/2,box_depth/2,box_height]) rotate([0,0,180]) corner();
        translate([-box_width/2,box_depth/2,box_height]) rotate([0,0,270]) corner();
    }
}

module corner() {
    difference() {
        intersection() {
            sphere(corner_radius, true);
            translate([0,0,-corner_radius]) cube(corner_radius);
        }
        translate([screw_offset,screw_offset,-screw_length]) linear_extrude(screw_length) {
            circle([screw_head/2],true);
        }
    }
}

module display() {
    rotate([90,0,0])
        linear_extrude(wall_thickness)
            square([display_width, display_height], true);
}

module device() {
    rotate([90,0,0]) linear_extrude(1.6) square([device_width, device_height], true);
}

// -------------------------------------------------
// Entry points
// -------------------------------------------------

module full() {
    union() {
        base();
        walls();
        rack();
        corners();
    }
}

full();
//translate([0,0,box_height + 10]) lid();
translate([0,box_depth + 10,0]) lid();
//translate([0,board_y+wall_thickness/2,wall_thickness + device_height/2]) device();
