#!/bin/bash
make -C ./src || exit -1
./src/tftpd 2000 data
