#!/usr/bin/env bash

function build() {
  CFLAGS="$1" pebble build
  pebble wipe
}

function screenshot() {
  pebble install --emulator "$1"

  if [[ "$3" == "true" ]]; then
    pebble emu-set-timeline-quick-view --emulator "$1" on
  fi

  sleep 2

  pebble screenshot --emulator "$1" ./img/screenshots/"$2".png

  if [[ "$3" == "true" ]]; then
    pebble emu-set-timeline-quick-view --emulator "$1" off
  fi

  pebble kill
}

build "-DHOUR=10 -DMINUTE=10"
screenshot diorite diorite
screenshot basalt basalt
screenshot chalk chalk
screenshot emery emery
