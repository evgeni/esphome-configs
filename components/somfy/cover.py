import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import cover
from esphome.const import CONF_ID
from esphome import pins

somfy_ns = cg.esphome_ns.namespace("somfy")
SomfyCover = somfy_ns.class_("SomfyCover", cover.Cover, cg.Component)

CONF_SOMFY_REMOTE_ADDRESS = "remote_address"
CONF_SOMFY_PIN = "pin"
CONF_SOMFY_STORAGE_KEY = "storage_key"
CONF_SOMFY_STORAGE_NAMESPACE = "storage_namespace"

CONFIG_SCHEMA = cover.COVER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(SomfyCover),
        cv.Required(CONF_SOMFY_PIN): pins.internal_gpio_output_pin_schema,
        cv.Required(CONF_SOMFY_REMOTE_ADDRESS): cv.int_,
        cv.Required(CONF_SOMFY_STORAGE_KEY): cv.All(cv.string, cv.Length(max=15)),
        cv.Optional(CONF_SOMFY_STORAGE_NAMESPACE, default="somfy"): cv.All(
            cv.string, cv.Length(max=15)
        ),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)

    pin = await cg.gpio_pin_expression(config[CONF_SOMFY_PIN])
    cg.add(var.set_pin(pin))
    cg.add(var.set_remote_address(config[CONF_SOMFY_REMOTE_ADDRESS]))
    cg.add(var.set_storage_key(config[CONF_SOMFY_STORAGE_KEY]))
    cg.add(var.set_storage_namespace(config[CONF_SOMFY_STORAGE_NAMESPACE]))
