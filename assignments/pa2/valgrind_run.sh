#!/bin/bash

if [ -z "$1" ]
  then
    port=2000
  else
    port=$1
fi

env G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --leak-check=full --show-leak-kinds=all ./src/httpd $port
