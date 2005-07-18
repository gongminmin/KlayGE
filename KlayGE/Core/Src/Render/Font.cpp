// Font.cpp
// KlayGE Font类 实现文件
// Ver 2.7.1
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.1
// 美化了字体显示效果 (2005.7.7)
//
// 2.3.0
// 使用FreeType实现字体读取 (2004.12.26)
//
// 2.0.4
// 纹理格式改为PF_A4L4 (2004.3.18)
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
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Box.hpp>
#include <KlayGE/ResLoader.hpp>

#include <cassert>
#include <algorithm>
#include <vector>
#include <cstring>

#include <KlayGE/Font.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "freetype2110MT_D.lib")
#else
	#pragma comment(lib, "freetype2110MT.lib")
#endif

namespace
{
	using namespace KlayGE;

	PixelFormat TEX_FORMAT = PF_A4R4G4B4;

	class FontRenderable : public Renderable
	{
	public:
		FontRenderable(RenderEffectPtr const & effect, VertexBufferPtr const & vb)
			: fontEffect_(effect),
				fontVB_(vb),
				box_(Vector3(0, 0, 0), Vector3(0, 0, 0))
		{
		}

		std::wstring const & Name() const
		{
			static const std::wstring name_(L"Font");
			return name_;
		}

		void OnRenderBegin()
		{
			fontVB_->GetVertexStream(VST_Positions)->Assign(&xyzs_[0], xyzs_.size() / 3);
			fontVB_->GetVertexStream(VST_TextureCoords0)->Assign(&texs_[0], texs_.size() / 2);

			fontVB_->GetIndexStream()->Assign(&indices_[0], indices_.size());

			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			Viewport const & viewport((*renderEngine.ActiveRenderTarget())->GetViewport());
			*(fontEffect_->ParameterByName("halfWidth")) = viewport.width / 2;
			*(fontEffect_->ParameterByName("halfHeight")) = viewport.height / 2;

			*(fontEffect_->ParameterByName("color")) = Vector4(clr_.r(), clr_.g(), clr_.b(), clr_.a());
		}

		RenderEffectPtr GetRenderEffect() const
		{
			return fontEffect_;
		}

		VertexBufferPtr GetVertexBuffer() const
		{
			return fontVB_;
		}

		Box GetBound() const
		{
			return box_;
		}

		bool CanBeCulled() const
		{
			return canBeCulled_;
		}

		bool ShortAge() const
		{
			return true;
		}

		void RenderText(uint32_t fontHeight, Font::CharInfoMapType& charInfoMap, float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t flags)
		{
			if (flags & Font::FA_CanBeCulled)
			{
				canBeCulled_ = true;
			}
			else
			{
				canBeCulled_ = false;
			}

			clr_ = clr;

			float const h(fontHeight * yScale);
			size_t const maxSize(text.length() - std::count(text.begin(), text.end(), L'\n'));
			float x(sx), y(sy);
			float maxx(sx), maxy(sy);

			xyzs_.clear();
			texs_.clear();
			indices_.clear();

			xyzs_.reserve(maxSize * 3 * 4);
			texs_.reserve(maxSize * 2 * 4);
			indices_.reserve(maxSize * 6);

			uint16_t lastIndex(0);
			for (std::wstring::const_iterator citer = text.begin(); citer != text.end(); ++ citer)
			{
				wchar_t const & ch(*citer);
				float const w(charInfoMap[ch].width * xScale);

				if (ch != L'\n')
				{
					Rect_T<float> const & texRect(charInfoMap[ch].texRect);

					xyzs_.push_back(x);
					xyzs_.push_back(y);
					xyzs_.push_back(sz);

					xyzs_.push_back(x + w);
					xyzs_.push_back(y);
					xyzs_.push_back(sz);

					xyzs_.push_back(x + w);
					xyzs_.push_back(y + h);
					xyzs_.push_back(sz);

					xyzs_.push_back(x);
					xyzs_.push_back(y + h);
					xyzs_.push_back(sz);


					texs_.push_back(texRect.left());
					texs_.push_back(texRect.top());

					texs_.push_back(texRect.right());
					texs_.push_back(texRect.top());

					texs_.push_back(texRect.right());
					texs_.push_back(texRect.bottom());

					texs_.push_back(texRect.left());
					texs_.push_back(texRect.bottom());


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

			box_ = Box(Vector3(sx, sy, sz), Vector3(maxx, maxy, sz + 0.1f));
		}

	private:
		RenderEffectPtr fontEffect_;
		VertexBufferPtr fontVB_;

		std::vector<float>		xyzs_;
		std::vector<float>		texs_;
		std::vector<uint16_t>	indices_;
		Color					clr_;

		Box box_;

		bool canBeCulled_;
	};
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	Font::Font(std::string const & fontName, uint32_t height, uint32_t /*flags*/)
				: curX_(0), curY_(0),
					fontHeight_(height),
					vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
	{
		RenderEngine const & renderEngine = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		RenderDeviceCaps const & caps = renderEngine.DeviceCaps();
		theTexture_ = Context::Instance().RenderFactoryInstance().MakeTexture2D(caps.max_texture_width,
			caps.max_texture_height, 1, TEX_FORMAT);

		effect_ = LoadRenderEffect("Font.fx");
		*(effect_->ParameterByName("texFont")) = theTexture_;
		effect_->SetTechnique("fontTec");

		vb_->AddVertexStream(VST_Positions, sizeof(float), 3);
		vb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2);

		vb_->AddIndexStream();

		::FT_Init_FreeType(&ftLib_);
		::FT_New_Face(ftLib_, ResLoader::Instance().Locate(fontName).c_str(), 0, &face_);
		::FT_Set_Pixel_Sizes(face_, 0, height);
		::FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
		slot_ = face_->glyph;
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	Font::~Font()
	{
		::FT_Done_Face(face_);
		::FT_Done_FreeType(ftLib_);
	}

	// 获取字体高度
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t Font::FontHeight() const
	{
		return fontHeight_;
	}

	// 更新纹理，使用LRU算法
	/////////////////////////////////////////////////////////////////////////////////
	void Font::UpdateTexture(std::wstring const & text)
	{
		for (std::wstring::const_iterator citer = text.begin(); citer != text.end(); ++ citer)
		{
			wchar_t const & ch(*citer);

			if (charInfoMap_.find(ch) != charInfoMap_.end())
			{
				// 在现有纹理中找到了

				if (ch != charLRU_.front())
				{
					charLRU_.remove(ch);
					charLRU_.push_front(ch);
				}
			}
			else
			{
				// 在现有纹理中找不到，所以得在现有纹理中添加新字

				if (ch != L'\n')
				{
					int max_width, max_height;
					max_width = max_height = this->FontHeight();

					// convert character code to glyph index
					::FT_Load_Char(face_, ch, FT_LOAD_RENDER);

					uint32_t const width = std::min<uint32_t>(max_width,
						(0 != slot_->bitmap.width) ? slot_->bitmap.width : max_width / 2);

					::RECT charRect;
					CharInfo charInfo;
					if ((curX_ < theTexture_->Width(0)) && (curY_ < theTexture_->Height(0)))
					{
						// 纹理还有空间
						charRect.left	= curX_;
						charRect.top	= curY_;
						charRect.right	= curX_ + width;
						charRect.bottom = curY_ + max_height;

						charInfo.texRect.left()		= static_cast<float>(charRect.left) / theTexture_->Width(0);
						charInfo.texRect.top()		= static_cast<float>(charRect.top) / theTexture_->Height(0);
						charInfo.texRect.right()	= static_cast<float>(charRect.right) / theTexture_->Width(0);
						charInfo.texRect.bottom()	= static_cast<float>(charRect.bottom) / theTexture_->Height(0);
						charInfo.width				= width;

						curX_ += width;
						if (curX_ >= theTexture_->Width(0))
						{
							curX_ = 0;
							curY_ += max_height;
						}
					}
					else
					{
						// 找到使用最长时间没有使用的字
						CharInfoMapType::iterator iter(charInfoMap_.find(charLRU_.back()));
						assert(iter != charInfoMap_.end());

						// 用当前字符替换
						charInfo.texRect	= iter->second.texRect;
						charInfo.width		= width;

						charLRU_.pop_back();
						charInfoMap_.erase(iter);

						charRect.left	= static_cast<long>(charInfo.texRect.left() * theTexture_->Width(0));
						charRect.top	= static_cast<long>(charInfo.texRect.top() * theTexture_->Height(0));
						charRect.right	= charRect.left + width;
						charRect.bottom	= charRect.top + this->FontHeight();
					}

					std::vector<uint16_t> dest(max_width * max_height, 0);
					int const rows(std::min<int>(slot_->bitmap.rows, max_height));
					int const cols(std::min<int>(slot_->bitmap.width, max_width));
					int const y_start = std::max<int>(max_height * 3 / 4 - slot_->bitmap_top, 0);
					for (int y = 0; y < rows; ++ y)
					{
						int const y_offset = y_start + y;
						if (y_offset < max_height)
						{
							for (int x = 0; x < cols; ++ x)
							{
								if ((y < max_height) && (x < max_width))
								{
									dest[y_offset * max_width + x]
										= ((slot_->bitmap.buffer[y * slot_->bitmap.width + x] & 0xF0) << 8) | 0x0FFF;
								}
							}
						}
					}
					theTexture_->CopyMemoryToTexture2D(0, &dest[0], TEX_FORMAT,
							max_width, max_height, charRect.left, charRect.top);

					charInfoMap_.insert(std::make_pair(ch, charInfo));
					charLRU_.push_front(ch);
				}
			}
		}
	}

	// 在指定位置画出文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float sx, float sy, Color const & clr, 
		std::wstring const & text, uint32_t flags)
	{
		this->RenderText(sx, sy, 0.5f, 1, 1, clr, text, flags);
	}

	// 在指定位置画出放缩的文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float sx, float sy, float sz,
		float xScale, float yScale, Color const & clr,
		std::wstring const & text, uint32_t flags)
	{
		if (!text.empty())
		{
			// 设置过滤属性
			if (flags & Font::FA_Filtered)
			{
				theTexture_->Filtering(Texture::TFT_Min, Texture::TFO_Bilinear);
				theTexture_->Filtering(Texture::TFT_Mag, Texture::TFO_Bilinear);
				theTexture_->Filtering(Texture::TFT_Mip, Texture::TFO_None);
			}
			else
			{
				theTexture_->Filtering(Texture::TFT_Min, Texture::TFO_Point);
				theTexture_->Filtering(Texture::TFT_Mag, Texture::TFO_Point);
				theTexture_->Filtering(Texture::TFT_Mip, Texture::TFO_None);
			}

			this->UpdateTexture(text);

			boost::shared_ptr<FontRenderable> renderable(new FontRenderable(effect_, vb_));
			renderable->RenderText(this->FontHeight(), charInfoMap_,
				sx, sy, sz, xScale, yScale, clr, text, flags);
			renderable->AddToSceneManager();
		}
	}
}
