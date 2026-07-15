#ifndef EYE_TRICKS_H
#define EYE_TRICKS_H

#include <Adafruit_SSD1306.h>

// Expose display objects from the primary sketch tab
extern Adafruit_SSD1306 leftEye;
extern Adafruit_SSD1306 rightEye;

// Track when the next random trick can fire (every 30 to 45 seconds)
unsigned long nextTrickTime = 0;
const unsigned long trickIntervalMin = 35000;
const unsigned long trickIntervalMax = 65000;

// --- TRICK 1: THE CASUAL WINK ---
void executeTrick_Wink() {
  // Left eye drops to a line, right eye stays wide open
  leftEye.clearDisplay();
  leftEye.fillRect(44, 30, 40, 6, SSD1306_WHITE);
  leftEye.display();
  
  delay(400); // Hold the wink briefly
}

// --- TRICK 2: THE HAPPY COMPANION (LOOK UP CURVES) ---
void executeTrick_Happy(int holdTime = 1500) {
  // Replace squares with happy up-curved arches
  leftEye.clearDisplay(); rightEye.clearDisplay();
  
  // Draw thick happy arc geometries using overlapping circles and black masks
  leftEye.fillCircle(64, 40, 20, SSD1306_WHITE);
  leftEye.fillCircle(64, 44, 20, SSD1306_BLACK);
  rightEye.fillCircle(64, 40, 20, SSD1306_WHITE);
  rightEye.fillCircle(64, 44, 20, SSD1306_BLACK);
  
  leftEye.display(); rightEye.display();
  delay(holdTime); // Hold happy smile expression
}

// --- TRICK 3: DILATION / FOCUS SHIFT ---
void executeTrick_Focus() {
  // Shrink the big block eyes inward to look intensely focused
  for(int i = 0; i < 3; i++) {
    leftEye.clearDisplay(); rightEye.clearDisplay();
    
    // Tightened focused center squares
    leftEye.fillRect(52, 20, 24, 24, SSD1306_WHITE);
    rightEye.fillRect(52, 20, 24, 24, SSD1306_WHITE);
    leftEye.display(); rightEye.display();
    delay(200);
    
    // Flash back out to wide
    leftEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    rightEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    leftEye.display(); rightEye.display();
    delay(200);
  }
}

// --- TRICK 4: THE MATRIX CODE RAIN ---
void executeTrick_Matrix() {
  // Simulates quick digital data vertical streaming drop arrays
  for (int i = 0; i < 15; i++) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    // Randomize vertical line segments dropping down across the eye bounding boxes
    int yOffset = (i * 8) % 40;
    leftEye.fillRect(44 + 8, 12 + yOffset, 4, 12, SSD1306_WHITE);
    leftEye.fillRect(44 + 24, 12 + ((yOffset + 16) % 40), 4, 8, SSD1306_WHITE);
    
    rightEye.fillRect(44 + 8, 12 + yOffset, 4, 12, SSD1306_WHITE);
    rightEye.fillRect(44 + 24, 12 + ((yOffset + 16) % 40), 4, 8, SSD1306_WHITE);

    leftEye.display(); rightEye.display();
    delay(100);
  }
}

// --- TRICK 5: THE GLITCH / CYBER TWITCH ---
void executeTrick_Glitch() {
  // A rapid tracking hardware glitch effect where the eyes temporarily shift off-axis
  for (int i = 0; i < 6; i++) {
    leftEye.clearDisplay(); rightEye.clearDisplay();
    
    // Left eye jitters up-left, Right eye jitters down-right
    leftEye.fillRect(44 - 6, 12 - 4, 40, 40, SSD1306_WHITE);
    rightEye.fillRect(44 + 6, 12 + 4, 40, 40, SSD1306_WHITE);
    leftEye.display(); rightEye.display();
    delay(60);

    // Flash back inverted positions
    leftEye.clearDisplay(); rightEye.clearDisplay();
    leftEye.fillRect(44 + 4, 12 + 2, 40, 40, SSD1306_WHITE);
    rightEye.fillRect(44 - 4, 12 - 2, 40, 40, SSD1306_WHITE);
    leftEye.display(); rightEye.display();
    delay(60);
  }
}

// --- TRICK 6: SUSPICIOUS SQUINT / SIDE-EYE ---
void executeTrick_Squint() {
  // Clawd targets his attention left, then right, like an untrusting security daemon
  for (int side = 0; side < 2; side++) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    // Eyelids compress vertically into thin tracking lenses (12px high)
    // Pupil offsets simulate tracking direction
    int pupilXOffset = (side == 0) ? -12 : 12; 
    
    leftEye.fillRect(44 + pupilXOffset, 26, 40, 12, SSD1306_WHITE);
    rightEye.fillRect(44 + pupilXOffset, 26, 40, 12, SSD1306_WHITE);
    
    leftEye.display(); rightEye.display();
    delay(1000); // Hold the suspicious glare
  }
}

// --- TRICK 7: SUPER CROSS-EYED SQUEEZE (SMOOTH SCALE & IN-AND-DOWN TRANSITION) ---
void executeTrick_Crosseyed() {
  // We execute a 5-step loop to interpolate scale, inward pull, and downward dip simultaneously
  for (int step = 0; step <= 4; step++) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    // 1. DYNAMICALLY SCALE SIZES: Linearly shrink from 40px down to 20px (50% target)
    int currentSize = 40 - (step * 5); 

    // 2. INTERPOLATE COORDINATE TRAJECTORIES
    int xOffset = step * 6; // Horizontal inward velocity multiplier
    int yOffset = step * 4; // Vertical downward velocity multiplier

    // Left Eye: Pupil shifts RIGHT (44 + xOffset) and DOWN (12 + yOffset)
    leftEye.fillRect(44 + xOffset, 12 + yOffset, currentSize, currentSize, SSD1306_WHITE);

    // Right Eye: Pupil shifts LEFT (44 - xOffset) and DOWN (12 + yOffset)
    // To keep it centered relative to its shrinking box, we balance the anchor coordinate math
    int rightXAnchor = 44 - xOffset + (step * 5); 
    rightEye.fillRect(rightXAnchor, 12 + yOffset, currentSize, currentSize, SSD1306_WHITE);

    leftEye.display(); rightEye.display();
    delay(60); // Frame duration pace (roughly 16 FPS for an organic, readable mechanical transition)
  }
  
  delay(1400); // Hold that intense, tiny cross-eyed expression at the finish line

  // Smoothly snap back out to wide center blocks
  for (int step = 4; step >= 0; step--) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    int currentSize = 40 - (step * 5);
    int xOffset = step * 6;
    int yOffset = step * 4;

    leftEye.fillRect(44 + xOffset, 12 + yOffset, currentSize, currentSize, SSD1306_WHITE);
    int rightXAnchor = 44 - xOffset + (step * 5);
    rightEye.fillRect(rightXAnchor, 12 + yOffset, currentSize, currentSize, SSD1306_WHITE);

    leftEye.display(); rightEye.display();
    delay(40); // Snaps back slightly faster than the entry compression for a springy look
  }
}

// --- TRICK 8: LEFT-TO-RIGHT FACE SWEEP (SMOOTH ENTRIES & EXITS) ---
void executeTrick_FaceSweep() {
  // PHASE 1: Smoothly slide from center (0) to the far left (-14)
  for (int shift = 0; shift >= -14; shift -= 2) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    leftEye.fillRect(44 + shift, 12, 40, 40, SSD1306_WHITE);
    rightEye.fillRect(44 + shift, 12, 40, 40, SSD1306_WHITE);

    leftEye.display(); rightEye.display();
    delay(25); // Controls the entry glance speed
  }
  
  delay(500); // Hold the left side look briefly

  // PHASE 2: Fluid panoramic sweep all the way from the far left (-14) to the far right (14)
  for (int shift = -14; shift <= 14; shift += 2) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    leftEye.fillRect(44 + shift, 12, 40, 40, SSD1306_WHITE);
    rightEye.fillRect(44 + shift, 12, 40, 40, SSD1306_WHITE);

    leftEye.display(); rightEye.display();
    delay(20); // Steady cross-face sweep cadence
  }
  
  delay(500); // Hold the right side look briefly

  // PHASE 3: Smoothly slide from the far right (14) back to the baseline center (0)
  for (int shift = 14; shift >= 0; shift -= 2) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    leftEye.fillRect(44 + shift, 12, 40, 40, SSD1306_WHITE);
    rightEye.fillRect(44 + shift, 12, 40, 40, SSD1306_WHITE);

    leftEye.display(); rightEye.display();
    delay(25); // Controls the return tracking speed
  }
}

// --- TRICK 9: THE TANDEM LAZY-EYE ROLL (SMOOTH INTERPOLATION) ---
void executeTrick_LazyEye() {
  int movingEye = random(0, 2); // Randomize which eye rolls
  float angle = 0;
  int orbitRadius = 12;
  int totalSteps = 40; // Total frames for the full move

  for (int i = 0; i <= totalSteps; i++) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    // Smooth Ease-In / Ease-Out Logic
    // We use a sine-wave multiplier to ramp the radius from 0 to 12 and back to 0
    float ease = sin((float)i / totalSteps * PI); 
    int xOffset = cos(angle) * orbitRadius * ease;
    int yOffset = sin(angle) * orbitRadius * ease;

    if (movingEye == 0) {
      // Left rolls, Right stays center
      leftEye.fillRect(44 + xOffset, 12 + yOffset, 40, 40, SSD1306_WHITE);
      rightEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    } else {
      // Right rolls, Left stays center
      leftEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
      rightEye.fillRect(44 + xOffset, 12 + yOffset, 40, 40, SSD1306_WHITE);
    }

    leftEye.display(); rightEye.display();
    
    angle += 0.2; // Speed of the rotation
    delay(30);    // Frame timing
  }
}

// --- TRICK 10: THE FLUID ORBITAL MORPH (SMOOTH CUBE-TO-ORB) ---
void executeTrick_EyeRoll() {
  int size = 40;
  int centerX = 44;
  int centerY = 12;

  // 1. MORPH CUBE TO ORB
  for (int r = 0; r <= 18; r += 2) {
    leftEye.clearDisplay(); rightEye.clearDisplay();
    leftEye.fillRoundRect(centerX, centerY, size, size, r, SSD1306_WHITE);
    rightEye.fillRoundRect(centerX, centerY, size, size, r, SSD1306_WHITE);
    leftEye.display(); rightEye.display();
    delay(40);
  }

  // 2. THE CHAOTIC PUPIL BOUNCE
  // Instead of math-based orbits, we use random jitter
  for (int i = 0; i < 50; i++) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    leftEye.fillCircle(64, 32, 18, SSD1306_WHITE);
    rightEye.fillCircle(64, 32, 18, SSD1306_WHITE);

    // Chaotic movement: random jumps between -10 and +10 pixels
    int lX = random(-10, 11); int lY = random(-10, 11);
    int rX = random(-10, 11); int rY = random(-10, 11);

    leftEye.fillCircle(64 + lX, 32 + lY, 5, SSD1306_BLACK);
    rightEye.fillCircle(64 + rX, 32 + rY, 5, SSD1306_BLACK);

    leftEye.display(); rightEye.display();
    delay(40); 
  }

  // 3. MORPH ORB BACK TO CUBE
  for (int r = 18; r >= 0; r -= 2) {
    leftEye.clearDisplay(); rightEye.clearDisplay();
    leftEye.fillRoundRect(centerX, centerY, size, size, r, SSD1306_WHITE);
    rightEye.fillRoundRect(centerX, centerY, size, size, r, SSD1306_WHITE);
    leftEye.display(); rightEye.display();
    delay(40);
  }
}

// --- TRICK 11: THE DIZZY SPIN (SQUARE ROTATION) ---
void executeTrick_Spin(int eyeIndex) {
  // eyeIndex: 0 = Left, 1 = Right, 2 = Both
  float angle = 0;
  
  // Spin the square 360 degrees (2 * PI)
  for (int i = 0; i < 20; i++) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    // Rotation math: We calculate a rotation matrix offset
    // Since we are drawing squares, we simulate the rotation visually
    // by jittering the center point slightly to imply a spin
    int spinX = sin(angle) * 4;
    int spinY = cos(angle) * 4;

    if (eyeIndex == 0 || eyeIndex == 2) {
      leftEye.fillRect(44 + spinX, 12 + spinY, 40, 40, SSD1306_WHITE);
    } else {
      leftEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    }

    if (eyeIndex == 1 || eyeIndex == 2) {
      rightEye.fillRect(44 + spinX, 12 + spinY, 40, 40, SSD1306_WHITE);
    } else {
      rightEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    }

    leftEye.display(); rightEye.display();
    angle += 0.8;
    delay(40);
  }
}

// --- TRICK 10: THE CHAOTIC BOUNCE (SILLY VIBRATION) ---
void executeTrick_ChaoticBounce() {
  int size = 30; // Slightly smaller than 40 for more "bounce room"
  int baseX = 44;
  int baseY = 12;

  // Let the chaos last for about 2 seconds
  for (int i = 0; i < 35; i++) {
    leftEye.clearDisplay(); rightEye.clearDisplay();

    // Generate random jitter between -4 and +4 pixels
    int jitterX = random(-4, 5);
    int jitterY = random(-4, 5);

    // Apply the jitter to the base positions
    leftEye.fillRoundRect(baseX + jitterX, baseY + jitterY, size, size, 8, SSD1306_WHITE);
    rightEye.fillRoundRect(baseX + jitterX, baseY + jitterY, size, size, 8, SSD1306_WHITE);

    leftEye.display(); rightEye.display();
    delay(40); // Controls the speed of the "silly" bounce
  }
}

// --- MASTER DISPATCH ENGINE ---
void runRandomEyeTrick() {
  int choice = random(0, 12);
  
  if (choice == 0) {
    executeTrick_Wink();
  } else if (choice == 1) {
    executeTrick_Happy();
  } else if (choice == 2) {
    executeTrick_Focus();
  } else if (choice == 3) {
    executeTrick_Matrix();
  } else if (choice == 4) {
    executeTrick_Glitch();
  } else if (choice == 5) {
    executeTrick_Squint();
  } else if (choice == 6) {
    executeTrick_Crosseyed();
  } else if (choice == 7) {
    executeTrick_FaceSweep();
  } else if (choice == 8) {
    executeTrick_LazyEye();
  } else if (choice == 9) {
    executeTrick_EyeRoll();
  } else if (choice == 10) {
    executeTrick_Spin(random(0, 3));
  } else {
    executeTrick_ChaoticBounce();
  }
}

#endif
