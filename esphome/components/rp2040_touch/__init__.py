import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import rp2040
from esphome.const import (
    CONF_ID,
    CONF_SETUP_MODE,
)

AUTO_LOAD = ["binary_sensor"]
DEPENDENCIES = ["rp2040", "rp2040_pio"]
CONF_PIO = "pio"

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

def generate_assembly_code():
    """
    Generate assembly code for pio
    """
    const_csdk_code = f"""% c-sdk {{
#include "hardware/clocks.h"

static inline void rp2040_pio_touch_init(PIO pio, uint sm, uint offset, uint pin) {{
    gpio_pull_up(pin);
    pio_gpio_init(pio, pin);
    //Set pin to input
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, false);
    pio_sm_config c = rp2040_pio_touch_program_get_default_config(offset);
    sm_config_set_jmp_pin(&c, pin);
    sm_config_set_set_pins(&c, pin, 1);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX); // 8 word RX FIFO
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}}
%}}
"""
    
    assembly_template = """
.program rp2040_pio_touch
    mov isr, null
    
    ; set y to the sample period count, by shifting in a 1 and a bunch of 0s
    set y, 1
    in y, 1
    in NULL, 20
    mov y, isr
    
    ; clear the counter
    mov x, !NULL
    
resample:
    ; set pin to input...
    set pindirs, 0
busy:
    ; ...and wait for it to pull high via pullup
    jmp pin, high
    jmp y--, busy
    ; if we run out of time
    jmp done

high:
    ; Set up for next measurement
    ; set pin to output, low
    set pindirs, 1
    set pins, 0
    
    ; wait for discharge, counting the time spent outside of the busy loop
    jmp y--, dec1
    jmp done
dec1:
    jmp y--, dec2
    jmp done
dec2:
    jmp y--, dec3
    jmp done
dec3:
    jmp y--, dec4
    jmp done
dec4:
    jmp y--, dec5
    jmp done
dec5:

    ; Count this cycle and repeat
    jmp x--, resample

done:
    ; time's up - push the count
    mov isr, x
    push block

"""

    return assembly_template + const_csdk_code


async def to_code(config):
    touch = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(touch, config)
    cg.add(touch.set_setup_mode(config[CONF_SETUP_MODE]))
    cg.add(touch.set_program(cg.RawExpression("&rp2040_pio_touch_program")))
    cg.add(
        touch.set_init_function(
            cg.RawExpression("rp2040_pio_touch_init")
        )
    )

    key = "pio_touch"
    rp2040.add_pio_file(__name__,key,generate_assembly_code())
