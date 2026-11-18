# Q2 Games: cross-builds inside DiamondBond's complete Rockbox runtime.
# The .rock format is not a standalone MIPS Linux executable.
ROCKBOX_ROOT ?= $(abspath ../q2-rockbox)
Q2_SYSROOT ?= $(HOME)/q2-rockbox-sysroot
Q2_BUILD_DIR ?= $(HOME)/q2-rockbox-build
JOBS ?= 4
CC ?= cc

.DEFAULT_GOAL := all
.PHONY: all check integrate configure build package clean

all: package

check:
	mkdir -p build
	$(CC) -O2 -Wall -Wextra -Werror -pedantic -std=c11 -Isrc -o build/catalog-test tests/catalog_test.c src/q2_games_catalog.c
	./build/catalog-test
	python3 -m unittest discover -s tests -v

integrate:
	@test -f "$(ROCKBOX_ROOT)/tools/configure" || { echo "Rockbox source not found: $(ROCKBOX_ROOT)"; exit 1; }
	python3 tools/install_into_rockbox.py "$(ROCKBOX_ROOT)"

configure: integrate
	@test -f "$(Q2_SYSROOT)/lib/libc.so.6" || { echo "Q2 sysroot missing. Run mksysroot.sh with stock V1.32 first: $(Q2_SYSROOT)"; exit 1; }
	mkdir -p "$(Q2_BUILD_DIR)"
	@if test ! -f "$(Q2_BUILD_DIR)/Makefile"; then \
	  cd "$(Q2_BUILD_DIR)" && Q2_SYSROOT="$(Q2_SYSROOT)" "$(ROCKBOX_ROOT)/tools/configure" --target=shanlingq2 --type=normal; \
	fi

build: configure
	$(MAKE) -C "$(Q2_BUILD_DIR)" -j"$(JOBS)" Q2_SYSROOT="$(Q2_SYSROOT)"

package: build
	$(MAKE) -C "$(Q2_BUILD_DIR)" zip Q2_SYSROOT="$(Q2_SYSROOT)"
	@echo "Rockbox package: $(Q2_BUILD_DIR)/rockbox.zip"
	@find "$(Q2_BUILD_DIR)" -name q2_games.rock -print
	@test -f "$(Q2_BUILD_DIR)/rockbox.zip" || { echo "Expected rockbox.zip is missing"; exit 1; }

clean:
	rm -rf build
	@echo "External Rockbox build left untouched: $(Q2_BUILD_DIR)"
