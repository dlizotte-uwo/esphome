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

DEPENDENCIES = ["rp2040_touch", "rp2040", "rp2040_pio"]

CONF_RP2040_TOUCH_ID = "rp2040_touch_id"

CONF_PIO = "pio"

AUTO_LOAD = ["rp2040_pio"]

RP2040TouchBinarySensor = rp2040_touch_ns.class_(
    "RP2040TouchBinarySensor", binary_sensor.BinarySensor
)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(RP2040TouchBinarySensor).extend(
    {
        cv.GenerateID(CONF_RP2040_TOUCH_ID): cv.use_id(RP2040TouchComponent),
        cv.Required(CONF_PIN): cv.uint8_t,
        cv.Optional(CONF_THRESHOLD, default=0): cv.uint32_t,
        cv.Optional(CONF_PIO): cv.one_of(0, 1, int=True),
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
    if CONF_PIO in config:
        cg.add(var.set_pio(config[CONF_PIO]))

    cg.add(hub.register_touch_pad(var))
