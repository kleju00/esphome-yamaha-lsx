#include "yamaha_lsx.h"
#include "esphome/core/log.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"

namespace esphome {
namespace yamaha_lsx {

static const char *TAG = "yamaha_lsx";
YamahaLSX *global_yamaha_instance = nullptr;

void YamahaLSX::set_mac_address(const std::array<uint8_t, 6> &mac) {
    std::copy(mac.begin(), mac.end(), this->peer_bd_addr_);
}

void YamahaLSX::setup() {
    global_yamaha_instance = this;
    ESP_LOGI(TAG, "Yamaha LSX: Oczekiwanie na start Bluetooth...");
}

void YamahaLSX::send_packet(uint8_t cmd, uint8_t param) {
    if (!this->connected_) return;

    // Protokół: [CC AA] [03 23 CMD PARAM] [CHECKSUM]
    uint8_t payload[4] = {0x03, 0x23, cmd, param};

    int sum = 0;
    for (int i = 0; i < 4; i++) sum += payload[i];
    uint8_t checksum = (uint8_t)((256 - (sum % 256)) & 0xFF);

    uint8_t frame[7];
    frame[0] = 0xCC; 
    frame[1] = 0xAA;
    memcpy(&frame[2], payload, 4);
    frame[6] = checksum;

    ESP_LOGD(TAG, "TX: %02X %02X %02X %02X %02X %02X %02X", 
             frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6]);

    esp_spp_write(this->handle_, 7, frame);
}

bool YamahaLSX::init_bluetooth() {
    esp_err_t ret;
    ESP_LOGW(TAG, "--- Startowanie Bluetooth ---");

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) return false;
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM)) != ESP_OK) return false;
    if ((ret = esp_bluedroid_init()) != ESP_OK) return false;
    if ((ret = esp_bluedroid_enable()) != ESP_OK) return false;
    if ((ret = esp_spp_register_callback(YamahaLSX::esp_spp_cb)) != ESP_OK) return false;

    esp_spp_cfg_t bt_spp_cfg = {
        .mode = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = true,
        .tx_buffer_size = 0, 
    };
    if ((ret = esp_spp_enhanced_init(&bt_spp_cfg)) != ESP_OK) return false;

    ESP_LOGI(TAG, "--- Bluetooth gotowy ---");
    return true;
}

void YamahaLSX::loop() {
    if (!this->bt_initialized_) {
        uint32_t now = millis();
        if (now - this->last_try_ > 5000) {
            this->last_try_ = now;
            if (this->init_bluetooth()) {
                this->bt_initialized_ = true;
            }
        }
    }
}

void YamahaLSX::connect() {
    if (this->connected_) return;
    ESP_LOGI(TAG, "Łączenie z głośnikiem (Kanał 1)...");
    esp_spp_connect(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_MASTER, 1, this->peer_bd_addr_);
}

void YamahaLSX::esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    if (global_yamaha_instance == nullptr) return;

    switch (event) {
        case ESP_SPP_INIT_EVT:
            global_yamaha_instance->connect();
            break;
            
        case ESP_SPP_OPEN_EVT:
            if (param->open.status == ESP_SPP_SUCCESS) {
                ESP_LOGI(TAG, "POŁĄCZONO! Handle: %d", param->open.handle);
                global_yamaha_instance->connected_ = true;
                global_yamaha_instance->handle_ = param->open.handle;
            } else {
                global_yamaha_instance->connected_ = false;
                global_yamaha_instance->set_timeout("reconnect", 10000, []() {
                    if(global_yamaha_instance && global_yamaha_instance->bt_initialized_) 
                        global_yamaha_instance->connect();
                });
            }
            break;

        case ESP_SPP_CLOSE_EVT:
            ESP_LOGW(TAG, "Rozłączono.");
            global_yamaha_instance->connected_ = false;
            global_yamaha_instance->set_timeout("reconnect", 10000, []() {
                if(global_yamaha_instance && global_yamaha_instance->bt_initialized_) 
                    global_yamaha_instance->connect();
            });
            break;
        default: break;
    }
}

light::LightTraits YamahaLSX::get_traits() {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
    return traits;
}

void YamahaLSX::write_state(light::LightState *state) {
    this->cancel_timeout("send_dimmer");

    if (!this->connected_) {
        static uint32_t last_log = 0;
        if (millis() - last_log > 5000) {
             ESP_LOGW(TAG, "Brak połączenia - próba łączenia...");
             last_log = millis();
             this->connect();
        }
        return;
    }

    bool is_on = state->current_values.is_on();
    float brightness = state->current_values.get_brightness();

    if (!is_on) {
        ESP_LOGI(TAG, "Komenda: OFF");
        send_packet(0x01, 0x00);
        
        this->set_timeout("force_off", 50, [=]() {
             if(global_yamaha_instance) global_yamaha_instance->send_packet(0x01, 0x00);
        });
        
    } else {
        int level = (int)(brightness * 5);
        if (level < 1) level = 1;
        if (level > 5) level = 5;

        ESP_LOGI(TAG, "Komenda: ON + DIMMER (Level %d/5)", level);
        
        send_packet(0x01, 0x01);
        
        this->set_timeout("send_dimmer", 250, [=]() {
             if(global_yamaha_instance && global_yamaha_instance->connected_) {
                 global_yamaha_instance->send_packet(0x04, (uint8_t)level);
             }
        });
    }
}

void YamahaLSX::dump_config() {
    ESP_LOGCONFIG(TAG, "Yamaha LSX Custom Light");
    ESP_LOGCONFIG(TAG, "  Status: %s", this->connected_ ? "POŁĄCZONY" : "ROZŁĄCZONY");
    ESP_LOGCONFIG(TAG, "  MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
                  this->peer_bd_addr_[0], this->peer_bd_addr_[1], this->peer_bd_addr_[2],
                  this->peer_bd_addr_[3], this->peer_bd_addr_[4], this->peer_bd_addr_[5]);
}

} // namespace yamaha_lsx
} // namespace esphome
