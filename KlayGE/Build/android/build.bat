echo "Building KlayGE Core..."

cd Core
call build.bat
cd ..

echo "Building KlayGE OpenGLES2 plugin..."

cd Plugins/Render/OpenGLES2
call build.bat
cd ../../..

echo "Building KlayGE OCTree plugin..."

cd Plugins/Scene/OCTree
call build.bat
cd ../../..
