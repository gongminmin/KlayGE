粘土游戏引擎(KlayGE) 2.0.4

Homepage: http://home.g365.net/enginedev
E-Mail:	  enginedev@163.com
OICQ:     33611210

欢迎使用粘土游戏引擎 2.0.4
包含的组件：KlayGE头文件，KlayGE源代码，KlayGE的VS.NET 2003工程文件

KlayGE是一个插件结构的游戏引擎。

__________________________________________________________________


目录

- 使用协议
- 安装
- 发展历程
- 目录结构
- 开发计划
- 编译器支持


__________________________________________________________________

使用协议

KlayGE遵循LGPL协议，版权归龚敏敏所有。您可以修改它，但请把修改后的文件Mail给我。如果您再开发过程中发现Bug，请麻烦您告诉我。

__________________________________________________________________



安装

1. 把KlayGE.zip解压缩到一个目录，比如说 F:\KlayGE

2. 设置路径。这里拿VS.NET 2003举例，其他环境请参考它们的帮助文件。
	2.1 打开“工具”->“选项”->“项目”，在“显示以下内容的目录”下拉框中选择“包含文件”，
		在下面添加 F:\KlayGE\Core\Include 和 F:\KlayGE\Plugins\Include
	2.2 在“显示以下内容的目录”下拉框中选择"库文件"，
		在下面添加 F:\KlayGE\Core\Lib 和 F:\KlayGE\Plugins\Lib
	2.3 点“确定”按钮，建议关闭VS.NET 2003 IDE以保存设置

__________________________________________________________________



编译

编译KlayGE内核需要一些第三方库：PThread、Python，分别在http://sources.redhat.com/pthreads-win32和http://www.python.org下载
编译OggWav插件需要Vorbis SDK，在http://www.vorbis.com下载
编译OpenAL插件需要OpenAL SDK，在http://www.openal.org下载，并把它的Include目录下建一个AL目录，把.h拷进去
除了Python以外的几个SDK的精简版本都能在我的网站找到。


__________________________________________________________________



发展历程

2.0.4
修正了OALMusicBuffer无法loop的bug
DSMusicBuffer改用timeSetEvent实现
改进了VertexBuffer

2.0.3 (2003.3.12)
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

	\Lib
		存放编译的内核静态连接库

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

\Doc
	文档

\AppLayer
	应用程序层

	\Include
		应用程序层的头文件

	\Lib
		存放编译的应用程序层静态连接库

	\Src
		应用程序层的源文件

\Plugins
	插件文件

	\Include
		插件头文件

	\Lib
		存放编译的插件静态连接库

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


__________________________________________________________________



开发计划
	在以后的版本中，将更好的支持控制台以及3D模型文件的读取

__________________________________________________________________


编译器支持

- 推荐使用Visual Studio.NET 2003 编译

- KlayGE是用Visual Studio.NET 2003 开发和编译的，还提供了Visual Studio.NET 2003 的 sln 文件。

- KlayGE从不能保证在 Visual Studio.NET 以前的版本中编译通过。

