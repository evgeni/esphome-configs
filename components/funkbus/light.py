import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import light, cc1101
from esphome import pins

DEPENDENCIES = ["cc1101"]

funkbus_ns = cg.esphome_ns.namespace("funkbus")
FunkbusLightOutput = funkbus_ns.class_("FunkbusLightOutput", cg.Component, light.LightOutput)

CONF_FUNKBUS_SERIAL = 'serial'
CONF_FUNKBUS_SWITCH = 'switch'
CONF_FUNKBUS_GROUP = 'group'
CONF_FUNKBUS_PIN = 'pin'
CONF_FUNKBUS_CC1101 = 'cc1101'

CONFIG_SCHEMA = light.light_schema(FunkbusLightOutput, light.LightType.BINARY).extend({
  cv.Required(CONF_FUNKBUS_PIN): pins.internal_gpio_output_pin_schema,
  cv.Required(CONF_FUNKBUS_CC1101): cv.use_id(cc1101.CC1101Component),
  cv.Required(CONF_FUNKBUS_SERIAL): cv.int_,
  cv.Optional(CONF_FUNKBUS_SWITCH, default=0): cv.int_,
  cv.Optional(CONF_FUNKBUS_GROUP, default=0): cv.int_,
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await light.new_light(config)
    await cg.register_component(var, config)

    pin = await cg.gpio_pin_expression(config[CONF_FUNKBUS_PIN])
    cg.add(var.set_pin(pin))

    cc1101 = await cg.get_variable(config[CONF_FUNKBUS_CC1101])
    cg.add(var.set_cc1101(cc1101))

    cg.add(var.set_serial(config[CONF_FUNKBUS_SERIAL]))
    cg.add(var.set_switch(config[CONF_FUNKBUS_SWITCH]))
    cg.add(var.set_group(config[CONF_FUNKBUS_GROUP]))
