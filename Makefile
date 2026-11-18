Q2_CC ?= clang
Q2_LD ?= ld.lld
CC ?= cc
Q2_FLAGS = --target=mipsel-linux-gnu -march=mips32r2 -mabi=32 -mfp64 -mnan=2008 -mabs=2008 -mno-abicalls -fno-pic -G0 -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables
CFLAGS ?= -O2 -Wall -Wextra -Werror -pedantic
SOURCES = src/native_app.c src/native_render.c src/extra_games.c src/puzzle_games.c src/reversi.c src/small_games.c src/native_platform.c
OBJECTS = $(patsubst src/%.c,build/q2/%.o,$(SOURCES))
.DEFAULT_GOAL := all
.PHONY: all check clean
all: dist/q2-games.zip
build/q2/%.o: src/%.c src/native_app.h
	mkdir -p $(@D)
	$(Q2_CC) $(Q2_FLAGS) -Oz -Wall -Wextra -Werror -std=c11 -Isrc -c $< -o $@
dist/.games/q2-games: $(OBJECTS)
	mkdir -p $(@D)
	$(Q2_LD) -m elf32ltsmip -static -e __start -s -z noexecstack -o $@ $^
dist/.games/q2app: q2app
	mkdir -p $(@D)
	tr -d '\r' < $< > $@
dist/.games/LICENSE.txt: LICENSE
	mkdir -p $(@D)
	cp $< $@
dist/q2-games.zip: dist/.games/q2-games dist/.games/q2app dist/.games/LICENSE.txt
	cd dist && rm -f q2-games.zip && zip -q q2-games.zip .games/q2-games .games/q2app .games/LICENSE.txt
check:
	mkdir -p build
	$(CC) $(CFLAGS) -std=c11 -Isrc -o build/native-test tests/native_test.c src/native_app.c src/native_render.c src/extra_games.c src/puzzle_games.c src/reversi.c src/small_games.c
	./build/native-test
clean:
	rm -rf build dist
