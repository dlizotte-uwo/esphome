from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    CONF_PIN,
    CONF_THRESHOLD,
    CONF_ID,
)
from . import rp2040_touch_ns, RP2040TouchComponent

DEPENDENCIES = ["rp2040_touch", "rp2040"]

CONF_RP2040_TOUCH_ID = "rp2040_touch_id"

RP2040TouchBinarySensor = rp2040_touch_ns.class_(
    "RP2040TouchBinarySensor", binary_sensor.BinarySensor
)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(RP2040TouchBinarySensor).extend(
    {
        cv.GenerateID(CONF_RP2040_TOUCH_ID): cv.use_id(RP2040TouchComponent),
        cv.Required(CONF_PIN): cv.uint8_t,
        cv.Required(CONF_THRESHOLD): cv.uint32_t,
    }
)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_RP2040_TOUCH_ID])
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_PIN],
        config[CONF_THRESHOLD],
    )
    await binary_sensor.register_binary_sensor(var, config)
    cg.add(hub.register_touch_pad(var))
