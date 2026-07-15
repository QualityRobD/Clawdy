#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>  

// LINK SEPARATE TAB LOGIC: Pulls in the network engine compiled in your other sub-file
#include "anthropic_status.h" 
#include "eye_tricks.h"

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET     -1 
#define SCREEN_ADDRESS_LEFT  0x3D
#define SCREEN_ADDRESS_RIGHT 0x3C
#define BOOT_BUTTON 9

Adafruit_SSD1306 leftEye(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 rightEye(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Global Variables (Declared here, exposed to sub-tab via 'extern')
bool clawdOperational = true; 
bool eyesOpen = true; 
int lastExpressionState = 0;

unsigned long nextTandemBlink = 0; 
unsigned long nextStatusCheck = 0;
const int blinkDuration = 150; 
const unsigned long apiPollInterval = 60000; 

void setup() {
  delay(500); // Startup rail power buffer
  Serial.begin(115200);
  
  Wire.begin(5, 6); // Fire up working Row 1 & Row 2 tracks

  // Connect to the oled "eyes"
  if(!leftEye.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS_LEFT)) {
    Serial.println(F("Left eye allocation failed"));
  }
  if(!rightEye.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS_RIGHT)) {
    Serial.println(F("Right eye allocation failed"));
  }

  // === STAGE 1: BOOT SPLASH SCREEN ===
  leftEye.clearDisplay(); rightEye.clearDisplay();
  leftEye.setTextSize(3); rightEye.setTextSize(3); // Large, bold text
  leftEye.setTextColor(SSD1306_WHITE); rightEye.setTextColor(SSD1306_WHITE);

  // Center "CLAWDY" (Width: 108px. Offset: X=10, Y=20)
  leftEye.setCursor(10, 20); leftEye.print("CLAWDY");
  rightEye.setCursor(10, 20); rightEye.print("CLAWDY");
  leftEye.display(); rightEye.display();

  delay(3000); // Hold splash screen for 3 seconds


  // === FACTORY RESET TRAP (THE STARTUP WINDOW) ===
  pinMode(BOOT_BUTTON, INPUT_PULLUP); 
  
  // 1. Display the 3-second warning window
  leftEye.clearDisplay(); rightEye.clearDisplay();
  leftEye.setTextSize(1); rightEye.setTextSize(1);
  leftEye.setCursor(15, 20); leftEye.print("HOLD 'BOOT'");
  rightEye.setCursor(15, 20); rightEye.print("HOLD 'BOOT'");
  leftEye.setCursor(15, 35); leftEye.print("TO WIPE WIFI");
  rightEye.setCursor(15, 35); rightEye.print("TO WIPE WIFI");
  leftEye.display(); rightEye.display();

  // 2. Scan the button rapidly for 3 seconds (30 loops of 100ms)
  bool wipeTriggered = false;
  for (int i = 0; i < 30; i++) {
    if (digitalRead(BOOT_BUTTON) == LOW) {
      wipeTriggered = true;
      break; // Button pressed, break out of the timer early
    }
    delay(100);
  }

  // 3. Execute the wipe if triggered
  if (wipeTriggered) {
    WiFiManager wm;
    leftEye.clearDisplay(); rightEye.clearDisplay();
    leftEye.setCursor(20, 30); leftEye.print("WIPING...");
    rightEye.setCursor(20, 30); rightEye.print("WIPING...");
    leftEye.display(); rightEye.display();
    
    wm.resetSettings(); // Nuke the memory
    
    delay(3000); // Give them time to read it
    ESP.restart(); // Reboot into the setup portal
  }

  // === STAGE 2: NETWORK HANDSHAKE SCREEN ===
  leftEye.clearDisplay(); rightEye.clearDisplay();

  // Line 1: "CONNECTING TO:" (Size 1. Width: 84px. Offset: X=22)
  leftEye.setTextSize(1); rightEye.setTextSize(1);
  leftEye.setCursor(22, 20); leftEye.print("CONNECTING TO:");
  rightEye.setCursor(22, 20); rightEye.print("CONNECTING TO:");
  
  // Line 2: "WIFI" (Size 2. Width: 48px. Offset: X=40)
  leftEye.setTextSize(2); rightEye.setTextSize(2);
  leftEye.setCursor(40, 35); leftEye.print("WIFI");
  rightEye.setCursor(40, 35); rightEye.print("WIFI");
  
  leftEye.display(); rightEye.display();

  WiFiManager wm;
  wm.setDebugOutput(false);
  wm.setConfigPortalTimeout(180); 

if (!wm.autoConnect("Clawdy_Setup")) {
    Serial.println("Portal configuration timeout. Resetting...");
    leftEye.clearDisplay(); rightEye.clearDisplay();
    leftEye.setCursor(15, 30); leftEye.print("SETUP TIMEOUT");
    rightEye.setCursor(15, 30); rightEye.print("REBOOTING...");
    leftEye.display(); rightEye.display();
    delay(3000); // Give them 3 seconds to read it
    ESP.restart();
  }

  executeTrick_Happy(3000);

  Serial.println("Network handshake secured successfully!");
  
  randomSeed(analogRead(0));
  nextTandemBlink = millis() + random(5000, 7000); // Stable 5-7 second tandem horizon [cite: 4731]
  nextTrickTime = millis() + random(trickIntervalMin, trickIntervalMax); // Sets the first happy trick time slot
  
  // First run
  nextStatusCheck = millis() + checkAnthropicStatus();
}

void loop() {
  unsigned long currentTime = millis();

  // --- OUTAGE RECOVERY CELEBRATION ---
  if (lastExpressionState != 0 && clawdExpressionState == 0) {
    executeTrick_Happy(); // Smile because the servers are back!
  }
  lastExpressionState = clawdExpressionState; // Update the tracking record

  // --- API ROUTING NETWORK THREAD ---
  if (currentTime >= nextStatusCheck) {
    nextStatusCheck = millis() + checkAnthropicStatus();
  }

  // --- RENDERING ROUTINES ---
  leftEye.clearDisplay();
  rightEye.clearDisplay();

  // Re-stamp the static hardware assembly identifiers [cite: 1535]
  leftEye.setTextSize(1); rightEye.setTextSize(1);
  leftEye.setTextColor(SSD1306_WHITE); rightEye.setTextColor(SSD1306_WHITE);

  // --- RENDERING PIPELINE EXPRESSION SWITCH ---
  if (clawdExpressionState == 0) {
    // === STATE 0: NORMAL OPERATIONAL LOOP (SYNCHRONIZED TANDEM BLINK) ===
    
    // Check if it's time for an operational baseline trick expression
    if (currentTime >= nextTrickTime) {
      runRandomEyeTrick();
      nextTrickTime = currentTime + random(trickIntervalMin, trickIntervalMax); // Reset interval
    }
    
    if (currentTime >= nextTandemBlink) {
      eyesOpen = false;
      if (currentTime >= nextTandemBlink + blinkDuration) {
        eyesOpen = true;
        nextTandemBlink = currentTime + random(5000, 7000); 
      }
    }

    if (eyesOpen) {
      leftEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
      rightEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    } else {
      leftEye.fillRect(44, 30, 40, 6, SSD1306_WHITE);
      rightEye.fillRect(44, 30, 40, 6, SSD1306_WHITE);
    }

  } 
  else if (clawdExpressionState == 1) {
    // === STATE 1: VALID CLAUDE OUTAGE (CRISPY GEOMETRIC X SHAPES) ===
    // Left Eye X
    leftEye.drawLine(44, 12, 84, 52, SSD1306_WHITE);
    leftEye.drawLine(44, 13, 83, 52, SSD1306_WHITE);
    leftEye.drawLine(45, 12, 84, 51, SSD1306_WHITE);
    leftEye.drawLine(84, 12, 44, 52, SSD1306_WHITE);
    leftEye.drawLine(84, 13, 45, 52, SSD1306_WHITE);
    leftEye.drawLine(83, 12, 44, 51, SSD1306_WHITE);

    // Right Eye X
    rightEye.drawLine(44, 12, 84, 52, SSD1306_WHITE);
    rightEye.drawLine(44, 13, 83, 52, SSD1306_WHITE);
    rightEye.drawLine(45, 12, 84, 51, SSD1306_WHITE);
    rightEye.drawLine(84, 12, 44, 52, SSD1306_WHITE);
    rightEye.drawLine(84, 13, 45, 52, SSD1306_WHITE);
    rightEye.drawLine(83, 12, 44, 51, SSD1306_WHITE);

    // Dynamic text floor centering
    int leftStartPos = (SCREEN_WIDTH - (downTargetText.length() * 6)) / 2;
    if (leftStartPos < 0) leftStartPos = 0; 
    leftEye.setCursor(leftStartPos, 53); leftEye.print(downTargetText);
    rightEye.setCursor(leftStartPos, 53); rightEye.print(downTargetText);
  } 
  else if (clawdExpressionState == 2) {
    // === STATE 2: SYSTEMIC ERROR / SCHEMA MISMATCH (ANGRY BROW Slices) ===
    // Left Angry Eye
    leftEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    leftEye.fillTriangle(44, 12, 84, 12, 84, 42, SSD1306_BLACK);

    // Right Angry Eye
    rightEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    rightEye.fillTriangle(44, 12, 84, 12, 44, 42, SSD1306_BLACK);

    // Dynamic text floor centering for Systemic alerts
    int leftStartPos = (SCREEN_WIDTH - (downTargetText.length() * 6)) / 2;
    if (leftStartPos < 0) leftStartPos = 0; 
    leftEye.setCursor(leftStartPos, 53); leftEye.print(downTargetText);
    rightEye.setCursor(leftStartPos, 53); rightEye.print(downTargetText);
  }

  leftEye.display();
  rightEye.display();
  delay(10); 
}
