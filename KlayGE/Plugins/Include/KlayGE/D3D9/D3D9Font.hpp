// D3D9Font.hpp
// KlayGE D3D9Font类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.9.4)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9FONT_HPP
#define _D3D9FONT_HPP

#include <KlayGE/Rect.hpp>

#include <list>
#include <map>

#include <KlayGE/Font.hpp>

#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")

namespace KlayGE
{
	// 在3D环境中画出文字
	/////////////////////////////////////////////////////////////////////////////////
	class D3D9Font : public Font
	{
	public:
		// 2D & 3D 画出文字的函数
		RenderablePtr RenderText(float x, float y, Color const & clr,
			std::wstring const & text, U32 flags = 0);
		RenderablePtr RenderText(float x, float y, float z, float xScale, float yScale,
			Color const & clr, std::wstring const & text, U32 flags = 0);
		//RenderablePtr RenderText(std::wstring const & text, U32 flags = 0);

		U32 FontHeight() const;

		D3D9Font(std::wstring const & fontName, U32 fontHeight = 12, U32 flags = 0);

	public:
		struct CharInfo
		{
			Rect_T<float>	texRect;
			U32				width;
		};

		typedef std::map<wchar_t, CharInfo> CharInfoMapType;
		typedef std::list<wchar_t> CharLRUType;

	public:
		void UpdateTexture(std::wstring const & text);

		LOGFONTW logFont_;

		RenderBufferPtr rb_;
		RenderEffectPtr	effect_;
		TexturePtr		theTexture_;

		CharInfoMapType charInfoMap_;
		CharLRUType charLRU_;

		U32		curX_, curY_;
	};
}

#endif		// _D3D9FONT_HPP
