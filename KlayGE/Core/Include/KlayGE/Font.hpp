// Font.hpp
// KlayGE Font类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2003-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 新的基于distance的字体格式 (2008.2.13)
//
// 3.6.0
// 增加了Rect对齐的方式 (2007.6.5)
//
// 3.3.0
// 支持渲染到3D位置 (2006.5.20)
//
// 2.8.0
// 增加了pool (2005.8.10)
//
// 2.3.0
// 使用FreeType实现字体读取 (2004.12.26)
// 
// 2.0.0
// 初次建立 (2003.8.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _FONT_HPP
#define _FONT_HPP

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/Rect.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/ClosedHashMap.hpp>

#include <list>
#include <map>
#include <vector>
#include <boost/functional/hash.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include <boost/pool/pool_alloc.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

namespace KlayGE
{
	class KLAYGE_CORE_API FontRenderable : public RenderableHelper
	{
	public:
		explicit FontRenderable(std::string const & fontName);

		RenderTechniquePtr GetRenderTechnique() const;

		void OnRenderBegin();
		void OnRenderEnd();

		void Render();

		Size_T<uint32_t> CalcSize(std::wstring const & text, uint32_t font_height);

		void AddText2D(float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height);
		void AddText2D(Rect const & rc, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height, uint32_t align);
		void AddText3D(float4x4 const & mvp, Color const & clr, std::wstring const & text, uint32_t font_height);

	private:
		void AddText(Rect const & rc, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height, uint32_t align);

		void AddText(float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height);

		void UpdateTexture(std::wstring const & text);

	private:
		typedef Rect_T<float> CharInfo;

#ifdef KLAYGE_PLATFORM_WINDOWS
	#pragma pack(push, 1)
#endif
		struct FontVert
		{
			float3 pos;
			uint32_t clr;
			float2 tex;

			FontVert()
			{
			}
			FontVert(float3 const & pos, uint32_t clr, float2 const & tex)
				: pos(pos), clr(clr), tex(tex)
			{
			}
		};

		struct font_info
		{
			int16_t top;
			int16_t left;
			uint16_t width;
			uint16_t height;
		};
#ifdef KLAYGE_PLATFORM_WINDOWS
	#pragma pack(pop)
#endif

		closed_hash_map<wchar_t, CharInfo, boost::hash<wchar_t>, std::equal_to<wchar_t>,
			boost::pool_allocator<std::pair<wchar_t, CharInfo> > > charInfoMap_;
		std::list<wchar_t, boost::fast_pool_allocator<wchar_t> > charLRU_;

		uint32_t curX_, curY_;

		bool three_dim_;

		std::vector<FontVert>	vertices_;
		std::vector<uint16_t>	indices_;

		GraphicsBufferPtr vb_;
		GraphicsBufferPtr ib_;

		TexturePtr		dist_texture_;
		TexturePtr		a_char_texture_;
		RenderEffectPtr	effect_;

		RenderEffectParameterPtr half_width_height_ep_;
		RenderEffectParameterPtr texel_to_pixel_offset_ep_;
		RenderEffectParameterPtr mvp_ep_;

		uint32_t kfont_char_size_;
		int16_t dist_base_;
		int16_t dist_scale_;
		closed_hash_map<int32_t, int32_t, boost::hash<int32_t>, std::equal_to<int32_t>,
			boost::pool_allocator<std::pair<int32_t, int32_t> > > char_index_;
		closed_hash_map<int32_t, Vector_T<uint16_t, 2>, boost::hash<int32_t>, std::equal_to<int32_t>,
			boost::pool_allocator<std::pair<int32_t, Vector_T<uint16_t, 2> > > > char_advance_;
		std::vector<font_info> char_info_;
		std::vector<uint8_t> distances_;
	};

	// 在3D环境中画出文字
	/////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API Font
	{
	public:
		// 字体建立标志
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
		Font(RenderablePtr const & font_renderable, uint32_t fontHeight = 16, uint32_t flags = 0);

		Size_T<uint32_t> CalcSize(std::wstring const & text);
		void RenderText(float x, float y, Color const & clr,
			std::wstring const & text);
		void RenderText(float x, float y, float z, float xScale, float yScale, Color const & clr, 
			std::wstring const & text);
		void RenderText(Rect const & rc, float z, float xScale, float yScale, Color const & clr, 
			std::wstring const & text, uint32_t align);
		void RenderText(float4x4 const & mvp, Color const & clr, std::wstring const & text);

		uint32_t FontHeight() const;

	private:
		RenderablePtr	font_renderable_;
		uint32_t		font_height_;
		uint32_t		fso_attrib_;
	};
}

#endif		// _FONT_HPP
