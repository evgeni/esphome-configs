TARGETS:=$(shell git ls-files '[^_]*.yaml')
SHELLY:=$(filter shelly%,$(TARGETS))

targets:
	@echo $(TARGETS)

compile-%: %
	esphome compile $*

deploy-%: %
	esphome run --no-logs $*

compile-all: $(addprefix compile-,$(TARGETS))

deploy-all: $(addprefix deploy-,$(TARGETS))

compile-shelly: $(addprefix compile-,$(SHELLY))

deploy-shelly: $(addprefix deploy-,$(SHELLY))
