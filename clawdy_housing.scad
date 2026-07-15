// ==========================================
// CLAW-D MASCOT: INTERNAL ELECTRONICS SLED
// ==========================================

oled_w = 27.8; // old: 27.8
oled_h = 28.6; // old: 28.0 & 28.02

esp_l = 26.5; // esp cradel wall length
esp_w = 17.85; // esp cradel wall space distance (old: 18mm)
esp_h = 12.0; // esp cradel wall height
esp_z_lift = 6.0; // Raises the board to create component clearance underneath
esp_y_offset = 0; // Negative = closer to the front wall, Positive = further back

sled_width = 63.0;  

// --- ASYMMETRICAL ADJUSTMENT ---
// Front restored to original position (-16.5)
// Back extended by exactly 1mm from original (16.5 -> 17.5)
front_y = -16.5; 
back_y = 17.5;   
sled_depth = back_y - front_y; // Dynamically calculates to 34.0mm

sled_height = 65.0; 
wall_thick = 2.0;
front_wall_thick = 4.0; // RESTORED to 4.0 for OLED clearance
eye_z_offset = 25.12; // eyehole vertical position

render_sled();

module render_sled() {
    difference() {
        union() {
            // Main U-Shape Sled
            difference() {
                translate([-sled_width/2, front_y, 0])
                    cube([sled_width, sled_depth, sled_height]);
                
                translate([-(sled_width - wall_thick*2)/2, front_y + front_wall_thick, wall_thick])
                    cube([sled_width - wall_thick*2, sled_depth, sled_height]);
            }
            
            // ESP32 Cradle Base (Extended to front wall, raised by 2mm)
            translate([-(esp_w + 2)/2, front_y + front_wall_thick, wall_thick])
                cube([esp_w + 2, back_y - (front_y + front_wall_thick), esp_h + esp_z_lift]);

            // ESP32 Cradle shelf (Raised by 2mm)
            translate([-(esp_w + 2)/2, front_y + front_wall_thick, wall_thick + 12 + esp_z_lift])
                cube([esp_w + 2, ((back_y - esp_l + esp_y_offset) - (front_y + front_wall_thick)) + 14.5, 2]);
                
    
            // 0.5mm Friction Tolerance Rails (outside of sled to easily adjust length)
            translate([-sled_width/2 - 0.7, front_y, 0])
                cube([0.7, sled_depth, sled_height]);
            translate([sled_width/2, front_y, 0])
                cube([0.7, sled_depth, sled_height]);
        }
        
        // --- SUBTRACTIONS ---
        
        // ESP32 Drop-In Seat 
        // Lifted by esp_z_lift so the board rests on a 2mm ledge
        translate([-esp_w/2, back_y - esp_l + esp_y_offset, wall_thick + esp_z_lift + 0.1])
            cube([esp_w, esp_l + 1, esp_h]);

        // ESP32 Underside Component Clearance (USB & Buttons)
        // 10mm wide gap leaves ~3.9mm ledges on both sides for the board to sit on.
        depth = back_y - 16 + esp_y_offset;
        translate([-10/2, depth, 3])
            cube([10, 16.1, esp_z_lift + 0.2]);

        // Left OLED Pocket
        translate([-sled_width/2 + wall_thick + 0.5, front_y - 0.1, eye_z_offset])
            cube([oled_w, front_wall_thick + 1, oled_h]);
            
        // Right OLED Pocket
        translate([sled_width/2 - wall_thick - 0.5 - oled_w, front_y - 0.1, eye_z_offset])
            cube([oled_w, front_wall_thick + 1, oled_h]);
    }
}