# Copyright (c) 2026 yuumei-02. All Rights Reserved.
# See the LICENSE file for more information.

compiler := gcc
flags := -Wall -Wextra -pedantic -std=c23

all: build
build: setup vystriyae example

vystriyae: setup ./vystriyae
	$(compiler) $(flags) -c $(wildcard ./vystriyae/*.c)
	ar rcs ./build/lib/libvystriyae.a *.o
	rm *.o
	cp ./vystriyae/core.h          ./build/lib/vystriyae/core.h
	cp ./vystriyae/wm.h            ./build/lib/vystriyae/wm.h
	cp ./vystriyae/colors.h        ./build/lib/vystriyae/colors.h
	cp ./vystriyae/unicode.h       ./build/lib/vystriyae/unicode.h
	cp ./vystriyae/notifications.h ./build/lib/vystriyae/notifications.h

example: setup ./example
	$(compiler) $(flags) -I./build/lib -L./build/lib $(wildcard ./example/*.c) -o ./build/bin/example -lvystriyae -lm -lmcu-debug

setup:
	mkdir -p ./build/bin
	mkdir -p ./build/lib/vystriyae

