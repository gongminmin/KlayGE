// Font.cpp
// KlayGE Font类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2003-2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 增加了Rect对齐的方式 (2007.6.5)
//
// 3.4.0
// 优化了顶点缓冲区 (2006.9.20)
//
// 3.3.0
// 支持渲染到3D位置 (2006.5.20)
//
// 2.8.0
// 修正了越界的bug (2005.7.20)
// 增加了pool (2005.8.10)
//
// 2.7.1
// 美化了字体显示效果 (2005.7.7)
//
// 2.3.0
// 使用FreeType实现字体读取 (2004.12.26)
//
// 2.0.4
// 纹理格式改为PF_AL4 (2004.3.18)
//
// 2.0.3
// 修正了RenderText的Bug (2004.2.18)
// 改用VertexShader完成2D变换 (2004.3.1)
//
// 2.0.0
// 初次建立 (2003.8.18)
// 使用LRU算法 (2003.9.26)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Box.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Math.hpp>

#include <algorithm>
#include <vector>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/mem_fn.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/Font.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "freetype234_D.lib")
#else
	#pragma comment(lib, "freetype234.lib")
#endif
#endif

namespace
{
	using namespace KlayGE;

	ElementFormat const TEX_FORMAT = EF_L8;

	class FontRenderable : public RenderableHelper
	{
	public:
		FontRenderable(std::string const & fontName, uint32_t fontHeight, uint32_t /*flags*/)
			: RenderableHelper(L"Font"),
				curX_(0), curY_(0),
				three_dim_(false),
				fontHeight_(fontHeight)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleList);

			RenderEngine const & renderEngine = rf.RenderEngineInstance();
			RenderDeviceCaps const & caps = renderEngine.DeviceCaps();
			theTexture_ = rf.MakeTexture2D(caps.max_texture_width,
				caps.max_texture_height, 1, TEX_FORMAT);

			effect_ = rf.LoadEffect("Font.kfx");
			*(effect_->ParameterByName("texFontSampler")) = theTexture_;

			vb_ = rf.MakeVertexBuffer(BU_Dynamic);
			rl_->BindVertexStream(vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F),
											vertex_element(VEU_Diffuse, 0, EF_ARGB8),
											vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			ib_ = rf.MakeIndexBuffer(BU_Dynamic);
			rl_->BindIndexStream(ib_, EF_R16);

			box_ = Box(float3(0, 0, 0), float3(0, 0, 0));

			::FT_Init_FreeType(&ftLib_);
			::FT_New_Face(ftLib_, ResLoader::Instance().Locate(fontName).c_str(), 0, &face_);
			::FT_Set_Pixel_Sizes(face_, 0, fontHeight);
			::FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
			slot_ = face_->glyph;
		}

		~FontRenderable()
		{
			::FT_Done_Face(face_);
			::FT_Done_FreeType(ftLib_);
		}

		void OnRenderBegin()
		{
			vb_->Resize(static_cast<uint32_t>(vertices_.size() * sizeof(vertices_[0])));
			{
				GraphicsBuffer::Mapper mapper(*vb_, BA_Write_Only);
				std::copy(vertices_.begin(), vertices_.end(), mapper.Pointer<FontVert>());
			}

			ib_->Resize(static_cast<uint32_t>(indices_.size() * sizeof(indices_[0])));
			{
				GraphicsBuffer::Mapper mapper(*rl_->GetIndexStream(), BA_Write_Only);
				std::copy(indices_.begin(), indices_.end(), mapper.Pointer<uint16_t>());
			}

			if (!three_dim_)
			{
				technique_ = effect_->TechniqueByName("Font2DTec");

				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				*(effect_->ParameterByName("halfWidth")) = static_cast<int>(re.CurRenderTarget()->Width() / 2);
				*(effect_->ParameterByName("halfHeight")) = static_cast<int>(re.CurRenderTarget()->Height() / 2);
			}
			else
			{
				technique_ = effect_->TechniqueByName("Font3DTec");

				*(effect_->ParameterByName("mvp")) = mvp_;
			}
		}

		void OnRenderEnd()
		{
			vertices_.resize(0);
			indices_.resize(0);

			box_ = Box(float3(0, 0, 0), float3(0, 0, 0));
		}

		void Render()
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			this->OnRenderBegin();
			renderEngine.Render(*rl_);
			this->OnRenderEnd();
		}

		uint32_t FontHeight() const
		{
			return fontHeight_;
		}

		void AddText2D(uint32_t fontHeight, float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text)
		{
			three_dim_ = false;

			this->AddText(fontHeight, sx, sy, sz, xScale, yScale, clr, text);
		}

		void AddText2D(uint32_t fontHeight, Rect const & rc, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t align)
		{
			three_dim_ = false;

			this->AddText(fontHeight, rc, sz, xScale, yScale, clr, text, align);
		}

		void AddText3D(uint32_t fontHeight, float4x4 const & mvp, Color const & clr,
			std::wstring const & text)
		{
			three_dim_ = true;
			mvp_ = mvp;

			this->AddText(fontHeight, 0, 0, 0, 1, 1, clr, text);
		}

	private:
		void AddText(uint32_t fontHeight, Rect const & rc, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t align)
		{
			float const h(fontHeight * yScale);

			std::vector<std::pair<float, std::wstring> > lines(1, std::make_pair(0.0f, L""));

			BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
			{
				CharInfoMapType::const_iterator cmiter = charInfoMap_.find(ch);
				float const w(cmiter->second.width * xScale);

				if (ch != L'\n')
				{
					lines.back().first += w;
					lines.back().second.push_back(ch);
				}
				else
				{
					lines.push_back(std::make_pair(0.0f, L""));
				}
			}

			std::vector<float> sx;
			sx.reserve(lines.size());
			std::vector<float> sy;
			sy.reserve(lines.size());

			if (align & Font::FA_Hor_Left)
			{
				sx.resize(lines.size(), rc.left());
			}
			else
			{
				if (align & Font::FA_Hor_Right)
				{
					BOOST_FOREACH(BOOST_TYPEOF(lines)::const_reference p, lines)
					{
						sx.push_back(rc.Width() - p.first);
					}
				}
				else
				{
					// Font::FA_Hor_Center
					BOOST_FOREACH(BOOST_TYPEOF(lines)::const_reference p, lines)
					{
						sx.push_back(rc.Width() / 2 - p.first / 2);
					}
				}
			}

			if (align & Font::FA_Ver_Top)
			{
				for (BOOST_AUTO(iter, lines.begin()); iter != lines.end(); ++ iter)
				{
					sy.push_back(rc.top() + (iter - lines.begin()) * h);
				}
			}
			else
			{
				if (align & Font::FA_Ver_Bottom)
				{
					for (BOOST_AUTO(iter, lines.begin()); iter != lines.end(); ++ iter)
					{
						sy.push_back(rc.bottom() - (lines.size() - (iter - lines.begin())) * h);
					}
				}
				else
				{
					// Font::FA_Ver_Middle
					for (BOOST_AUTO(iter, lines.begin()); iter != lines.end(); ++ iter)
					{
						sy.push_back(rc.Height() / 2 - (lines.size() / 2 - (iter - lines.begin())) * h);
					}
				}
			}

			for (size_t i = 0; i < sx.size(); ++ i)
			{
				this->AddText(fontHeight, sx[i], sy[i], sz, xScale, yScale, clr, lines[i].second);
			}
		}

		void AddText(uint32_t fontHeight, float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text)
		{
			this->UpdateTexture(text);

			uint32_t clr32 = clr.ABGR();
			float const h(fontHeight * yScale);
			size_t const maxSize(text.length() - std::count(text.begin(), text.end(), L'\n'));
			float x(sx), y(sy);
			float maxx(sx), maxy(sy);

			vertices_.reserve(vertices_.size() + maxSize * 4);
			indices_.reserve(indices_.size() + maxSize * 6);

			uint16_t lastIndex(static_cast<uint16_t>(vertices_.size()));

			BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
			{
				CharInfoMapType::const_iterator cmiter = charInfoMap_.find(ch);
				float const w(cmiter->second.width * xScale);

				if (ch != L'\n')
				{
					Rect_T<float> const & texRect(cmiter->second.texRect);

					vertices_.push_back(FontVert(float3(x + 0, y + 0, sz),
											clr32,
											float2(texRect.left(), texRect.top())));
					vertices_.push_back(FontVert(float3(x + w, y + 0, sz),
											clr32,
											float2(texRect.right(), texRect.top())));
					vertices_.push_back(FontVert(float3(x + w, y + h, sz),
											clr32,
											float2(texRect.right(), texRect.bottom())));
					vertices_.push_back(FontVert(float3(x + 0, y + h, sz),
											clr32,
											float2(texRect.left(), texRect.bottom())));

					indices_.push_back(lastIndex + 0);
					indices_.push_back(lastIndex + 1);
					indices_.push_back(lastIndex + 2);
					indices_.push_back(lastIndex + 2);
					indices_.push_back(lastIndex + 3);
					indices_.push_back(lastIndex + 0);
					lastIndex += 4;

					x += w;

					if (x > maxx)
					{
						maxx = x;
					}
				}
				else
				{
					y += h;
					x = sx;

					if (y > maxy)
					{
						maxy = y;
					}
				}
			}

			box_ |= Box(float3(sx, sy, sz), float3(maxx, maxy, sz + 0.1f));
		}

		// 更新纹理，使用LRU算法
		/////////////////////////////////////////////////////////////////////////////////
		void UpdateTexture(std::wstring const & text)
		{
			BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
			{
				if (charInfoMap_.find(ch) != charInfoMap_.end())
				{
					// 在现有纹理中找到了

					BOOST_AUTO(lruIter, std::find(charLRU_.begin(), charLRU_.end(), ch));
					if (lruIter != charLRU_.begin())
					{
						charLRU_.splice(charLRU_.begin(), charLRU_, lruIter);
					}
				}
				else
				{
					// 在现有纹理中找不到，所以得在现有纹理中添加新字

					if (ch != L'\n')
					{
						int max_width, max_height;
						max_width = max_height = fontHeight_;

						// convert character code to glyph index
						::FT_Load_Char(face_, ch, FT_LOAD_RENDER);

						uint32_t const width = std::min<uint32_t>(max_width,
							(0 != slot_->bitmap.width) ? slot_->bitmap.width : max_width / 2);

						uint32_t const tex_width = theTexture_->Width(0);
						uint32_t const tex_height = theTexture_->Height(0);

						::RECT charRect;
						CharInfo charInfo;
						if ((curX_ < tex_width) && (curY_ < tex_height) && (curY_ + max_height < tex_height))
						{
							if (curX_ + width > tex_width)
							{
								curX_ = 0;
								curY_ += max_height;
							}

							// 纹理还有空间
							charRect.left	= curX_;
							charRect.top	= curY_;
							charRect.right	= curX_ + width;
							charRect.bottom = curY_ + max_height;

							charInfo.texRect.left()		= static_cast<float>(charRect.left) / tex_width;
							charInfo.texRect.top()		= static_cast<float>(charRect.top) / tex_height;
							charInfo.texRect.right()	= static_cast<float>(charRect.right) / tex_width;
							charInfo.texRect.bottom()	= static_cast<float>(charRect.bottom) / tex_height;
							charInfo.width				= width;

							curX_ += width;
						}
						else
						{
							// 找到使用最长时间没有使用的字
							BOOST_AUTO(iter, charInfoMap_.find(charLRU_.back()));
							BOOST_ASSERT(iter != charInfoMap_.end());

							// 用当前字符替换
							charInfo.texRect	= iter->second.texRect;
							charInfo.width		= width;

							charLRU_.pop_back();
							charInfoMap_.erase(iter);

							charRect.left	= static_cast<long>(charInfo.texRect.left() * tex_width);
							charRect.top	= static_cast<long>(charInfo.texRect.top() * tex_height);
							charRect.right	= charRect.left + width;
							charRect.bottom	= charRect.top + max_height;
						}

						int const buf_width = slot_->bitmap.width;
						int const buf_height = slot_->bitmap.rows;
						int const y_start = std::max<int>(max_height * 3 / 4 - slot_->bitmap_top, 0);
						std::vector<uint8_t> dest(max_width * max_height, 0);
						for (int y = 0; y < buf_height; ++ y)
						{
							for (int x = 0; x < buf_width; ++ x)
							{
								dest[(y + y_start) * max_width + x] = slot_->bitmap.buffer[y * buf_width + x];
							}
						}
						theTexture_->CopyMemoryToTexture2D(0, &dest[0], TEX_FORMAT,
								max_width, max_height, charRect.left, charRect.top,
								max_width, max_height);

						charInfoMap_.insert(std::make_pair(ch, charInfo));
						charLRU_.push_front(ch);
					}
				}
			}
		}

	private:
#ifdef KLAYGE_PLATFORM_WINDOWS
	#pragma pack(push, 1)
#endif
		struct CharInfo
		{
			Rect_T<float>	texRect;
			uint32_t		width;
		};

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
#ifdef KLAYGE_PLATFORM_WINDOWS
	#pragma pack(pop)
#endif

		typedef std::map<wchar_t, CharInfo, std::less<wchar_t>, boost::fast_pool_allocator<std::pair<wchar_t, CharInfo> > > CharInfoMapType;
		typedef std::list<wchar_t, boost::fast_pool_allocator<wchar_t> > CharLRUType;

		CharInfoMapType		charInfoMap_;
		CharLRUType			charLRU_;

		uint32_t curX_, curY_;

		bool three_dim_;
		float4x4 mvp_;

		uint32_t fontHeight_;

		std::vector<FontVert>	vertices_;
		std::vector<uint16_t>	indices_;

		GraphicsBufferPtr vb_;
		GraphicsBufferPtr ib_;

		TexturePtr		theTexture_;

		::FT_Library	ftLib_;
		::FT_Face		face_;
		::FT_GlyphSlot	slot_;

		RenderEffectPtr	effect_;
	};

	class FontObject : public SceneObjectHelper
	{
	public:
		FontObject(RenderablePtr renderable, uint32_t attrib)
			: SceneObjectHelper(renderable, attrib)
		{
		}
	};
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	Font::Font(std::string const & fontName, uint32_t height, uint32_t flags)
				: font_renderable_(new FontRenderable(fontName, height, flags))
	{
		fso_attrib_ = SceneObject::SOA_ShortAge;
		if (flags & Font::FS_Cullable)
		{
			fso_attrib_ |= SceneObject::SOA_Cullable;
		}
	}

	// 获取字体高度
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t Font::FontHeight() const
	{
		return checked_pointer_cast<FontRenderable>(font_renderable_)->FontHeight();
	}

	// 在指定位置画出文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float sx, float sy, Color const & clr,
		std::wstring const & text)
	{
		this->RenderText(sx, sy, 0, 1, 1, clr, text);
	}

	// 在指定位置画出放缩的文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float x, float y, float z,
		float xScale, float yScale, Color const & clr,
		std::wstring const & text)
	{
		if (!text.empty())
		{
			boost::shared_ptr<FontObject> font_obj(new FontObject(font_renderable_, fso_attrib_));
			checked_pointer_cast<FontRenderable>(font_renderable_)->AddText2D(this->FontHeight(),
				x, y, z, xScale, yScale, clr, text);
			font_obj->AddToSceneManager();
		}
	}

	// 在指定矩形区域内画出放缩的文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(Rect const & rc, float z,
		float xScale, float yScale, Color const & clr, 
		std::wstring const & text, uint32_t align)
	{
		if (!text.empty())
		{
			boost::shared_ptr<FontObject> font_obj(new FontObject(font_renderable_, fso_attrib_));
			checked_pointer_cast<FontRenderable>(font_renderable_)->AddText2D(this->FontHeight(),
				rc, z, xScale, yScale, clr, text, align);
			font_obj->AddToSceneManager();
		}
	}

	// 在指定位置画出3D的文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float4x4 const & mvp, Color const & clr, std::wstring const & text)
	{
		if (!text.empty())
		{
			boost::shared_ptr<FontObject> font_obj(new FontObject(font_renderable_, fso_attrib_));
			checked_pointer_cast<FontRenderable>(font_renderable_)->AddText3D(this->FontHeight(),
				mvp, clr, text);
			font_obj->AddToSceneManager();
		}
	}
}
