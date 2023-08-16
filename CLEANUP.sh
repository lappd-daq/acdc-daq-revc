#!/bin/bash
rm -r bin
rm -r build

find . -type f -name "Command" -delete
find . -type f -name "ConnectedBoards" -delete
find . -type f -name "TestConnection" -delete
find . -type f -name "SpeedTest" -delete