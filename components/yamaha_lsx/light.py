import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, esp32
from esphome.const import CONF_OUTPUT_ID

CONF_MAC_ADDRESS = "mac_address"

yamaha_lsx_ns = cg.esphome_ns.namespace('yamaha_lsx')
YamahaLSX = yamaha_lsx_ns.class_('YamahaLSX', cg.Component, light.LightOutput)

CONFIG_SCHEMA = light.BRIGHTNESS_ONLY_LIGHT_SCHEMA.extend({
    cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(YamahaLSX),
    cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    esp32.include_builtin_idf_component("bt")

    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])

    await cg.register_component(var, config)
    await light.register_light(var, config)

    mac = config[CONF_MAC_ADDRESS].parts
    cg.add(var.set_mac_address([mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]]))
