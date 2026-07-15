#ifndef ANTHROPIC_STATUS_H
#define ANTHROPIC_STATUS_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Instantiated directly inside the header to cascade into your main sketch tab automatically
String downTargetText = ""; 
int activeOutageCount = 0;

// STATE TRACKER: 0 = Normal, 1 = Core Outage (X Eyes), 2 = Systemic Error (Angry Eyes)
int clawdExpressionState = 0; 
int consecutiveNetworkFails = 0;
int allowedNetworkFails = 3;

// Explicitly share these state flags back to the main sketch tab
extern bool clawdOperational;
extern const unsigned long apiPollInterval;

unsigned long checkAnthropicStatus() {
  if (WiFi.status() != WL_CONNECTED) return apiPollInterval;

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(10000); // Give the SSL handshake more time to complete

  HTTPClient http;
  http.setTimeout(10000); // Give the HTTP request more time to return data
  Serial.println("Polling https://status.claude.com/api/v2/summary.json ...");
  
  if (http.begin(client, "https://status.claude.com/api/v2/summary.json")) {
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      consecutiveNetworkFails = 0; // Reset fail counter on success
      String payload = http.getString();
      
      // Allocating space to safely deserialize the full nested array profile
      DynamicJsonDocument doc(6144);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        JsonArray components = doc["components"];
        bool systemIsHealthy = true;
        int matchedTargets = 0;
        
        // Reset our global tracking registers before counting the active live state
        activeOutageCount = 0;
        String singleDownName = "";

        // Iterate through the live payload to scan the 4 critical components
        for (JsonObject component : components) {
          const char* compName = component["name"];
          const char* compStatus = component["status"];

          if (compName && compStatus) {
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
                
                // Track the name of the last broken system found for single-outage cases
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

        // --- PIPELINE EVALUATION DISPATCHER ---
        if (matchedTargets == 0) {
          // SYSTEMIC/SCHEMA ERROR: Endpoint parsed but targets missing entirely
          Serial.println("Error: Schema mismatch. Zero targets matched.");
          downTargetText = "SCHEMA ERROR";
          clawdExpressionState = 2; // ANGRY EYES
          clawdOperational = false;
        } 
        else if (activeOutageCount == 1) {
          // VALID CLAUDE OUTAGE (Single Component)
          downTargetText = singleDownName + " DOWN";
          clawdExpressionState = 1; // X EYES
          clawdOperational = false;
        } 
        else if (activeOutageCount > 1) {
          // VALID CLAUDE OUTAGE (Multiple Components)
          downTargetText = "Claude Outages (" + String(activeOutageCount) + ")";
          clawdExpressionState = 1; // X EYES
          clawdOperational = false;
        } 
        else {
          // baseline operation -> All systems healthy
          downTargetText = "";
          clawdExpressionState = 0; // NORMAL BLINKING EYES
          clawdOperational = true;
        }

      } else {
        Serial.println("JSON Deserialization Error");
        downTargetText = "PAYLOAD ERROR"; 
        clawdExpressionState = 2; // ANGRY EYES
        clawdOperational = false; 
      }

      http.end();
      return apiPollInterval;

    } else {
      Serial.print("HTTP Failure Code: ");
      Serial.println(httpCode);
      
      consecutiveNetworkFails++;
      http.end();

      if (consecutiveNetworkFails >= allowedNetworkFails) {
        downTargetText = "HTTP ERROR (" + String(httpCode) + ")"; 
        clawdExpressionState = 2; // ANGRY EYES
        clawdOperational = false; 
        return apiPollInterval; // Standard 60s wait so we don't hammer the network
      } else {
        Serial.println("Network blip detected. Ignoring for one cycle.");
        return 5000; // Return the 5s fast-retry
      }
    }
  } else {
    Serial.println("Unable to establish secure route connection");

    consecutiveNetworkFails++;
    http.end(); //shouldn't be needed but adding it just in case

    if (consecutiveNetworkFails >= allowedNetworkFails) {
      downTargetText = "ROUTE TIMEOUT"; 
      clawdExpressionState = 2; // ANGRY EYES
      clawdOperational = false; 
      return apiPollInterval; // Standard 60s wait so we don't hammer the network
    } else {
      Serial.println("Route timeout detected. Ignoring for one cycle.");
        return 5000; // Return the 5s fast-retry
    }
  }
}

#endif
