// Font.cpp
// KlayGE Font类 实现文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.5.0
// 试用Instance来实现 (2006.11.12)
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

#include <KlayGE/Font.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "freetype221_D.lib")
#else
	#pragma comment(lib, "freetype221.lib")
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

			GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static);
			vb->Resize(4 * sizeof(float2));
			{
				GraphicsBuffer::Mapper mapper(*vb, BA_Write_Only);
				mapper.Pointer<float2>()[0] = float2(0, 0);
				mapper.Pointer<float2>()[1] = float2(1, 0);
				mapper.Pointer<float2>()[2] = float2(1, 1);
				mapper.Pointer<float2>()[3] = float2(0, 1);
			}
			rl_->BindVertexStream(vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

			instancing_ = rf.MakeVertexBuffer(BU_Dynamic);
			rl_->BindVertexStream(instancing_,
				boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_BGR32F),
										vertex_element(VEU_TextureCoord, 1, EF_GR32F),
										vertex_element(VEU_TextureCoord, 2, EF_ABGR32F),
										vertex_element(VEU_Diffuse, 0, EF_ARGB8)),
				RenderLayout::ST_Instance, 1);

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
			ib->Resize(6 * sizeof(uint16_t));
			{
				GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
				mapper.Pointer<uint16_t>()[0] = 0;
				mapper.Pointer<uint16_t>()[1] = 1;
				mapper.Pointer<uint16_t>()[2] = 2;
				mapper.Pointer<uint16_t>()[3] = 2;
				mapper.Pointer<uint16_t>()[4] = 3;
				mapper.Pointer<uint16_t>()[5] = 0;
			}
			rl_->BindIndexStream(ib, EF_R16);

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
			instancing_->Resize(static_cast<uint32_t>(font_instances_.size() * sizeof(font_instances_[0])));
			{
				GraphicsBuffer::Mapper mapper(*instancing_, BA_Write_Only);
				std::copy(font_instances_.begin(), font_instances_.end(), mapper.Pointer<FontVert>());
			}

			for (uint32_t i = 0; i < rl_->NumVertexStreams(); ++ i)
			{
				rl_->VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, static_cast<uint32_t>(font_instances_.size()));
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
			font_instances_.resize(0);

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

		void AddText3D(uint32_t fontHeight, float4x4 const & mvp, Color const & clr,
			std::wstring const & text)
		{
			three_dim_ = true;
			mvp_ = mvp;

			this->AddText(fontHeight, 0, 0, 0, 1, 1, clr, text);
		}

	private:
		void AddText(uint32_t fontHeight, float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text)
		{
			this->UpdateTexture(text);

			uint32_t clr32 = clr.ABGR();
			float const h(fontHeight * yScale);
			size_t const maxSize(text.length() - std::count(text.begin(), text.end(), L'\n'));
			float x(sx), y(sy);
			float maxx(sx), maxy(sy);

			font_instances_.reserve(font_instances_.size() + maxSize);

			for (std::wstring::const_iterator citer = text.begin(); citer != text.end(); ++ citer)
			{
				wchar_t const & ch(*citer);
				CharInfoMapType::const_iterator cmiter = charInfoMap_.find(ch);
				float const w(cmiter->second.width * xScale);

				if (ch != L'\n')
				{
					font_instances_.push_back(FontVert(float3(x, y, sz), float2(w, h),
						cmiter->second.tex_width_height, clr32));

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
			for (std::wstring::const_iterator citer = text.begin(); citer != text.end(); ++ citer)
			{
				wchar_t const ch = *citer;

				if (charInfoMap_.find(ch) != charInfoMap_.end())
				{
					// 在现有纹理中找到了

					CharLRUType::iterator lruIter = std::find(charLRU_.begin(), charLRU_.end(), ch);
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
							charRect.right	= width;
							charRect.bottom = max_height;

							charInfo.tex_width_height.x()	= static_cast<float>(charRect.left) / tex_width;
							charInfo.tex_width_height.y()	= static_cast<float>(charRect.top) / tex_height;
							charInfo.tex_width_height.z()	= static_cast<float>(charRect.right) / tex_width;
							charInfo.tex_width_height.w()	= static_cast<float>(charRect.bottom) / tex_height;
							charInfo.width					= width;

							curX_ += width;
						}
						else
						{
							// 找到使用最长时间没有使用的字
							CharInfoMapType::iterator iter(charInfoMap_.find(charLRU_.back()));
							BOOST_ASSERT(iter != charInfoMap_.end());

							// 用当前字符替换
							charInfo.tex_width_height	= iter->second.tex_width_height;
							charInfo.width				= width;

							charLRU_.pop_back();
							charInfoMap_.erase(iter);

							charRect.left	= static_cast<long>(charInfo.tex_width_height.x() * tex_width);
							charRect.top	= static_cast<long>(charInfo.tex_width_height.y() * tex_height);
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
			float4		tex_width_height;
			uint32_t	width;
		};

		struct FontVert
		{
			float3 pos;
			float2 pos_width_height;
			float4 tex_width_height;
			uint32_t clr;

			FontVert()
			{
			}
			FontVert(float3 const & pos, float2 const & pos_width_height,
				float4 const & tex_width_height, uint32_t clr)
				: pos(pos), pos_width_height(pos_width_height),
					tex_width_height(tex_width_height), clr(clr)
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

		std::vector<FontVert>	font_instances_;

		GraphicsBufferPtr instancing_;

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
	void Font::RenderText(float sx, float sy, float sz,
		float xScale, float yScale, Color const & clr,
		std::wstring const & text)
	{
		if (!text.empty())
		{
			boost::shared_ptr<FontObject> font_obj(new FontObject(font_renderable_, fso_attrib_));
			checked_pointer_cast<FontRenderable>(font_renderable_)->AddText2D(this->FontHeight(),
				sx, sy, sz, xScale, yScale, clr, text);
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
