#pragma once
#include "esphome.h"
#include "esp_spp_api.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_main.h"
#include "esp_bt.h"
#include <array>

namespace esphome {
namespace yamaha_lsx {

class YamahaLSX : public Component, public light::LightOutput {
public:
    void setup() override;
    void loop() override;
    light::LightTraits get_traits() override;
    void write_state(light::LightState *state) override;
    void dump_config() override;

    static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    void connect();
    
    // Yamaha LSX MAC
    void set_mac_address(const std::array<uint8_t, 6> &mac);

protected:
    bool init_bluetooth();
    void send_packet(uint8_t cmd, uint8_t param);
    
    uint8_t peer_bd_addr_[6] = {0, 0, 0, 0, 0, 0}; 
    uint32_t handle_ = 0;
    bool connected_ = false;
    
    bool bt_initialized_ = false;
    uint32_t last_try_ = 0;
};

extern YamahaLSX *global_yamaha_instance;

} // namespace yamaha_lsx
} // namespace esphome
