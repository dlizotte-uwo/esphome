import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_SETUP_MODE,
)

AUTO_LOAD = ["binary_sensor"]
DEPENDENCIES = ["rp2040"]

rp2040_touch_ns = cg.esphome_ns.namespace("rp2040_touch")
RP2040TouchComponent = rp2040_touch_ns.class_("RP2040TouchComponent", cg.Component)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(RP2040TouchComponent),
            cv.Optional(CONF_SETUP_MODE, default=False): cv.boolean,
            # common options
        }
    )
)

async def to_code(config):
    touch = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(touch, config)
    cg.add(touch.set_setup_mode(config[CONF_SETUP_MODE]))
    