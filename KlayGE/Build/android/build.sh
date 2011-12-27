#!/bin/bash

echo "Building KlayGE Core..."

cd Core
./build.sh
cd ..

echo "Building KlayGE OpenGLES2 plugin..."

cd Plugins/Render/OpenGLES2
./build.sh
cd ../../..

echo "Building KlayGE OCTree plugin..."

cd Plugins/Scene/OCTree
./build.sh
cd ../../..
