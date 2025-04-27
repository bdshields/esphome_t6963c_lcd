from esphome import pins
import esphome.codegen as cg
from esphome.components import display
import esphome.config_validation as cv

from esphome.const import (
    CONF_ENABLE_PIN,
    CONF_RESET_PIN,
    CONF_DATA_PINS,
    CONF_WIDTH,
    CONF_HEIGHT,
    CONF_ID,
    CONF_LAMBDA
)

CONF_RD_PIN = "rd_pin"
CONF_WR_PIN = "wr_pin"
CONF_CD_PIN = "cd_pin"

AUTO_LOAD = ["display"]

t6963c_ns = cg.esphome_ns.namespace("t6963c")
T6963C = t6963c_ns.class_(
    "T6963C", cg.PollingComponent, display.DisplayBuffer
)
T6963CRef = T6963C.operator("ref")


CONFIG_SCHEMA = (
    display.FULL_DISPLAY_SCHEMA.extend(
      {
        cv.GenerateID(): cv.declare_id(T6963C),
        cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_RD_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_WR_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_CD_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_DATA_PINS): cv.All(
            [pins.gpio_output_pin_schema], cv.Length(min=8,max=8)
        ),

        cv.Optional(CONF_WIDTH, default=240): cv.int_range(min=0, max=256),
        cv.Optional(CONF_HEIGHT, default=128): cv.int_range(min=0, max=128),
      }
    ).extend(cv.polling_component_schema("1s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    pins_ = []
    for conf in config[CONF_DATA_PINS]:
        pins_.append(await cg.gpio_pin_expression(conf))
    cg.add(var.set_data_pins(*pins_))

    reset = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
    cg.add(var.set_reset_pin(reset))

    enable = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
    cg.add(var.set_enable_pin(enable))

    wr = await cg.gpio_pin_expression(config[CONF_WR_PIN])
    cg.add(var.set_wr_pin(wr))

    cd = await cg.gpio_pin_expression(config[CONF_CD_PIN])
    cg.add(var.set_cd_pin(cd))

    if CONF_RD_PIN in config:
        rd = await cg.gpio_pin_expression(config[CONF_RD_PIN])
        cg.add(var.set_rd_pin(rd))


    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
    cg.add(var.set_width(config[CONF_WIDTH]))
    cg.add(var.set_height(config[CONF_HEIGHT]))

    await display.register_display(var, config)
