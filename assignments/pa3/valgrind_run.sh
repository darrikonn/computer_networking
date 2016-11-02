#!/bin/bash

if [ -z "$1" ]
  then
    port=2000
  else
    port=$1
fi

make -C src/.
env G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./src/chatd $port
# --show-leak-kinds=all
