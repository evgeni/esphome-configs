import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import cover, cc1101
from esphome import pins

DEPENDENCIES = ["esp32", "cc1101"]

somfy_ns = cg.esphome_ns.namespace("somfy")
SomfyCover = somfy_ns.class_("SomfyCover", cover.Cover, cg.Component)

CONF_SOMFY_REMOTE_ADDRESS = "remote_address"
CONF_SOMFY_PIN = "pin"
CONF_SOMFY_STORAGE_KEY = "storage_key"
CONF_SOMFY_STORAGE_NAMESPACE = "storage_namespace"
CONF_SOMFY_REPEAT = "repeat"
CONF_SOMFY_CC1101 = "cc1101"

CONFIG_SCHEMA = cover.cover_schema(SomfyCover).extend(
    {
        cv.Required(CONF_SOMFY_PIN): pins.internal_gpio_output_pin_schema,
        cv.Required(CONF_SOMFY_REMOTE_ADDRESS): cv.int_,
        cv.Required(CONF_SOMFY_STORAGE_KEY): cv.All(cv.string, cv.Length(max=15)),
        cv.Optional(CONF_SOMFY_STORAGE_NAMESPACE, default="somfy"): cv.All(
            cv.string, cv.Length(max=15)
        ),
        cv.Optional(CONF_SOMFY_REPEAT, default=4): cv.int_range(min=1, max=16),
        cv.Required(CONF_SOMFY_CC1101): cv.use_id(cc1101.CC1101Component),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await cover.new_cover(config)
    await cg.register_component(var, config)

    pin = await cg.gpio_pin_expression(config[CONF_SOMFY_PIN])
    cg.add(var.set_pin(pin))

    cc1101 = await cg.get_variable(config[CONF_SOMFY_CC1101])
    cg.add(var.set_cc1101(cc1101))

    cg.add(var.set_remote_address(config[CONF_SOMFY_REMOTE_ADDRESS]))
    cg.add(var.set_storage_key(config[CONF_SOMFY_STORAGE_KEY]))
    cg.add(var.set_storage_namespace(config[CONF_SOMFY_STORAGE_NAMESPACE]))
    cg.add(var.set_repeat(config[CONF_SOMFY_REPEAT]))
