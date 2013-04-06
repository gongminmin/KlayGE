#!/bin/bash

echo "Building KlayGE Core..."

cd Core
./build.sh
cd ..

echo "Building KlayGE OpenGLES plugin..."

cd Plugins/Render/OpenGLES
./build.sh
cd ../../..

echo "Building KlayGE OCTree plugin..."

cd Plugins/Scene/OCTree
./build.sh
cd ../../..

echo "Building KlayGE Script plugin..."

cd Plugins/Script/Python
./build.sh
cd ../../..

echo "Building KlayGE MsgInput plugin..."

cd Plugins/Input/MsgInput
./build.sh
cd ../../..
