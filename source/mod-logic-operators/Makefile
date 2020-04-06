#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

include dpf/Makefile.base.mk

all: plugins gen

# --------------------------------------------------------------

libs:
	$(MAKE) -C common

dgl:
ifeq ($(HAVE_DGL),true)
	$(MAKE) -C dpf/dgl
endif

plugins:
	$(MAKE) all -C plugins/logic-operators

ifneq ($(CROSS_COMPILING),true)
gen: plugins dpf/utils/lv2_ttl_generator
	#@$(CURDIR)/dpf/utils/generate-ttl.sh
	cp -r plugins/logic-operators/lv2-data/* bin/logic-operators.lv2/
ifeq ($(MACOS),true)
	@$(CURDIR)/dpf/utils/generate-vst-bundles.sh
endif

dpf/utils/lv2_ttl_generator:
	$(MAKE) -C dpf/utils/lv2-ttl-generator
else
gen:
endif

# --------------------------------------------------------------
install:
	cp -r bin/*.lv2/ /usr/lib/lv2
	cp bin/*.so /usr/lib/vst

clean:
	#$(MAKE) clean -C dpf/dgl
	$(MAKE) clean -C dpf/utils/lv2-ttl-generator
	$(MAKE) clean -C plugins/logic-operators
	rm -rf bin build

# --------------------------------------------------------------

.PHONY: plugins
