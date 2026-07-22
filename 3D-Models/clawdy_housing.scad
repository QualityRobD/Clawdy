// =========================================================================
// CLAW-D MASCOT: INTERNAL ELECTRONICS SLED
// =========================================================================

/* [OLED Display Specifications] */
oled_w = 27.8; // Pocket width for the dual OLED screens (old: 27.8)
oled_h = 28.6; // Pocket height for the dual OLED screens (old: 28.0 & 28.02)

/* [ESP32 Cradle Specifications] */
esp_l               = 23.3; // Length of the ESP32 cradle walls
esp_w               = 18.2; // Distance between cradle walls (old: 18mm)
esp_h               = 12.0; // Height of the cradle walls securing the board
esp_z_lift          = 5.0;  // Raises the board to create clearance for bottom-mounted components
cradle_floor_h      = 4.0;  // Z-height of the lower level floor (below the board ledge)
esp_y_offset        = 0;    // Shift board position: Negative = closer to front, Positive = further back
esp_clearance_w     = 14.0; // Width of the underside component clearance cavity (auto-centers)
crade_roof_overhang = 20;   // Length of the top retaining roof (original 14.5)

/* [Main Sled Dimensions] */
sled_width = 63.0;  // Total overall width of the structural sled

/* [Roof Slit Parameters] */
roof_slit_w = 2.0; // Width of the wiring slit in the top retaining roof
roof_slit_l = 30;  // Length of the slit (extends forward from the back interior wall)

/* [Coax Antenna Clearance Parameters] */
coax_w = 5;    // Width of the coax connector cutout
coax_l = 14;   // Length of the cutout (extends the valley towards the button holes)
coax_h = 15.1; // Depth/Height of the cutout (measured upward from the cradle floor)

/* [Asymmetrical Depth Adjustments] */
// The front is restored to its original position (-16.5)
// The back is extended by exactly 1mm from original (16.5 -> 17.5) to fix fitment
front_y    = -16.5; 
back_y     = 17.5;   
sled_depth = back_y - front_y; // Dynamically calculates to 34.0mm total depth

/* [Structural Walls & Placement] */
sled_height      = 65.0;  // Total height of the sled unit
wall_thick       = 2.0;   // Standard thickness for outer side/back walls
front_wall_thick = 4.0;   // Restored to 4.0mm to provide correct OLED resting clearance
eye_z_offset     = 25.12; // Vertical Z-position mapping for the OLED eye pockets

/* [Button Hole Alignment (BOOT / RESET)] */
button_hole_d              = 1.5;  // Diameter of the access holes (sized for a paperclip)
left_button_hole_x_offset  = 4.25; // X-axis offset from left inner cradle wall
right_button_hole_x_offset = 5.0;  // X-axis offset from right inner cradle wall
button_hole_y = back_y - 22.25;    // Y-axis alignment (dynamically tracks with back_y adjustments)

// =========================================================================
// MAIN EXECUTION
// =========================================================================

// Orient for 3D printing (Rotates the sled so the front OLED face lies flat on the build plate)
translate([0, 0, -front_y]) 
    rotate([90, 0, 0]) 
        render_sled();

// =========================================================================
// SLED MODULE
// =========================================================================

module render_sled() {
    difference() {
        
        // --- ADDITIONS (POSITIVE SPACE) ---
        union() {
            // Main U-Shape Sled Outer Body
            difference() {
                // Solid outer bounding box
                translate([-sled_width/2, front_y, 0])
                    cube([sled_width, sled_depth, sled_height]);
                
                // Hollow out the center to create the "U" shape (leaves walls intact)
                translate([-(sled_width - wall_thick*2)/2, front_y + front_wall_thick, wall_thick])
                    cube([sled_width - wall_thick*2, sled_depth, sled_height]);
            }
            
            // ESP32 Cradle Base (Solid block extended to front wall, raised by esp_z_lift)
            translate([-(esp_w + 2)/2, front_y + front_wall_thick, wall_thick])
                cube([esp_w + 2, back_y - (front_y + front_wall_thick), esp_h + esp_z_lift]);

            // ESP32 Cradle Top Retaining Roof (Shelters the board and secures it down)
            translate([-(esp_w + 2)/2, front_y + front_wall_thick, wall_thick + 12 + esp_z_lift])
                cube([esp_w + 2, ((back_y - esp_l + esp_y_offset) - (front_y + front_wall_thick)) + crade_roof_overhang, 2]);
                
            // 0.5mm Friction Tolerance Rails
            // (Placed on the outer sides to allow sanding for a perfect press-fit without reprinting)
            translate([-sled_width/2 - 0.7, front_y, 0])
                cube([0.7, sled_depth, sled_height]);
            translate([sled_width/2, front_y, 0])
                cube([0.7, sled_depth, sled_height]);
        }
        
        // --- SUBTRACTIONS (NEGATIVE SPACE) ---
        
        // 1. ESP32 Drop-In Seat 
        // Lifted by esp_z_lift so the board rests precisely on a 2mm bottom ledge
        translate([-esp_w/2, back_y - esp_l + esp_y_offset, wall_thick + esp_z_lift + 0.1])
            cube([esp_w, esp_l + 1, esp_h]);

        // 2. ESP32 Underside Component Clearance (For USB port, bottom ICs, and Buttons)
        // This gap leaves rails on both sides for the PCB to sit on.
        depth = back_y - 16 + esp_y_offset;
        clearance_h = (wall_thick + esp_z_lift + 2) - cradle_floor_h; // Auto-calculates height to meet the step cleanly
        translate([-esp_clearance_w/2, depth - 7.30, cradle_floor_h])
            cube([esp_clearance_w, 25, clearance_h]);

        // 3. Coax Antenna Cutout
        // Anchored directly to cradle_floor_h so it only cuts UPward and leaves the floor intact
        translate([-coax_w/2, depth - coax_l, cradle_floor_h])
            cube([coax_w, coax_l + 0.1, coax_h]);
            
        // 4. Top Roof Slit 
        // Cuts a wire-routing channel through the roof starting from the interior back wall
        interior_back_y = back_y + esp_y_offset;
        translate([-roof_slit_w/2, interior_back_y - roof_slit_l, wall_thick + 12 + esp_z_lift - 1])
            cube([roof_slit_w, roof_slit_l + 1, 4]); // Height is 4mm to ensure it fully pierces the 2mm shelf

        // 5. Left OLED Pocket (Cut into the front wall)
        translate([-sled_width/2 + wall_thick + 0.5, front_y - 0.1, eye_z_offset])
            cube([oled_w, front_wall_thick + 1, oled_h]);
            
        // 6. Right OLED Pocket (Cut into the front wall)
        translate([sled_width/2 - wall_thick - 0.5 - oled_w, front_y - 0.1, eye_z_offset])
            cube([oled_w, front_wall_thick + 1, oled_h]);
            
        // 7. BUTTON HOLES (BOOT / RESET)
        // Left Hole (Boot) - Offset inward from the left inner cradle wall
        translate([-esp_w/2 + left_button_hole_x_offset, button_hole_y, -1])
            cylinder(d=button_hole_d, h=esp_z_lift + 2, $fn=30); // Note: Changed h parameter from esp_z_lift to h=esp_z_lift+2 to ensure clean cut
            
        // Right Hole (Reset) - Offset inward from the right inner cradle wall
        translate([esp_w/2 - right_button_hole_x_offset, button_hole_y, -1])
            cylinder(d=button_hole_d, h=esp_z_lift + 2, $fn=30); // Note: Changed h parameter from esp_z_lift to h=esp_z_lift+2 to ensure clean cut
    }
}
