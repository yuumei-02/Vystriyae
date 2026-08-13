# Copyright (c) 2026 yuumei-02. All Rights Reserved.
# See the LICENSE file for more information.

.ONESHELL:

compiler := gcc
flags := -Wall -Wextra -pedantic -std=c23

build: setup vystriyae example

vystriyae:
	cd ./vystriyae
	$(compiler) $(flags) -c *.c
	ar rcs ../build/lib/libvystriyae.a *.o
	rm *.o
	cp ./core.h          ../build/lib/vystriyae/core.h
	cp ./wm.h            ../build/lib/vystriyae/wm.h
	cp ./colors.h        ../build/lib/vystriyae/colors.h
	cp ./unicode.h       ../build/lib/vystriyae/unicode.h
	cp ./notifications.h ../build/lib/vystriyae/notifications.h

example:
	cd ./example
	$(compiler) $(flags) -I../build/lib -L../build/lib *.c -o ../build/bin/example -lvystriyae -lm -lmcu-debug

setup:
	mkdir -p ./build/bin
	mkdir -p ./build/lib/vystriyae

