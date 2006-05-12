GLLoader v2.4.0

The GLLoader is an OpenGL extension loading library. It supports OpenGL core 1.0 to 2.0, as well as WGL, GLX, and other GL extensions. There is a automatic code generater. All the things you want to do is to write a xml script if you have to support new extensions.
The GLLoader's primary selling point is that it offers a initializing path of an extension. So if an extension has promoted into the core, the loader will try to load it from the core. If failed, try the extension one.
The GLLoader is licensed under GPL. See gpl.txt.

2.4.0
Support more extensions
x64 is supported

2.3.0 (2005.7.23)
Support promote feature strings
Add supports for codeblocks

2.2.0 (2005.7.2)
Switch to GPL.

2.1.0 (2005.6.2)
OpenGL 1.1 is changed to static link.

2.0.1 (2005.3.30)
Use functions to get the support infomation.

2.0.0 (2005.3.1)
All function's names are lower case.

1.5.0 (2005.2.14)
Support self init. GLLoader_Init is not required.

1.1.4 (2005.1.31)
Added some extensions

1.1.3 (2004.11.12)
Added some extensions

1.1.2 (2004.10.17)
Bug fixed

1.1.1 (2004.10.9)
Fixed bug in WGL

1.1.0 (2004.10.9)
Added the support of WGL and GLX

1.0.3 (2004.9.30)
Added some extensions
Bug fixed

1.0.2 (2004.9.26)
Added the check to an extension which don't contain any function

1.0.1 (2004.9.24)
Only one header file and one source file is generated

1.0.0 (2004.9.24)
Initial release

0.9.99
Added OpenGL 2.0 support

0.9.9
Added automatic code generater


Minmin Gong
http://cosoft.org.cn/projects/glloader
