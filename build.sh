#!/bin/bash

BUILD_ENV="1.0.1"
ENV_DIR="build/env"
SYSTEM="Linux64"
SCRIPT_PATH="$( cd "$(dirname "$0")" ; pwd -P )"
REDOWNLOAD="false"

usage() {
	echo "usage: build.sh --download --system WIN|LINUX"
}

while [ "$1" != "" ]; do
    case $1 in
        --platform)           
			shift
            if [ "$1" == "WIN" ]; then
            	SYSTEM="Win64"
            fi
            if [ "$1" == "LINUX" ]; then
            	SYSTEM="Linux64"
            fi
            ;;

        --download )    
			REDOWNLOAD="true"
            ;;

        -h | --help )           
			usage
            exit
            ;;
    esac
    shift
done

if [ "$REDOWNLOAD" == "true" ]; then
	rm -rf "$ENV_DIR" || exit 6
fi

if [ ! -d "$ENV_DIR" ]; then
	mkdir -p "$ENV_DIR" || exit 1
    wget "https://github.com/prusa3d/MM-build-env/releases/download/$BUILD_ENV/MM-build-env-$SYSTEM-$BUILD_ENV.zip" || exit 2
    unzip "MM-build-env-$SYSTEM-$BUILD_ENV.zip" -d "$ENV_DIR" || exit 3
    rm "MM-build-env-$SYSTEM-$BUILD_ENV.zip"
fi

export PATH=$SCRIPT_PATH/$ENV_DIR/avr/bin:$PATH

for LG in "en" "cs" "de" "es" "fr" "it" "pl" ; do
	make clean || exit 4
	make "LANG=$LG" dist || exit 5
done