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

**Note:** Remember to replace `YourUsername` in the URL with your actual GitHub username, and update the `mac_address` to match your speaker.

esphome:
  name: yamaha-bridge
  friendly_name: Yamaha Bridge

esp32:
  board: esp32dev
  framework:
    type: esp-idf
    # Mandatory SDK configurations to enable Bluetooth Classic
    sdkconfig_options:
      CONFIG_BT_ENABLED: "y"
      CONFIG_BT_CLASSIC_ENABLED: "y"
      CONFIG_BT_SPP_ENABLED: "y"
      CONFIG_BT_SSP_ENABLED: "y" 
      CONFIG_BTDM_CTRL_MODE_BLE_ONLY: "n"
      CONFIG_BTDM_CTRL_MODE_BR_EDR_ONLY: "n"
      CONFIG_BTDM_CTRL_MODE_BTDM: "y"
      CONFIG_ESP32_WIFI_SW_COEXIST_ENABLE: "y"
  
  # Highly recommended: Use a custom partition table to fit the BT Classic stack
  partitions: partitions_custom.csv

# Fetch the component directly from GitHub
external_components:
  - source:
      type: github
      url: [https://github.com/YourUsername/esphome-yamaha-lsx](https://github.com/YourUsername/esphome-yamaha-lsx)
    components: [ yamaha_lsx ]

# Define your light entity
light:
  - platform: yamaha_lsx
    name: "Yamaha LSX-70 Light"
    id: yamaha_light_entity
    mac_address: "00:1F:47:EB:89:40" # <-- Replace with your Yamaha's MAC address


    
