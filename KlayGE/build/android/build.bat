echo "Building KlayGE Core..."

cd Core
call build.bat
cd ..

echo "Building KlayGE OpenGLES plugin..."

cd Plugins/Render/OpenGLES
call build.bat
cd ../../..

echo "Building KlayGE OCTree plugin..."

cd Plugins/Scene/OCTree
call build.bat
cd ../../..

echo "Building KlayGE Python plugin..."

cd Plugins/Script/Python
call build.bat
cd ../../..
