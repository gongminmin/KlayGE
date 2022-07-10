// Font.hpp
// KlayGE Font�� ͷ�ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// RenderText�ٶ�����50% (2010.3.9)
//
// 3.9.0
// ������KFontLoader (2009.10.16)
//
// 3.7.0
// �µĻ���distance�������ʽ (2008.2.13)
//
// 3.6.0
// ������Rect����ķ�ʽ (2007.6.5)
//
// 3.3.0
// ֧����Ⱦ��3Dλ�� (2006.5.20)
//
// 2.8.0
// ������pool (2005.8.10)
//
// 2.3.0
// ʹ��FreeTypeʵ�������ȡ (2004.12.26)
//
// 2.0.0
// ���ν��� (2003.8.18)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#ifndef _FONT_HPP
#define _FONT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <KFL/Rect.hpp>
#include <KlayGE/Renderable.hpp>

#include <list>
#include <vector>

namespace KlayGE
{
	class FontRenderable;

	// ��3D�����л�������
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API Font final : boost::noncopyable
	{
	public:
		// ���彨����־
		enum FontStyle
		{
			FS_TwoSided		= 1UL << 0,
			FS_Cullable		= 1UL << 1
		};

		enum FontAlign
		{
			FA_Hor_Left		= 1UL << 0,
			FA_Hor_Center	= 1UL << 1,
			FA_Hor_Right	= 1UL << 2,

			FA_Ver_Top		= 1UL << 3,
			FA_Ver_Middle	= 1UL << 4,
			FA_Ver_Bottom	= 1UL << 5
		};

	public:
		explicit Font(std::shared_ptr<FontRenderable> const & fr);
		Font(std::shared_ptr<FontRenderable> const & fr, uint32_t flags);

		Size_T<float> CalcSize(std::wstring_view text, float font_size);
		void RenderText(float x, float y, Color const & clr,
			std::wstring_view text, float font_size);
		void RenderText(float x, float y, float z, float xScale, float yScale, Color const & clr,
			std::wstring_view text, float font_size);
		void RenderText(Rect const & rc, float z, float xScale, float yScale, Color const & clr,
			std::wstring_view text, float font_size, uint32_t align);
		void RenderText(float4x4 const & mvp, Color const & clr, std::wstring_view text, float font_size);

	private:
		std::shared_ptr<FontRenderable> font_renderable_;
		uint32_t fsn_attrib_;
	};

	KLAYGE_CORE_API FontPtr SyncLoadFont(std::string_view font_name, uint32_t flags = 0);
	KLAYGE_CORE_API FontPtr ASyncLoadFont(std::string_view font_name, uint32_t flags = 0);
}

#endif		// _FONT_HPP
