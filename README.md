# Clawdy: The Anthropic Status Monitor

Clawdy is a dedicated desktop hardware monitor that tracks the operational status of Claude's infrastructure (claude.ai, Console, API, and Code). Using two I2C OLED displays as expressive "eyes," the system provides immediate, ambient visual feedback on service degradation, API outages, and network connectivity.

## Features
* **Real-Time API Polling:** Tracks `status.claude.com` for active outages and reflects the system health natively.
* **Expressive State Machine:** Features over a dozen eye animations (winks, lazy eyes, matrix rain) during normal operation, and distinct visual alert states (Angry Eyes, X Eyes) when infrastructure degrades or a schema mismatch occurs.
* **Resilient Polling:** Built with a reliability-first mindset, gracefully handling network timeouts, HTTP errors, and JSON payload changes without hard-crashing. 
* **Dynamic WiFi Provisioning:** Uses `WiFiManager` to spin up a captive portal (`Clawdy_Setup`) on first boot, keeping local network credentials entirely out of the source code.
* **Hardware Factory Reset:** Built-in startup trap allows for easy network clearing.

## 📶 Wi-Fi Setup & User Manual

Clawdy handles network credentials securely by keeping them off the codebase and storing them directly in local memory.
* The XIAO ESP32-C5 microcontroller supports dual-band Wi-Fi 6 (2.4 GHz and 5 GHz), allowing Clawdy to connect to most modern networks without compatibility issues.
* The Wi-Fi manager allows access to standard networks where a name and password are required. (Enterprise networks requiring a username/VPN are not currently supported).

### Initial Setup (First Boot)
1. **Power On:** Plug Clawdy in. You will see the "CLAWDY" splash screen for 3 seconds.
2. **Access Point Mode:** Because no networks are saved, the screens will read `"CONNECTING TO: WIFI"`. Clawdy will broadcast its own temporary Wi-Fi network.
3. **Connect:** Use your phone or computer to connect to the Wi-Fi network named **`Clawdy_Setup`**.
4. **Configure:** A captive portal should automatically pop up. (If it doesn't, open a web browser and navigate to `192.168.4.1`). Click "Configure WiFi", select your local network, and enter your password.
5. **Success:** Clawdy will save the credentials to its flash memory, automatically restart, and perform a "Happy" eye trick once connected to the internet.

### Setup Timeout 
For security and stability, the configuration portal will only stay active for **3 minutes (180 seconds)**. 
* If you do not enter credentials within that window, Clawdy will display `"SETUP TIMEOUT / REBOOTING..."` and restart itself to try again.

### How to Reset / Wipe Wi-Fi Credentials
If you move Clawdy to a new location or change your router password, you'll need to wipe the old network from memory:
1. Power cycle (unplug and replug) the device.
2. Immediately after the "CLAWDY" splash screen finishes, a 3-second trap window will appear reading `"HOLD 'BOOT' TO WIPE WIFI"`.
3. Press and hold your BOOT button (wired to GPIO 28) during this window.
4. The screens will switch to `"WIPING..."`. 
5. The device will completely erase the old credentials and reboot back into the initial `Clawdy_Setup` Access Point mode so you can connect it to the new network.

## Bill of Materials (BOM)

**Electronics & Circuitry**
* 1x Seeed Studio XIAO ESP32-C5 development board with soldered pins
* 2x 0.96 Inch OLED Display Module 128x64 SSD1306 (I2C)
* 1x Breadboard
* 24 AWG solid core hook wire kit (6 colors)
* 1x Power Battery
* 1x Carry case

**Soldering Equipment**
* Pinecil soldering iron
* 65W Anker Nano USB-C power adapter and high-power USB-C cable
* AUSTOR 60-40 Tin Lead Rosin Core Solder Wire
* Desoldering wick and soldering flux paste
* Soldering desk pad

**3D Printing & Enclosure**
* Terracotta Matte PLA filament

**Post-Processing & Finishing**
* Wet sandpaper (180, 300, 600, 1000, 1500, and 2000 grit)
* Micro-Mesh SoftTouch Pads (1500- 12000)
* Tamiya Polishing Compount Fine
* Tamiya Polishing Compount Finish
* Renaissance Micro-Crystalline Wax Polish (uncessary, but adds extra sheen and protection)
* High-quality microfiber towels
* High-quality wood polishing microfiber buffing pads

## 3D Printed Enclosure
* The repository includes a large edition Claw-D mascot chassis featuring a direct rear USB-C cable pass-through.
* The chassis features a precision screen retention slot designed to fit standard 0.96-inch PCBs.
* The base of the design utilizes a stable eight leg squid structure.
* The chassis OpenSCAD file supports multiple isolated print modes, allowing you to independently print the base, cap, legs, and arms.
* The independent arms mode automatically generates the pieces laid flat for vertical pin printing.
* The repository includes a separate internal electronics sled.
* The sled features specific left and right OLED pockets to securely seat the displays.
* The sled incorporates an ESP32 cradle base with dedicated underside clearance for USB ports and buttons.
* The sled utilizes 0.5mm friction tolerance rails on the outside to easily adjust the fit.
  * *Note on tolerances:* The 0.5mm friction rails and 0.4mm global clearances were designed and tested on a Bambu Lab P2S. If you are using an older or less precise printer, you may need to apply slight negative horizontal expansion in your slicer.

## Software Dependencies
* [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
* [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
* [WiFiManager](https://github.com/tzapu/WiFiManager)
* **ArduinoJson (IMPORTANT: Must use Version 6.x)** 
  *(Note: This codebase utilizes `DynamicJsonDocument`, which was deprecated in ArduinoJson v7).*

## Architecture Notes
* `program_sketch.ino`: The main operational loop and rendering pipeline.
* `anthropic_status.h`: The networking thread, managing HTTP/TLS connections and JSON payload deserialization.
* `eye_tricks.h`: The geometry and animation library for the OLED eyes.

## License
MIT License
