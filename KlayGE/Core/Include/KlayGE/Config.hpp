#ifndef _CONFIG_HPP
#define _CONFIG_HPP

// 定义各种编译期选项
#define _SELECT1ST2ND_SUPPORT
#define _COPYIF_SUPPORT

#if defined(DEBUG) | defined(_DEBUG)
#define KLAYGE_DEBUG
#endif

#ifdef KLAYGE_DEBUG
#define D3D_DEBUG_INFO
#endif

// 定义本地的endian方式
#define _LITTLE_ENDIAN

#endif		// _CONFIG_HPP
