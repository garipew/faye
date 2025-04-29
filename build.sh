#!/usr/bin/env bash

if [[ ! -d bin ]]; then
	mkdir bin
fi

make clean
make build
