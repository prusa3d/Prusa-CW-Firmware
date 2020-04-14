#!/bin/bash

BUILD_ENV="1.0.1"
ENV_DIR="build/env"
SCRIPT_PATH="$( cd "$(dirname "$0")" ; pwd -P )"

if [ ! -d "$ENV_DIR" ]; then
	mkdir -p "$ENV_DIR" || exit 1
    wget "https://github.com/prusa3d/MM-build-env/releases/download/$BUILD_ENV/MM-build-env-Linux64-$BUILD_ENV.zip" || exit 2
    unzip -q "MM-build-env-Linux64-$BUILD_ENV.zip" -d "$ENV_DIR" || exit 3
fi

export PATH=$SCRIPT_PATH/$ENV_DIR/avr/bin:$PATH

make dist || exit 4
