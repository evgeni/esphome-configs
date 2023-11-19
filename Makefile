TARGETS:=$(shell git ls-files '[^_]*.yaml')

targets:
	@echo $(TARGETS)
