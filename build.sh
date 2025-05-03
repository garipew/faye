#!/usr/bin/env bash

mkdir -p bin
mkdir -p $HOME/bin

make clean
make build

if [[ ! -e $HOME/bin/faye ]]; then
	ln -sf $PWD/bin/faye $HOME/bin/faye
fi
