粘土游戏引擎(KlayGE) 2.1.3

Homepage: http://klayge.sourceforge.net
E-Mail:	  enginedev@163.com
OICQ:     33611210

欢迎使用粘土游戏引擎 2.1.3
包含的组件：KlayGE头文件，KlayGE源代码，KlayGE的VS.NET 2003工程文件

KlayGE是一个开放，跨平台，插件结构的游戏引擎。

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

KlayGE遵循LGPL协议，版权归龚敏敏所有。您可以修改它，但请告诉我您的修改。如果您再开发过程中发现Bug，请麻烦您告诉我。

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

编译KlayGE内核需要一些第三方库：Python、Boost，分别在http://www.python.org和http://boost.sourceforge.net下载
编译OggWav插件需要Vorbis SDK，在http://www.vorbis.com下载
编译OpenAL插件需要OpenAL SDK，在http://www.openal.org下载，并把它的Include目录下建一个AL目录，把.h拷进去
除了Python以外的几个SDK的精简版本都能在我的网站找到。


__________________________________________________________________



下步计划

内部支持骨骼动画
加强特效脚本化
加强OpenGL插件


__________________________________________________________________



发展历程

2.1.3
增加了以boost::tuple为参数的ScriptEngine::Call (2004.9.15)
修正了一些关于STL的问题 (2004.10.14)

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
		\Math
			数学库源文件
		\Net
			网络引擎的源文件
		\Render
			渲染系统内核的源文件
		\Res
			资源引擎的源文件
		\Script
			脚本引擎的源文件
		\Show
			播放引擎内核的源文件
		\Timer
			定时器的源文件

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
		\Render
			渲染系统插件的源文件
		\Show
			播放引擎插件的源文件

\RenderFX
	渲染特效脚本

\Tools
	工具文件

	\Bin
		编译生成的工具文件
	\Pkt
		文件打包工具源文件
	\KlayTXConvert
		把tga转换成KlayGE支持的.klaytx格式的工具源文件

\Lib
		存放编译后的静态连接库

	
__________________________________________________________________



开发计划
	在以后的版本中，将更好的支持控制台以及3D模型文件的读取

__________________________________________________________________


编译器支持

- 推荐使用Visual Studio.NET 2003 编译

- KlayGE是用Visual Studio.NET 2003 开发和编译的，还提供了Visual Studio.NET 2003 的 sln 文件。

- KlayGE从不能保证在 Visual Studio.NET 以前的版本中编译通过。

