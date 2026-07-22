// =========================================================================
// CLAW-D MASCOT: LARGE EDITION (SLED READY, REAR SCREW MOUNTS)
// =========================================================================

/* [Fixed Hardware Specifications] */
pcb_w     = 27.3;             // Physical width of standard 0.96" OLED PCB
glass_t   = 2.4;              // Depth from front of OLED glass to back of PCB
clearance = 0.4;              // Global 3D printing tolerance fit
$fn       = 24;               // Curve smoothness (resolution for cylinders/arcs)

/* [Adjustable Features] */
print_mode           = "base"; // [ "assembled", "base", "cap", "legs", "arms" ]
eye_w                = 25;     // Width of the front eye cutouts (original: 23)
eye_h                = 12;     // Height of the front eye cutouts (original: 14)
eye_mm_height_adjust = 5;      // Vertical shift for the eye cutouts
eye_chamfer          = 2.0;    // Outward slant on outer edges for better viewing angle
top_softness         = 1.0;    // Top edge chamfer amount (leaves the rest flat for supports)
leg_inset            = 3.0;    // Inward adjustment distance for front and back legs

usb_hole_height_adjuster = 5.0; // Vertical shift for the rear USB-C cable pass-through

// Variables to protect the internal cavity while adding alignment pin holes
bottom_thickness_added = 2.4;  // Extra thickness added to the base for structural integrity
pin_height             = 3.0;  // Height of the alignment pins (arms/legs)
hole_depth             = 3.6;  // Depth of the receiving holes (allows 0.6mm clearance for glue/fit)

/* [Base Pocket Adjustments] */
pocket_w_extra = clearance + 0.2; // Extra width for base pocket (default was global clearance: 0.4)
pocket_d_extra = clearance + 0.2; // Extra depth for base pocket (default was global clearance: 0.4)
pocket_h_extra = clearance + 0.0; // Extra height/depth for base pocket (default was hardcoded 0.1)

/* [Magnetic Clasp & Split] */
magnet_dia      = 5.2; // Diameter of hidden pocket (+0.2mm tolerance for 5mm magnet)
magnet_depth    = 3.2; // Depth of hidden pocket (+0.2mm tolerance for 3mm magnet)
cover_thickness = 0.6; // Layers of plastic hiding the magnet (0.6 = 3x 0.2mm print layers)

/* [Sled Button Alignment] */
button_hole_d              = 2.0;  // Hole size for boot/reset access (sized for a paperclip)
esp_w                      = 18.2; // Matches internal sled's ESP width for exact offset mapping
left_button_hole_x_offset  = 4.25; // X-axis offset for the left (BOOT) button
right_button_hole_x_offset = 5.0;  // X-axis offset for the right (RESET) button
button_hole_y              = -5;   // Y-axis depth alignment (Maps to sled's updated math: 17.5 - 22.25)

// =========================================================================
// MAIN EXECUTION
// =========================================================================

if (print_mode == "cap") {
    // Flips the cap 180 degrees and drops the flat seam exactly to the Z=0 build plate
    // Math: Z-height shift = (2 * unit + head_h - 4)
    translate([0, 0, 2*7.2 + 69.12 - 4]) 
        rotate([180, 0, 0]) 
            clawd_chassis();
} else {
    clawd_chassis();
}

// =========================================================================
// CHASSIS MODULE
// =========================================================================

module clawd_chassis() {
    
    // 1. HARDCODED LARGE PARAMETERS
    unit   = 7.2;
    head_w = 86.4;    // 12 * 7.2 (Total head width)
    head_h = 69.12;   // 86.4 / 1.25 (Total head height proportional to width)
    head_d = 39.6;    // 5.5 * 7.2 (Total head depth)
    
    // Split line and Boss parameters for two-part printing
    split_z = 2*unit + head_h - 4; // Z-height where the cap splits from the base
    boss_w  = 6.0;                 // Width of the alignment bosses
    boss_d  = 12.0;                // Depth of the alignment bosses
    boss_h  = 3.0;                 // Height of the alignment bosses
    boss_x  = 37.8;                // X-axis position (centered precisely in the solid side-walls)

    // Render the main body if we are printing the base, cap, or full assembly
    if (print_mode == "base" || print_mode == "cap" || print_mode == "assembled") {
        difference() {
            // 2. THE ACCURATE PROPORTIONAL OUTER SHELL
            union() {
                // Main Head (Bottom extended downward by bottom_thickness_added)
                translate([-head_w/2, -head_d/2, 2*unit - bottom_thickness_added]) 
                    cube([head_w, head_d, head_h + bottom_thickness_added]);
                    
                // Structural Alignment Bosses (Added to Cap for reassembly)
                if (print_mode == "cap" || print_mode == "assembled") {
                    translate([boss_x - boss_w/2, -boss_d/2, split_z - boss_h])
                        cube([boss_w, boss_d, boss_h]);
                    translate([-boss_x - boss_w/2, -boss_d/2, split_z - boss_h])
                        cube([boss_w, boss_d, boss_h]);
                }
                
                // Attach non-removable limbs for 'assembled' visualization
                if (print_mode == "assembled") {
                    // Left Side Stub/Arm
                    translate([-head_w/2 - 2*unit, -unit/2, head_h/2 + unit]) 
                        cube([2*unit, unit, 2*unit]);
                    
                    // Right Side Stub/Arm
                    translate([head_w/2, -unit/2, head_h/2 + unit]) 
                        cube([2*unit, unit, 2*unit]);
                        
                    // ---- STABLE EIGHT LEG SQUID STRUCTURE ----
                    for(x = [-4.5*unit, -2.5*unit, 1.5*unit, 3.5*unit]) {
                        for(y = [-head_d/2 + leg_inset, head_d/2 - unit - leg_inset]) {
                            // Legs end where the newly lowered chassis begins
                            translate([x, y, 0]) 
                                cube([unit, unit, 2*unit - bottom_thickness_added]);
                        }
                    }
                }
            }
            
            // 3. INTERNAL CAVITY (Untouched coordinates)
            // Hollows out the center for the PCB sled and wiring
            translate([-4.5*unit, -head_d/2 + 2.5, 2*unit + 2])
                cube([9*unit, head_d - 5, head_h - 3.8]);
                
            // 4. FRONT FACE EYE WINDOWS (Two-Stage Cut for Supports + Soft Edge)
            eye_shift_x = 0.4; // Fine-tuning offset to perfectly center the eyes
            
            // Left Eye Cutout
            translate([-2.5*unit - eye_w/2.75 + eye_shift_x, -head_d/2 - 0.1, head_h - 2.25*unit - eye_h/2 + eye_mm_height_adjust])
                chamfered_eye(is_left = true);
                
            // Right Eye Cutout
            translate([2.5*unit - eye_w/1.5 + eye_shift_x, -head_d/2 - 0.1, head_h - 2.25*unit - eye_h/2 + eye_mm_height_adjust])
                chamfered_eye(is_left = false);
                
            // 5. PRECISION SCREEN RETENTION SLOT (Untouched coordinates)
            // Secures the OLED glass directly against the front wall
            translate([-4.5*unit, -head_d/2 + 2.5, 2*unit + 2])
                cube([9*unit, glass_t + clearance, head_h - 3.8]);

            // 6. UPPER WIRING & PIN RELIEF CHANNEL (Untouched coordinates)
            // Provides headroom for dupont wires or headers at the top of the sled
            translate([-4.5*unit, -head_d/2 + 2.5, head_h + unit - 2])
                cube([9*unit, head_d - 5, 3]);
                
            // 7. DIRECT REAR USB-C CABLE PASS-THROUGH (Untouched coordinates)
            translate([-8, head_d/2 - 4, 2*unit + usb_hole_height_adjuster])
                cube([16, 10, 10]);

            // 8. LEG ALIGNMENT HOLES
            if (print_mode == "base") {
                for(x = [-4.5*unit, -2.5*unit, 1.5*unit, 3.5*unit]) {
                    for(y = [-head_d/2 + leg_inset, head_d/2 - unit - leg_inset]) {
                        // Cut upwards from the newly lowered chassis bottom to receive leg pins
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
                
            // 9. FLAT SEAM SPLIT (Creates the physical separation between Cap and Base)
            if (print_mode == "base") {
                // Flush cut everything above the split line
                translate([-head_w/2 - 5, -head_d/2 - 5, split_z]) 
                    cube([head_w + 10, head_d + 10, 20]);
            }
            
            if (print_mode == "cap") {
                // Flush cut everything below the split line, EXCEPT the bosses
                difference() {
                    translate([-100, -100, -20]) 
                        cube([200, 200, split_z + 20]);
                        
                    // Protect bosses from being severed by the flush cut
                    translate([boss_x - boss_w/2 - 0.1, -boss_d/2 - 0.1, split_z - boss_h - 0.1])
                        cube([boss_w + 0.2, boss_d + 0.2, boss_h + 0.2]);
                    translate([-boss_x - boss_w/2 - 0.1, -boss_d/2 - 0.1, split_z - boss_h - 0.1])
                        cube([boss_w + 0.2, boss_d + 0.2, boss_h + 0.2]);
                }
            }

            // 9.5 ALIGNMENT POCKETS & HIDDEN MAGNET VOIDS
            if (print_mode == "base") {
                // Clearance pockets for the Cap bosses to slot into
                translate([boss_x - boss_w/2 - pocket_w_extra/2, -boss_d/2 - pocket_d_extra/2, split_z - boss_h])
                    cube([boss_w + pocket_w_extra, boss_d + pocket_d_extra, boss_h + pocket_h_extra]);
                translate([-boss_x - boss_w/2 - pocket_w_extra/2, -boss_d/2 - pocket_d_extra/2, split_z - boss_h])
                    cube([boss_w + pocket_w_extra, boss_d + pocket_d_extra, boss_h + pocket_h_extra]);
                    
                // Hidden magnet voids below the clearance pockets
                // (Intended for a mid-print pause to drop magnets in before covering the top)
                translate([boss_x, 0, split_z - boss_h - cover_thickness - magnet_depth])
                    cylinder(h=magnet_depth, d=magnet_dia);
                translate([-boss_x, 0, split_z - boss_h - cover_thickness - magnet_depth])
                    cylinder(h=magnet_depth, d=magnet_dia);
            }
            
            // 10. SLED BUTTON HOLES (BOOT / RESET access from underneath)
            if (print_mode == "base" || print_mode == "assembled") {
                // Left Hole (Boot)
                translate([-esp_w/2 + left_button_hole_x_offset, button_hole_y, 0])
                    cylinder(d=button_hole_d, h=20, $fn=30);
                
                // Right Hole (Reset)
                translate([esp_w/2 - right_button_hole_x_offset, button_hole_y, 0])
                    cylinder(d=button_hole_d, h=20, $fn=30);
            }
            
            if (print_mode == "cap") {
                // Hidden magnet voids inside the protruding bosses
                // (When cap is printed upside down, printer pauses here before bridging the floor)
                translate([boss_x, 0, split_z - boss_h + cover_thickness])
                    cylinder(h=magnet_depth, d=magnet_dia);
                translate([-boss_x, 0, split_z - boss_h + cover_thickness])
                    cylinder(h=magnet_depth, d=magnet_dia);
            }
        }
    }

    // 10. INDEPENDENT LEGS MODE
    if (print_mode == "legs") {
        for(x = [-4.5*unit, -2.5*unit, 1.5*unit, 3.5*unit]) {
            for(y = [-head_d/2 + leg_inset, head_d/2 - unit - leg_inset]) {
                union() {
                    // Leg base (exactly meets the lowered bottom of the chassis)
                    translate([x, y, 0]) 
                        cube([unit, unit, 2*unit - bottom_thickness_added]);
                    // Alignment pin (inserts into chassis hole)
                    translate([x + 1.2, y + 1.2, 2*unit - bottom_thickness_added]) 
                        cube([unit - 2.4, unit - 2.4, pin_height]);
                }
            }
        }
    }
    
    // 11. INDEPENDENT ARMS MODE (Laid flat for vertical pin printing)
    if (print_mode == "arms") {
        // Generates two identical arms side-by-side on the Z=0 build plane
        for(x = [-1.5*unit, 1.5*unit]) {
            translate([x - unit, -unit/2, 0]) {
                union() {
                    // Arm body (laying on its flat side for strength)
                    cube([2*unit, unit, 2*unit]);
                    // Alignment pin (pointing straight up)
                    translate([1.2, 1.2, 2*unit]) 
                        cube([2*unit - 2.4, unit - 2.4, pin_height]);
                }
            }
        }
    }
}

// =========================================================================
// HELPER MODULES
// =========================================================================

// Module to create a slanted eye-hole cut with a flat roof for FDM supports, ending in a soft outer chamfer
module chamfered_eye(is_left) {
    depth = 3.2; // Distance needed to penetrate the 2.5mm front wall completely
    
    // Calculates the intermediate side/bottom flare to keep those slopes perfectly straight
    flare_mid = eye_chamfer * ((depth - top_softness) / depth);
    
    union() {
        // STAGE 1: MAIN BODY (Flat Top for Supports) 
        // Extends from the inside out to exactly 'top_softness' distance from the outer surface
        hull() {
            // INNER HOLE: Matches OLED screen exactly (touching the glass)
            translate([0, depth - 0.1, 0])
                cube([eye_w, 0.1, eye_h]);
                
            // MID HOLE: Transition point where the flat top ends and chamfer begins
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
            // MID HOLE: Transition point (same as above)
            if (is_left) {
                translate([-flare_mid, top_softness, -flare_mid])
                    cube([eye_w + flare_mid, 0.1, eye_h + flare_mid]);
            } else {
                translate([0, top_softness, -flare_mid])
                    cube([eye_w + flare_mid, 0.1, eye_h + flare_mid]);
            }
            
            // OUTER HOLE: Flared sides/bottom AND flared top (completes the chamfer)
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
