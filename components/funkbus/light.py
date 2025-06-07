import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import light
from esphome import pins

funkbus_ns = cg.esphome_ns.namespace("funkbus")
FunkbusLightOutput = funkbus_ns.class_("FunkbusLightOutput", cg.Component, light.LightOutput)

CONF_FUNKBUS_SERIAL = 'serial'
CONF_FUNKBUS_SWITCH = 'switch'
CONF_FUNKBUS_GROUP = 'group'
CONF_FUNKBUS_PIN = 'pin'

CONFIG_SCHEMA = light.light_schema(FunkbusLightOutput, light.LightType.BINARY).extend({
  cv.Required(CONF_FUNKBUS_PIN): pins.internal_gpio_output_pin_schema,
  cv.Required(CONF_FUNKBUS_SERIAL): cv.int_,
  cv.Optional(CONF_FUNKBUS_SWITCH, default=0): cv.int_,
  cv.Optional(CONF_FUNKBUS_GROUP, default=0): cv.int_,
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await light.new_light(config)
    await cg.register_component(var, config)

    pin = await cg.gpio_pin_expression(config[CONF_FUNKBUS_PIN])
    cg.add(var.set_pin(pin))
    cg.add(var.set_serial(config[CONF_FUNKBUS_SERIAL]))
    cg.add(var.set_switch(config[CONF_FUNKBUS_SWITCH]))
    cg.add(var.set_group(config[CONF_FUNKBUS_GROUP]))
