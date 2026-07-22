// =========================================================================
// CLAWDY MASCOT: ANTHROPIC STATUS API ENGINE
// Handles secure network polling, JSON parsing, and state management
// =========================================================================

#ifndef ANTHROPIC_STATUS_H
#define ANTHROPIC_STATUS_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// --- GLOBAL STATE VARIABLES ---
// Instantiated directly inside the header to cascade into your main sketch tab automatically
String downTargetText    = ""; 
int    activeOutageCount = 0;

// STATE TRACKER: 
// 0 = Normal Operation (Tandem blinks & tricks)
// 1 = Core Outage (Geometric 'X' Eyes)
// 2 = Systemic Error / Schema Mismatch / Network Failure (Angry Eyes)
int clawdExpressionState    = 0; 
int consecutiveNetworkFails = 0;
int allowedNetworkFails     = 3; // Threshold before triggering a systemic network error expression

// --- EXTERNAL LINKAGES ---
// Explicitly share these state flags with the main sketch tab
extern bool clawdOperational;
extern const unsigned long apiPollInterval;

// =========================================================================
// MAIN API POLLING FUNCTION
// Returns the number of milliseconds to wait before the next poll
// =========================================================================

unsigned long checkAnthropicStatus() {
  
  // Abort early if the network drops, returning standard polling interval
  if (WiFi.status() != WL_CONNECTED) return apiPollInterval;

  // === 1. SECURE CLIENT SETUP ===
  WiFiClientSecure client;
  client.setInsecure();     // Bypass certificate validation (easier for embedded devices)
  client.setTimeout(10000); // 10-second timeout to allow the SSL handshake to complete

  HTTPClient http;
  http.setTimeout(10000);   // 10-second timeout for the HTTP request to return data
  
  Serial.println("Polling https://status.claude.com/api/v2/summary.json ...");
  
  // === 2. HTTP REQUEST & ROUTING ===
  if (http.begin(client, "https://status.claude.com/api/v2/summary.json")) {

    // Spoof a standard desktop browser to bypass basic bot-blocking middleware (e.g., Cloudflare)
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

    int httpCode = http.GET();
    
    // === 3. PAYLOAD PARSING ===
    if (httpCode == HTTP_CODE_OK) {
      consecutiveNetworkFails = 0; // Reset fail counter on a successful 200 OK response
      String payload = http.getString();
      
      // Allocate memory for the JSON document. 
      // 6144 bytes is sized to safely deserialize Anthropic's full nested status payload
      DynamicJsonDocument doc(6144);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        JsonArray components = doc["components"];
        bool systemIsHealthy = true;
        int  matchedTargets  = 0;
        
        // Reset our global tracking registers before scanning the active live state
        activeOutageCount = 0;
        String singleDownName = "";

        // Iterate through the live payload to scan for our 4 critical targets
        for (JsonObject component : components) {
          const char* compName   = component["name"];
          const char* compStatus = component["status"];

          if (compName && compStatus) {
            // Verify if the current component is one we care about monitoring
            if (strcmp(compName, "claude.ai") == 0 || 
                strcmp(compName, "Claude Console (platform.claude.com)") == 0 || 
                strcmp(compName, "Claude API (api.anthropic.com)") == 0 || 
                strcmp(compName, "Claude Code") == 0) {
              
              matchedTargets++;
              
              Serial.print("Target Found -> ");
              Serial.print(compName);
              Serial.print(": ");
              Serial.println(compStatus);

              // Check if the specific target is experiencing any service degradation
              if (strcmp(compStatus, "operational") != 0) {
                systemIsHealthy = false;
                activeOutageCount++;
                
                // Track the name of the degraded system to display for single-outage cases
                if (strcmp(compName, "claude.ai") == 0) {
                  singleDownName = "CLAUDE.AI";
                } else if (strcmp(compName, "Claude Console (platform.claude.com)") == 0) {
                  singleDownName = "CLAUDE CONSOLE";
                } else if (strcmp(compName, "Claude API (api.anthropic.com)") == 0) {
                  singleDownName = "CLAUDE API";
                } else if (strcmp(compName, "Claude Code") == 0) {
                  singleDownName = "CLAUDE CODE";
                }
                
                Serial.println("^^ ALERT: Target state degraded!");
              }
            }
          }
        }

        // === 4. PIPELINE EVALUATION DISPATCHER ===
        // Determine the mascot's expression and output text based on the parsed data
        
        if (matchedTargets == 0) {
          // SYSTEMIC/SCHEMA ERROR: Endpoint was reached and parsed, but our specific targets vanished (API changed)
          Serial.println("Error: Schema mismatch. Zero targets matched.");
          downTargetText       = "SCHEMA ERROR";
          clawdExpressionState = 2; // ANGRY EYES
          clawdOperational     = false;
        } 
        else if (activeOutageCount == 1) {
          // VALID CLAUDE OUTAGE (Single Component is down)
          downTargetText       = singleDownName + " DOWN";
          clawdExpressionState = 1; // X EYES
          clawdOperational     = false;
        } 
        else if (activeOutageCount > 1) {
          // VALID CLAUDE OUTAGE (Multiple Components are down)
          downTargetText       = "Claude Outages (" + String(activeOutageCount) + ")";
          clawdExpressionState = 1; // X EYES
          clawdOperational     = false;
        } 
        else {
          // BASELINE OPERATION: All targeted systems are healthy
          downTargetText       = "";
          clawdExpressionState = 0; // NORMAL BLINKING EYES
          clawdOperational     = true;
        }

      } else {
        // ERROR: The HTTP request succeeded, but the payload wasn't valid JSON
        Serial.println("JSON Deserialization Error");
        downTargetText       = "PAYLOAD ERROR"; 
        clawdExpressionState = 2; // ANGRY EYES
        clawdOperational     = false; 
      }

      http.end();
      return apiPollInterval; // Return the standard 60-second delay

    } else {
      // ERROR: The server responded, but with a failing HTTP Code (e.g., 403, 500)
      Serial.print("HTTP Failure Code: ");
      Serial.println(httpCode);
      
      consecutiveNetworkFails++;
      http.end();

      if (consecutiveNetworkFails >= allowedNetworkFails) {
        downTargetText       = "HTTP ERROR (" + String(httpCode) + ")"; 
        clawdExpressionState = 2; // ANGRY EYES
        clawdOperational     = false; 
        return apiPollInterval; // Standard 60s wait so we don't hammer the network on hard fails
      } else {
        Serial.println("Network blip detected. Ignoring for one cycle.");
        return 5000; // Return a fast 5-second retry interval
      }
    }
  } else {
    // ERROR: The board couldn't even reach the URL (DNS issue, WiFi drop, SSL timeout)
    Serial.println("Unable to establish secure route connection");

    consecutiveNetworkFails++;
    http.end(); // Cleanup just in case

    if (consecutiveNetworkFails >= allowedNetworkFails) {
      downTargetText       = "ROUTE TIMEOUT"; 
      clawdExpressionState = 2; // ANGRY EYES
      clawdOperational     = false; 
      return apiPollInterval; // Standard 60s wait so we don't hammer the network on hard fails
    } else {
      Serial.println("Route timeout detected. Ignoring for one cycle.");
      return 5000; // Return a fast 5-second retry interval
    }
  }
}

#endif
