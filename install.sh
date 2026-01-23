#!/bin/bash
set -xe

TARGET_DIR="/home/$USER/.nemi"

mkdir -p $TARGET_DIR
rsync -av ./configs $TARGET_DIR
rsync -av ./scripts $TARGET_DIR
rsync -av ./fonts $TARGET_DIR
rsync -v libnemi.so $TARGET_DIR
