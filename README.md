# ESPHome Yamaha LSX-70 Light Bridge

This project provides a custom ESPHome component that allows you to control the built-in light of the **Yamaha LSX-70** Bluetooth speaker directly from Home Assistant using an ESP32 microcontroller. 

It uses Bluetooth Classic (SPP) to communicate with the speaker, reverse-engineering the serial protocol used by the official Yamaha app.

## ✨ Features
* **Power Control:** Turn the speaker's light ON and OFF.
* **Brightness Control:** Adjust the brightness level seamlessly. The Yamaha LSX-70 supports 5 distinct brightness levels, and this component automatically scales Home Assistant's 0-100% brightness slider to match the speaker's native levels (1-5).
* **Easy Integration:** Loadable as an `external_component` directly from GitHub—no manual file copying required!

## ⚠️ Requirements
1. **ESP32 Board:** An ESP32 is mandatory because it requires Bluetooth Classic capabilities (ESP8266 or ESP32-C3/S2/S3 models with BLE-only will not work).
2. **ESP-IDF Framework:** This component relies on the `esp-idf` framework in ESPHome, not Arduino.
3. **Custom Partitions:** Enabling Bluetooth Classic consumes a significant amount of the ESP32's flash memory. You will likely need a custom partition table (e.g., `partitions_custom.csv`) to successfully compile the firmware.

## 🚀 Installation & Configuration

You don't need to download any C++ or Python files manually. Just add the following configuration to your ESPHome YAML file. 

```yaml
# Example configuration for Yamaha LSX-70 Light Bridge
# Place this file in your ESPHome project directory.
# You will also need the 'partitions_custom.csv' file in the same folder.

esphome:
  name: yamaha-lsx-bridge
  friendly_name: Yamaha LSX Bridge

esp32:
  board: esp32dev
  framework:
    type: esp-idf
    # Bluetooth Classic requires specific SDK settings in ESP-IDF
    sdkconfig_options:
      CONFIG_BT_ENABLED: "y"
      CONFIG_BT_CLASSIC_ENABLED: "y"
      CONFIG_BT_SPP_ENABLED: "y"
      CONFIG_BT_SSP_ENABLED: "y" 
      CONFIG_BTDM_CTRL_MODE_BLE_ONLY: "n"
      CONFIG_BTDM_CTRL_MODE_BR_EDR_ONLY: "n"
      CONFIG_BTDM_CTRL_MODE_BTDM: "y"
      CONFIG_ESP32_WIFI_SW_COEXIST_ENABLE: "y"
  
  # Bluetooth Classic stack is large, so a custom partition table is required
  partitions: partitions_custom.csv

# Fetch the component from GitHub repository
external_components:
  - source:
      type: github
      url: https://github.com/klej00/esphome-yamaha-lsx
      # Optional: specify a branch or a tag
      # ref: main
    components: [ yamaha_lsx ]

# Enable logging
logger:
  level: INFO

# Basic networking (replace with your settings or use secrets)
wifi:
  ssid: "Your_WiFi_SSID"
  password: "Your_WiFi_Password"

# Required for Home Assistant integration
api:

ota:
  - platform: esphome

# The actual light configuration
light:
  - platform: yamaha_lsx
    name: "Yamaha LSX-70 Light"
    id: yamaha_light
    # Replace with the MAC address of your speaker
    mac_address: "00:00:00:00:00:00"
```

🔍 How to find your speaker's MAC address?
You can find the MAC address of your Yamaha LSX-70 by pairing it with an Android smartphone and checking the device details in your Bluetooth settings. Alternatively, you can use a free app like nRF Connect or Bluetooth Scanner on your phone to scan for nearby devices and copy the MAC address.

🛠️ Known Limitations
One-way Communication: The ESP32 sends commands to the speaker but does not actively read its status back. If you change the light state or brightness using the physical buttons on the speaker or the official Yamaha app, Home Assistant will not be aware of this change, and the state in the dashboard may become temporarily out of sync.

Connection delay: Bluetooth Classic takes a moment to establish a connection. If the ESP32 disconnects or restarts, it might take a few seconds before the light reacts to the first command.

Audio Playback: This component only controls the built-in light. It does not stream audio or interfere with existing audio connections to the speaker.

🤝 Credits
Created as a custom integration to bring 'dumb' Bluetooth speaker lights into the smart home ecosystem using ESPHome and Home Assistant.
