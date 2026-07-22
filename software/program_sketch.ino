// =========================================================================
// CLAWDY MASCOT: MAIN EXECUTION SCRIPT
// Hardware: ESP32 (Optimized for XIAO ESP32-C5) & Dual 0.96" I2C OLEDs
// =========================================================================

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#include <esp_wifi.h>

// LINK SEPARATE TAB LOGIC: Pulls in the network engine compiled in your other sub-files
// These files inject external variables like 'clawdExpressionState' and 'downTargetText'
#include "anthropic_status.h" 
#include "eye_tricks.h"

// --- OLED HARDWARE DEFINITIONS ---
#define SCREEN_WIDTH         128 
#define SCREEN_HEIGHT        64 
#define OLED_RESET           -1 
#define SCREEN_ADDRESS_LEFT  0x3D // Jumpered/Resistor-modified OLED address
#define SCREEN_ADDRESS_RIGHT 0x3C // Default OLED address

// --- ESP32 CHIP-SPECIFIC BOOT BUTTON PIN MAPPING ---
#if CONFIG_IDF_TARGET_ESP32C5
  #define BOOT_BUTTON 28 // XIAO ESP32-C5 boot pin
#elif CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C6
  #define BOOT_BUTTON 9  // Standard C3/C6 boot pin
#else
  #define BOOT_BUTTON 0  // Classic ESP32, S2, and S3 boot pin
#endif

// Instantiate the dual display objects sharing the same I2C bus
Adafruit_SSD1306 leftEye(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 rightEye(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- GLOBAL STATE VARIABLES ---
// Exposed to sub-tabs via 'extern' in their respective header files
bool clawdOperational    = true; 
bool eyesOpen            = true; 
int  lastExpressionState = 0; // Tracks previous state to detect recovery events

unsigned long nextTandemBlink = 0; 
unsigned long nextStatusCheck = 0;

const int           blinkDuration   = 150;   // Milliseconds the eyes stay closed during a blink
const unsigned long apiPollInterval = 60000; // Frequency of checking the Anthropic status API

// =========================================================================
// SYSTEM INITIALIZATION
// =========================================================================

void setup() {
  delay(500); // Startup rail power buffer to allow hardware to stabilize
  Serial.begin(115200);
  
  // Initialize I2C specifically mapped to the XIAO ESP32-C5 pins
  Wire.begin(D4, D5); 

  // Initialize both OLED "eyes" on the shared I2C bus
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

  // Center "CLAWDY" text (Width: 108px. Offset: X=10, Y=20)
  leftEye.setCursor(10, 20); leftEye.print("CLAWDY");
  rightEye.setCursor(10, 20); rightEye.print("CLAWDY");
  leftEye.display(); rightEye.display();

  delay(3000); // Hold splash screen for 3 seconds


  // === FACTORY RESET TRAP (THE STARTUP WINDOW) ===
  // Allows users to clear saved WiFi credentials if moving the device
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
      break; // Button pressed, break out of the timer early to immediately execute wipe
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
    
    wm.resetSettings(); // Nuke the saved WiFi memory
    
    delay(3000); // Give the user time to read confirmation
    ESP.restart(); // Reboot into the AP Setup portal
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

  // ===  Force MAC address change - only needed if firewalls block microcontrollers or odd MAC addresses
  // 1. Wake up the Wi-Fi interface in Client (Station) mode
  // WiFi.mode(WIFI_STA); 
  
  // 2. Clone the mobile phone's identity
  // uint8_t spoofedMAC[] = {<MAC_ADDRESS>}; 
  //esp_wifi_set_mac(WIFI_IF_STA, &spoofedMAC[0]);

  // Force Google/Cloudflare DNS 
  // (Passing zeros ensures the router still handles standard IP assignment via DHCP)
  IPAddress primaryDNS(8, 8, 8, 8); 
  IPAddress secondaryDNS(1, 1, 1, 1);
  WiFi.config(IPAddress(0,0,0,0), IPAddress(0,0,0,0), IPAddress(0,0,0,0), primaryDNS, secondaryDNS);

  // === START WIFI MANAGER ===
  WiFiManager wm;
  wm.setDebugOutput(false);
  wm.setConfigPortalTimeout(180); // 3-minute timeout for the captive portal

  if (!wm.autoConnect("Clawdy_Setup")) {
    Serial.println("Portal configuration timeout. Resetting...");
    leftEye.clearDisplay(); rightEye.clearDisplay();
    leftEye.setCursor(15, 30); leftEye.print("SETUP TIMEOUT");
    rightEye.setCursor(15, 30); rightEye.print("REBOOTING...");
    leftEye.display(); rightEye.display();
    delay(3000); // Give them 3 seconds to read it
    ESP.restart();
  }

  // Play a happy animation once connected (function provided by eye_tricks.h)
  executeTrick_Happy(3000);

  Serial.println("Network handshake secured successfully!");
  
  // Initialize random behaviors
  randomSeed(analogRead(0));
  nextTandemBlink = millis() + random(5000, 7000); // Stable 5-7 second tandem blink horizon
  nextTrickTime   = millis() + random(trickIntervalMin, trickIntervalMax); // Set the first happy trick time slot
  
  // Perform the initial API request immediately
  nextStatusCheck = millis() + checkAnthropicStatus();
}

// =========================================================================
// MAIN PROCESSING LOOP
// =========================================================================

void loop() {
  unsigned long currentTime = millis();

  // --- OUTAGE RECOVERY CELEBRATION ---
  // If we were previously in an error state (>0) and are now operational (0)
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

  // Re-stamp the static hardware assembly identifiers (default text settings)
  leftEye.setTextSize(1); rightEye.setTextSize(1);
  leftEye.setTextColor(SSD1306_WHITE); rightEye.setTextColor(SSD1306_WHITE);

  // --- RENDERING PIPELINE EXPRESSION SWITCH ---
  // clawdExpressionState is updated globally by checkAnthropicStatus()
  
  if (clawdExpressionState == 0) {
    
    // === STATE 0: NORMAL OPERATIONAL LOOP (SYNCHRONIZED TANDEM BLINK) ===
    
    // Check if it's time to interrupt the idle state with an animation trick
    if (currentTime >= nextTrickTime) {
      runRandomEyeTrick();
      nextTrickTime = currentTime + random(trickIntervalMin, trickIntervalMax); // Reset interval
    }
    
    // Handle the standard idle blinking logic
    if (currentTime >= nextTandemBlink) {
      eyesOpen = false;
      // Hold the blink shut until blinkDuration passes
      if (currentTime >= nextTandemBlink + blinkDuration) {
        eyesOpen = true;
        nextTandemBlink = currentTime + random(5000, 7000); // Schedule next blink
      }
    }

    // Draw the eyes (Open = Large squares, Closed = Flat lines)
    if (eyesOpen) {
      leftEye.fillRect( 44, 12, 40, 40, SSD1306_WHITE);
      rightEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    } else {
      leftEye.fillRect( 44, 30, 40, 6, SSD1306_WHITE);
      rightEye.fillRect(44, 30, 40, 6, SSD1306_WHITE);
    }

  } 
  else if (clawdExpressionState == 1) {
    
    // === STATE 1: VALID CLAUDE OUTAGE (CRISPY GEOMETRIC X SHAPES) ===
    
    // Left Eye 'X' (Drawn with multiple offset lines for thickness)
    leftEye.drawLine(44, 12, 84, 52, SSD1306_WHITE);
    leftEye.drawLine(44, 13, 83, 52, SSD1306_WHITE);
    leftEye.drawLine(45, 12, 84, 51, SSD1306_WHITE);
    leftEye.drawLine(84, 12, 44, 52, SSD1306_WHITE);
    leftEye.drawLine(84, 13, 45, 52, SSD1306_WHITE);
    leftEye.drawLine(83, 12, 44, 51, SSD1306_WHITE);

    // Right Eye 'X' (Drawn with multiple offset lines for thickness)
    rightEye.drawLine(44, 12, 84, 52, SSD1306_WHITE);
    rightEye.drawLine(44, 13, 83, 52, SSD1306_WHITE);
    rightEye.drawLine(45, 12, 84, 51, SSD1306_WHITE);
    rightEye.drawLine(84, 12, 44, 52, SSD1306_WHITE);
    rightEye.drawLine(84, 13, 45, 52, SSD1306_WHITE);
    rightEye.drawLine(83, 12, 44, 51, SSD1306_WHITE);

    // Dynamic text floor centering for the specific component that is down
    // (Text size 1 characters are 6px wide with spacing)
    int leftStartPos = (SCREEN_WIDTH - (downTargetText.length() * 6)) / 2;
    if (leftStartPos < 0) leftStartPos = 0; 
    leftEye.setCursor(leftStartPos, 53);  leftEye.print(downTargetText);
    rightEye.setCursor(leftStartPos, 53); rightEye.print(downTargetText);
    
  } 
  else if (clawdExpressionState == 2) {
    
    // === STATE 2: SYSTEMIC ERROR / SCHEMA MISMATCH (ANGRY BROW Slices) ===
    
    // Left Angry Eye (Square with a black triangle subtracted from the top)
    leftEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    leftEye.fillTriangle(44, 12, 84, 12, 84, 42, SSD1306_BLACK);

    // Right Angry Eye (Square with a black triangle subtracted from the top)
    rightEye.fillRect(44, 12, 40, 40, SSD1306_WHITE);
    rightEye.fillTriangle(44, 12, 84, 12, 44, 42, SSD1306_BLACK);

    // Dynamic text floor centering for Systemic alerts
    int leftStartPos = (SCREEN_WIDTH - (downTargetText.length() * 6)) / 2;
    if (leftStartPos < 0) leftStartPos = 0; 
    leftEye.setCursor(leftStartPos, 53);  leftEye.print(downTargetText);
    rightEye.setCursor(leftStartPos, 53); rightEye.print(downTargetText);
  }

  // Push buffer to the physical screens
  leftEye.display();
  rightEye.display();
  
  delay(10); // Short breather to prevent watchdog resets and screen tearing
}
