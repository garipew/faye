#!/usr/bin/env bash

if [[ ! -d bin ]]; then
	mkdir bin
fi

make build
