// =========================================================================
// CLAW-D MASCOT: LARGE EDITION (SLED READY, REAR SCREW MOUNTS)
// =========================================================================

/* [Fixed Hardware Specifications] */
pcb_w = 27.3;         // Physical width of standard 0.96" PCB
glass_t = 2.4;        // Front of glass to back of PCB
clearance = 0.4;      // 3D printing tolerance fit
$fn = 24;             // Curve smoothness

/* [Adjustable Features] */
print_mode = "arms";  // [ "assembled", "base", "cap", "legs", "arms" ]
eye_w = 25;           // Width of the front eye cutouts - original: 23
eye_h = 12;           // Height of the front eye cutouts - original: 14
eye_mm_height_adjust = 5;
eye_chamfer = 2.0;    // How much the outer edges slant outward for better viewing angle
top_softness = 1.0;   // How much the outer top edge is chamfered (leaves the rest flat for supports)

usb_hole_height_adjuster = 5.0;

// Variables to protect the internal cavity while adding pin holes
bottom_thickness_added = 2.4; 
pin_height = 3.0;
hole_depth = 3.6; 

clawd_chassis();

module clawd_chassis() {
    
    // 1. HARDCODED LARGE PARAMETERS
    unit = 7.2;
    head_w = 86.4;    // 12 * 7.2
    head_h = 69.12;   // 86.4 / 1.25
    head_d = 39.6;    // 5.5 * 7.2

    if (print_mode == "base" || print_mode == "cap" || print_mode == "assembled") {
        difference() {
            // 2. THE ACCURATE PROPORTIONAL OUTER SHELL
            union() {
                // Main Head (Bottom extended downward by bottom_thickness_added)
                translate([-head_w/2, -head_d/2, 2*unit - bottom_thickness_added]) 
                    cube([head_w, head_d, head_h + bottom_thickness_added]);
                
                if (print_mode == "assembled") {
                    // Left Side Stub/Arm
                    translate([-head_w/2 - 2*unit, -unit/2, head_h/2 + unit]) 
                        cube([2*unit, unit, 2*unit]);
                    
                    // Right Side Stub/Arm
                    translate([head_w/2, -unit/2, head_h/2 + unit]) 
                        cube([2*unit, unit, 2*unit]);
                        
                    // ---- STABLE EIGHT LEG SQUID STRUCTURE ----
                    for(x = [-4.5*unit, -2.5*unit, 1.5*unit, 3.5*unit]) {
                        for(y = [-head_d/2, head_d/2 - unit]) {
                            // Legs end where the newly lowered chassis begins
                            translate([x, y, 0]) cube([unit, unit, 2*unit - bottom_thickness_added]);
                        }
                    }
                }
            }
            
            // 3. INTERNAL CAVITY (Untouched coordinates)
            translate([-4.5*unit, -head_d/2 + 2.5, 2*unit + 2])
                cube([9*unit, head_d - 5, head_h - 3.8]);
                
            // 4. FRONT FACE EYE WINDOWS (Two-Stage Cut for Supports + Soft Edge)
            translate([-2.5*unit - eye_w/2.75, -head_d/2 - 0.1, head_h - 2.25*unit - eye_h/2 + eye_mm_height_adjust])
                chamfered_eye(is_left = true);
                
            translate([2.5*unit - eye_w/1.5, -head_d/2 - 0.1, head_h - 2.25*unit - eye_h/2 + eye_mm_height_adjust])
                chamfered_eye(is_left = false);
                
            // 5. PRECISION SCREEN RETENTION SLOT (Untouched coordinates)
            translate([-4.5*unit, -head_d/2 + 2.5, 2*unit + 2])
                cube([9*unit, glass_t + clearance, head_h - 3.8]);

            // 6. UPPER WIRING & PIN RELIEF CHANNEL (Untouched coordinates)
            translate([-4.5*unit, -head_d/2 + 2.5, head_h + unit - 2])
                cube([9*unit, head_d - 5, 3]);
                
            // 7. DIRECT REAR USB-C CABLE PASS-THROUGH (Untouched coordinates)
            translate([-8, head_d/2 - 4, 2*unit + usb_hole_height_adjuster])
                cube([16, 10, 10]);

            // 8. LEG ALIGNMENT HOLES
            if (print_mode == "base") {
                for(x = [-4.5*unit, -2.5*unit, 1.5*unit, 3.5*unit]) {
                    for(y = [-head_d/2, head_d/2 - unit]) {
                        // Cut upwards from the newly lowered chassis bottom
                        translate([x + 1.2 - clearance/2, y + 1.2 - clearance/2, 2*unit - bottom_thickness_added - 0.1])
                            cube([unit - 2.4 + clearance, unit - 2.4 + clearance, hole_depth + 0.1]);
                    }
                }
            }
            
            // 8.5 ARM ALIGNMENT HOLES
            if (print_mode == "base") {
                // Left Arm Hole
                translate([-head_w/2 - 0.1, -unit/2 + 1.2 - clearance/2, head_h/2 + unit + 1.2 - clearance/2])
                    cube([hole_depth + 0.1, unit - 2.4 + clearance, 2*unit - 2.4 + clearance]);
                    
                // Right Arm Hole
                translate([head_w/2 - hole_depth, -unit/2 + 1.2 - clearance/2, head_h/2 + unit + 1.2 - clearance/2])
                    cube([hole_depth + 0.1, unit - 2.4 + clearance, 2*unit - 2.4 + clearance]);
            }
                
            // 9. TUPPERWARE SPLIT CUTTERS
            if (print_mode == "base") {
                difference() {
                    translate([-head_w/2 - 5, -head_d/2 - 5, 2*unit + head_h - 4]) 
                        cube([head_w + 10, head_d + 10, 20]);
                    translate([-head_w/2 + 2, -head_d/2 + 2, 2*unit + head_h - 4]) 
                        cube([head_w - 4, head_d - 4, 2]);
                }
            }
            
            if (print_mode == "cap") {
                translate([-100, -100, -20]) 
                    cube([200, 200, 2*unit + head_h - 4 + 20]);
                    
                translate([-head_w/2 + 1.8, -head_d/2 + 1.8, 2*unit + head_h - 4 - 0.1]) 
                    cube([head_w - 3.6, head_d - 3.6, 2.2]);
            }
        }
    }

    // 10. INDEPENDENT LEGS MODE
    if (print_mode == "legs") {
        for(x = [-4.5*unit, -2.5*unit, 1.5*unit, 3.5*unit]) {
            for(y = [-head_d/2, head_d/2 - unit]) {
                union() {
                    // Leg base (exactly meets the lowered bottom of the chassis)
                    translate([x, y, 0]) 
                        cube([unit, unit, 2*unit - bottom_thickness_added]);
                    // Alignment pin
                    translate([x + 1.2, y + 1.2, 2*unit - bottom_thickness_added]) 
                        cube([unit - 2.4, unit - 2.4, pin_height]);
                }
            }
        }
    }
    
    // 11. INDEPENDENT ARMS MODE (Laid flat for vertical pin printing)
    if (print_mode == "arms") {
        // Generates two identical arms side-by-side on the Z=0 plane
        for(x = [-1.5*unit, 1.5*unit]) {
            translate([x - unit, -unit/2, 0]) {
                union() {
                    // Arm body (laying on its flat side)
                    cube([2*unit, unit, 2*unit]);
                    // Alignment pin (pointing straight up)
                    translate([1.2, 1.2, 2*unit]) 
                        cube([2*unit - 2.4, unit - 2.4, pin_height]);
                }
            }
        }
    }
}

// Module to create a slanted eye-hole cut with a flat roof for supports, ending in a soft outer chamfer
module chamfered_eye(is_left) {
    depth = 3.2; // Penetrates the 2.5mm front wall completely
    
    // Calculates the intermediate side/bottom flare to keep those slopes perfectly straight
    flare_mid = eye_chamfer * ((depth - top_softness) / depth);
    
    union() {
        // STAGE 1: MAIN BODY (Flat Top for Supports) 
        // Extends from the inside out to exactly 'top_softness' distance from the surface
        hull() {
            // INNER HOLE: Matches OLED screen exactly (touching the glass)
            translate([0, depth - 0.1, 0])
                cube([eye_w, 0.1, eye_h]);
                
            // MID HOLE: Transition point where the flat top ends
            if (is_left) {
                translate([-flare_mid, top_softness, -flare_mid])
                    cube([eye_w + flare_mid, 0.1, eye_h + flare_mid]);
            } else {
                translate([0, top_softness, -flare_mid])
                    cube([eye_w + flare_mid, 0.1, eye_h + flare_mid]);
            }
        }
        
        // STAGE 2: OUTER LIP (Softened/Chamfered Top) 
        // Covers the final distance to the outside face, ramping the roof up to break the sharp edge
        hull() {
            // MID HOLE: Transition point
            if (is_left) {
                translate([-flare_mid, top_softness, -flare_mid])
                    cube([eye_w + flare_mid, 0.1, eye_h + flare_mid]);
            } else {
                translate([0, top_softness, -flare_mid])
                    cube([eye_w + flare_mid, 0.1, eye_h + flare_mid]);
            }
            
            // OUTER HOLE: Flared sides/bottom AND flared top
            if (is_left) {
                translate([-eye_chamfer, 0, -eye_chamfer])
                    cube([eye_w + eye_chamfer, 0.1, eye_h + eye_chamfer + top_softness]);
            } else {
                translate([0, 0, -eye_chamfer])
                    cube([eye_w + eye_chamfer, 0.1, eye_h + eye_chamfer + top_softness]);
            }
        }
    }
}
