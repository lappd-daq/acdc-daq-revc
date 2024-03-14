#!/bin/bash

./Command_ETH 0x0 0x1 w
sleep 3
echo ""
./Command_ETH 0x100 0xffff0000 w
sleep 3
echo ""
./Command_ETH 0x2019 0x1 rs