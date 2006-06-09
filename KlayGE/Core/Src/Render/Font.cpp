// Font.cpp
// KlayGE Font类 实现文件
// Ver 3.3.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
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

#define NOMINMAX

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

#include <algorithm>
#include <vector>
#include <cstring>
#include <boost/assert.hpp>
#include <boost/mem_fn.hpp>

#include <KlayGE/Font.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "freetype2110MT_D.lib")
#else
	#pragma comment(lib, "freetype2110MT.lib")
#endif

namespace
{
	using namespace KlayGE;

	ElementFormat const TEX_FORMAT = EF_ARGB4;

	class FontRenderable : public RenderableHelper
	{
	public:
		FontRenderable(std::string const & fontName, uint32_t fontHeight, uint32_t flags)
			: RenderableHelper(L"Font"),
				curX_(0), curY_(0),
				fontHeight_(fontHeight),
				theSampler_(new Sampler),
				three_dim_(false)
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleList);

			RenderEngine const & renderEngine = rf.RenderEngineInstance();
			RenderDeviceCaps const & caps = renderEngine.DeviceCaps();
			theTexture_ = rf.MakeTexture2D(caps.max_texture_width,
				caps.max_texture_height, 1, TEX_FORMAT);
			theSampler_->SetTexture(theTexture_);

			// 设置过滤属性
			if (flags & Font::FS_Filtered)
			{
				theSampler_->Filtering(Sampler::TFO_Bilinear);
			}
			else
			{
				theSampler_->Filtering(Sampler::TFO_Point);
			}

			effect_ = rf.LoadEffect("Font.fx");
			*(effect_->ParameterByName("texFontSampler")) = theSampler_;

			xyz_vb_ = rf.MakeVertexBuffer(BU_Dynamic);
			clr_vb_ = rf.MakeVertexBuffer(BU_Dynamic);
			tex_vb_ = rf.MakeVertexBuffer(BU_Dynamic);

			rl_->BindVertexStream(xyz_vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
			rl_->BindVertexStream(clr_vb_, boost::make_tuple(vertex_element(VEU_Diffuse, 0, EF_ABGR32F)));
			rl_->BindVertexStream(tex_vb_, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			rl_->BindIndexStream(rf.MakeIndexBuffer(BU_Dynamic), EF_D16);

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
			xyz_vb_->Resize(static_cast<uint32_t>(xyzs_.size() * sizeof(xyzs_[0])));
			{
				GraphicsBuffer::Mapper mapper(*xyz_vb_, BA_Write_Only);
				std::copy(xyzs_.begin(), xyzs_.end(), mapper.Pointer<float3>());
			}
			clr_vb_->Resize(static_cast<uint32_t>(clrs_.size() * sizeof(clrs_[0])));
			{
				GraphicsBuffer::Mapper mapper(*clr_vb_, BA_Write_Only);
				std::copy(clrs_.begin(), clrs_.end(), mapper.Pointer<Color>());
			}
			tex_vb_->Resize(static_cast<uint32_t>(texs_.size() * sizeof(texs_[0])));
			{
				GraphicsBuffer::Mapper mapper(*tex_vb_, BA_Write_Only);
				std::copy(texs_.begin(), texs_.end(), mapper.Pointer<float2>());
			}

			rl_->GetIndexStream()->Resize(static_cast<uint32_t>(indices_.size() * sizeof(indices_[0])));
			{
				GraphicsBuffer::Mapper mapper(*rl_->GetIndexStream(), BA_Write_Only);
				std::copy(indices_.begin(), indices_.end(), mapper.Pointer<uint16_t>());
			}

			if (!three_dim_)
			{
				technique_ = effect_->Technique("Font2DTec");
			
				RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
				Viewport const & viewport(renderEngine.CurRenderTarget()->GetViewport());
				*(effect_->ParameterByName("halfWidth")) = viewport.width / 2;
				*(effect_->ParameterByName("halfHeight")) = viewport.height / 2;
			}
			else
			{
				technique_ = effect_->Technique("Font3DTec");

				*(effect_->ParameterByName("mvp")) = mvp_;
			}
		}

		void OnRenderEnd()
		{
			xyzs_.resize(0);
			clrs_.resize(0);
			texs_.resize(0);
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

			float const h(fontHeight * yScale);
			size_t const maxSize(text.length() - std::count(text.begin(), text.end(), L'\n'));
			float x(sx), y(sy);
			float maxx(sx), maxy(sy);

			xyzs_.reserve(xyzs_.size() + maxSize * 4);
			clrs_.reserve(clrs_.size() + maxSize * 4);
			texs_.reserve(texs_.size() + maxSize * 4);
			indices_.reserve(indices_.size() + maxSize * 6);

			uint16_t lastIndex(static_cast<uint16_t>(xyzs_.size()));
			for (std::wstring::const_iterator citer = text.begin(); citer != text.end(); ++ citer)
			{
				wchar_t const & ch(*citer);
				CharInfoMapType::const_iterator cmiter = charInfoMap_.find(ch);
				float const w(cmiter->second.width * xScale);

				if (ch != L'\n')
				{
					Rect_T<float> const & texRect(cmiter->second.texRect);

					xyzs_.push_back(float3(x + 0, y + 0, sz));
					xyzs_.push_back(float3(x + w, y + 0, sz));
					xyzs_.push_back(float3(x + w, y + h, sz));
					xyzs_.push_back(float3(x + 0, y + h, sz));

					clrs_.push_back(clr);
					clrs_.push_back(clr);
					clrs_.push_back(clr);
					clrs_.push_back(clr);

					texs_.push_back(float2(texRect.left(), texRect.top()));
					texs_.push_back(float2(texRect.right(), texRect.top()));
					texs_.push_back(float2(texRect.right(), texRect.bottom()));
					texs_.push_back(float2(texRect.left(), texRect.bottom()));

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
							CharInfoMapType::iterator iter(charInfoMap_.find(charLRU_.back()));
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
						std::vector<uint16_t> dest(max_width * max_height, 0);
						for (int y = 0; y < buf_height; ++ y)
						{
							for (int x = 0; x < buf_width; ++ x)
							{
								dest[(y + y_start) * max_width + x]
										= ((slot_->bitmap.buffer[y * buf_width + x] & 0xF0) << 8) | 0x0FFF;
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
		struct CharInfo
		{
			Rect_T<float>	texRect;
			uint32_t		width;
		};

		typedef std::map<wchar_t, CharInfo, std::less<wchar_t>, boost::fast_pool_allocator<std::pair<wchar_t, CharInfo> > > CharInfoMapType;
		typedef std::list<wchar_t, boost::fast_pool_allocator<wchar_t> > CharLRUType;

		CharInfoMapType		charInfoMap_;
		CharLRUType			charLRU_;

		uint32_t curX_, curY_;

		bool three_dim_;
		float4x4 mvp_;

		uint32_t fontHeight_;

		std::vector<float3>	xyzs_;
		std::vector<Color>		clrs_;
		std::vector<float2>	texs_;
		std::vector<uint16_t>	indices_;

		GraphicsBufferPtr xyz_vb_;
		GraphicsBufferPtr clr_vb_;
		GraphicsBufferPtr tex_vb_;

		TexturePtr		theTexture_;
		SamplerPtr		theSampler_;

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
		return checked_cast<FontRenderable*>(font_renderable_.get())->FontHeight();
	}

	// 在指定位置画出文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float sx, float sy, Color const & clr, 
		std::wstring const & text)
	{
		this->RenderText(sx, sy, 0.5f, 1, 1, clr, text);
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
			checked_cast<FontRenderable*>(font_renderable_.get())->AddText2D(this->FontHeight(),
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
			checked_cast<FontRenderable*>(font_renderable_.get())->AddText3D(this->FontHeight(),
				mvp, clr, text);
			font_obj->AddToSceneManager();
		}
	}
}
