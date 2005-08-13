粘土游戏引擎(KlayGE) 2.8.0

Homepage: http://klayge.sourceforge.net
E-Mail:	  enginedev@163.com
OICQ:     33611210

欢迎使用粘土游戏引擎 2.8.0
包含的组件：KlayGE头文件，KlayGE源代码，KlayGE的VS.NET 2003工程文件

KlayGE是一个开放源代码的，跨平台的，基于插件结构的游戏引擎。

__________________________________________________________________


目录

- 使用协议
- 安装
- 下步计划
- 发展历程
- 目录结构
- 开发计划
- 编译器支持


__________________________________________________________________

使用协议

KlayGE版权归龚敏敏所有，从2.7.0开始遵循GPL协议。您任意地使用它，
但必须保证使用KlayGE的项目也遵循GPL协议。如果您再开发过程中发现Bug，
请麻烦您告诉我。

__________________________________________________________________



安装

1. 把KlayGE.zip解压缩到一个目录，比如说 F:\KlayGE

2. 设置路径。这里拿VS.NET 2003举例，其他环境请参考它们的帮助文件。
	2.1 打开“工具”->“选项”->“项目”，在“显示以下内容的目录”下拉框中选择“包含文件”，
		在下面添加F:\KlayGE\Core\Include、F:\KlayGE\AppLayer\Include和F:\KlayGE\Plugins\Include
	2.2 在“显示以下内容的目录”下拉框中选择"库文件"，
		在下面添加F:\KlayGE\Lib
	2.3 点“确定”按钮，建议关闭VS.NET 2003 IDE以保存设置

__________________________________________________________________



编译

编译KlayGE内核需要一些第三方库：Python 2.4.1、Boost 1.32.0、FreeType 2.1.10，分别在http://www.python.org、http://boost.sourceforge.net和http://freetype.sourceforge.net下载
编译DX相关插件需要DirectX 9.0 SDK June 2005，在http://msdn.microsoft.com/directx下载
编译OggWav插件需要Vorbis SDK，在http://www.vorbis.com下载
编译OpenAL插件需要OpenAL SDK，在http://www.openal.org下载


__________________________________________________________________



下步计划

内部支持骨骼动画
加强特效脚本化
加强OpenGL插件


__________________________________________________________________



发展历程

2.8.1 (2005.8.14)
增加了RenderDeviceCaps
增加了OpenGL兼容性检测工具
增加了Sampler
重写了RenderEffect
只支持OpenGL 1.5及以上
简化了RenderEngine
DInputDevice改为多继承结构
增加了RenderToVertexStream

2.7.1 (2005.7.15)
美化了字体显示效果
LoadKMesh可以使用自定义类
增加了RenderableHelper基类
RenderEngine::ViewMatrix和ProjectionMatrix改为const

2.7.0 (2005.7.2)
改成GPL协议
增加了KMesh
去掉了RenderEngine::TextureCoordSet
AddressingMode, Filtering和Anisotropy从RenderEngine移到Texture中
Texture增加了MaxMipLevel和MipMapLodBias
App3D增加了Quit
支持OpenGL 1.5

2.6.0 (2005.6.2)
修正了SceneManager类CanBeCulled的bug
增加了half类型
D3D9Texture增加了对surface的检查
增加了RenderableSkyBox
支持HDR

2.5.0 (2005.5.2)
增加了3DSMax导出插件
可以同时使用多个输入动作表
MathLib改为使用返回值返回结果
改进了CameraController
增加了RenderableHelper
视锥裁减改为使用LUT实现

2.4.0 (2005.3.28)
支持纹理压缩
增加了D3D9Resource
支持DDS格式的载入和保存
八叉树改为线性实现
支持深度纹理

2.3.0 (2005.3.2)
使用FreeType实现Font
修正了几个内存泄漏
增加了对浮点纹理的支持
增加了CopyToMemory
增加了视锥裁减插件
D3D9插件增加了OnLostDevice和OnResetDevice

2.2.0 (2004.11.20)
修正了DSound插件的音量问题
去掉了ManagerBase、FileSystem、Crc32、alloc、Timer、Random
使用boost 1.32.0
增加了Trace

2.1.3 (2004.10.19)
增加了以boost::tuple为参数的ScriptEngine::Call
修正了一些关于STL的问题
修正了Pkt的CRC错误
去掉了MemoryLib

2.1.2 (2004.9.5)
增加了BoneMesh
增加了RenderEffectParameter
支持DX 9.0c SDK
修正了DiskFile::Seek的Bug
把const T&改为更规则的T const &
InputEngine改用Bridge实现

2.1.1 (2004.5.25)
大量使用boost
数学库都采用泛型实现
增加了Parallax的Demo
增加了ResLocator

2.1.0 (2004.4.20)
增强了OpenGL
去掉了汇编代码
直接支持单独的Shader
简化了打包文件目录表的表示法

2.0.5 (2004.4.13)
增强了OpenGL
修正了WindowMovedOrResized的bug
MemFile改用stringstream实现
提高了代码安全性

2.0.4 (2004.4.7)
修正了OALMusicBuffer无法loop的bug
DSMusicBuffer改用timeSetEvent实现
改进了VertexBuffer
Audio部分增加了NullObject
增加了Demo

2.0.3 (2004.3.12)
简化了VertexBuffer
去掉了软件顶点混合

2.0.2 (2003.12.28)
改进了渲染队列
使用Python做脚本语言
DiskFile改为用标准C++流实现

2.0.1 (2003.10.17)
去掉了DX8的支持 
增加了一些工具

2.0.0 (2003.10.1)
正式版发布

2.0.0 Beta (2003.9.5)
初次发布


__________________________________________________________________


目录结构：


下面是关于完整的KlayGE目录的主要描述。

\Core
	内核文件

	\Include
		内核头文件

	\Src
		内核源文件

		\Audio
			音频引擎内核的源文件
		\Input
			输入引擎内核的源文件
		\Kernel
			核心源文件
		\Net
			网络引擎的源文件
		\Render
			渲染系统内核的源文件
		\Script
			脚本引擎的源文件
		\Show
			播放引擎内核的源文件

\Demos
	演示

\Doc
	文档

\AppLayer
	应用程序层

	\Include
		应用程序层的头文件

	\Src
		应用程序层的源文件

\Plugins
	插件文件

	\Include
		插件头文件

	\Src
		插件源代码

		\Audio
			音频引擎插件的源文件
		\AudioDataSource
			音频数据源插件的源文件
		\Input
			输入引擎插件的源文件
		\PackSystem
			文件打包系统的源文件
		\Render
			渲染系统插件的源文件
		\Scene
			场景管理器插件的源文件
		\Show
			播放引擎插件的源文件
		\Terrain
			地形插件的源文件

\RenderFX
	渲染特效脚本

\Tools
	工具文件

	\Bin
		编译生成的工具文件
	\DistanceMapCreator
		从height map建立distance map的工具
	\MeshML2KMesh
		从.MeshML编译为.KMesh的工具
	\NormalizerCubeMap
		生成归一化Cube map的工具
	\NormalMapGen
		从height map建立normal map的工具
	\Pkt
		文件打包工具

\Lib
		存放编译后的静态连接库



__________________________________________________________________


编译器支持

- 推荐使用Visual Studio.NET 2003编译
- KlayGE是用Visual Studio.NET 2003开发和编译的，还提供了Visual Studio.NET 2003的sln文件。
- KlayGE不保证能在Visual Studio.NET以前的版本中编译通过。

