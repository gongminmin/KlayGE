// OGLFont.cpp
// KlayGE OGLFont类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 初次建立 (2004.4.4)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#define NOMINMAX

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderBuffer.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Box.hpp>

#include <cassert>
#include <algorithm>
#include <vector>
#include <cstring>

#include <KlayGE/OpenGL/OGLFont.hpp>

namespace
{
	using namespace KlayGE;

	class OGLFontRenderable : public Renderable
	{
	public:
		OGLFontRenderable(RenderEffectPtr const & effect, RenderBufferPtr const & rb)
			: fontEffect_(effect),
				fontRB_(rb)
		{
		}

		std::wstring const & Name() const
		{
			static const std::wstring name_(L"OpenGL Font");
			return name_;
		}

		void OnRenderBegin()
		{
			fontRB_->GetVertexStream(VST_Positions)->Assign(&xyzs_[0], xyzs_.size() / 3);
			fontRB_->GetVertexStream(VST_Diffuses)->Assign(&clrs_[0], clrs_.size());
			fontRB_->GetVertexStream(VST_TextureCoords0)->Assign(&texs_[0], texs_.size() / 2);

			fontRB_->GetIndexStream()->Assign(&indices_[0], indices_.size());
		}

		RenderEffectPtr GetRenderEffect() const
			{ return fontEffect_; }

		RenderBufferPtr GetRenderBuffer() const
			{ return fontRB_; }

		Box GetBound() const
			{ return box_; }

		bool CanBeCulled() const
			{ return false; }

		void RenderText(uint32_t fontHeight, OGLFont::CharInfoMapType& charInfoMap, float sx, float sy, float sz,
			float xScale, float yScale, uint32_t clr, std::wstring const & text, uint32_t flags)
		{
			// 设置过滤属性
			if (flags & Font::FA_Filtered)
			{
				Context::Instance().RenderFactoryInstance().RenderEngineInstance().TextureFiltering(0, RenderEngine::TF_Bilinear);
			}

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

			clrs_.resize(xyzs_.size() / 3, clr);

			box_ = Box(Vector3(sx, sy, sz), Vector3(maxx, maxy, sz + 0.1f));
		}

	private:
		RenderEffectPtr fontEffect_;
		RenderBufferPtr fontRB_;

		std::vector<float>	xyzs_;
		std::vector<uint32_t>	clrs_;
		std::vector<float>	texs_;
		std::vector<uint16_t>	indices_;

		Box box_;
	};
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	OGLFont::OGLFont(std::wstring const & fontName, uint32_t height, uint32_t flags)
				: curX_(0), curY_(0),
					theTexture_(Context::Instance().RenderFactoryInstance().MakeTexture(1024, 1024, 1, PF_A4L4)),
					rb_(new RenderBuffer(RenderBuffer::BT_TriangleList))
	{
		effect_ = LoadRenderEffect("Font.fx");
		*(effect_->ParameterByName("texFont")) = theTexture_;
		effect_->SetTechnique("fontTec");

		RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		Viewport const & viewport((*renderEngine.ActiveRenderTarget())->GetViewport());
		*(effect_->ParameterByName("halfWidth")) = viewport.width / 2;
		*(effect_->ParameterByName("halfHeight")) = viewport.height / 2;


		rb_->AddVertexStream(VST_Positions, sizeof(float), 3);
		rb_->AddVertexStream(VST_Diffuses, sizeof(uint32_t), 1);
		rb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2);

		rb_->AddIndexStream();


		logFont_.lfHeight			= height;
		logFont_.lfWidth			= 0;
		logFont_.lfEscapement		= 0;
		logFont_.lfOrientation		= 0;
		logFont_.lfWeight			= flags & Font::FS_Bold ? FW_BOLD : FW_NORMAL;
		logFont_.lfItalic			= flags & Font::FS_Italic ? TRUE : FALSE;
		logFont_.lfUnderline		= flags & Font::FS_Underline ? TRUE : FALSE;
		logFont_.lfStrikeOut		= flags & Font::FS_Strikeout ? TRUE : FALSE;
		logFont_.lfCharSet			= DEFAULT_CHARSET;
		logFont_.lfOutPrecision		= OUT_DEFAULT_PRECIS; 
		logFont_.lfClipPrecision	= CLIP_DEFAULT_PRECIS; 
		logFont_.lfQuality			= ANTIALIASED_QUALITY;
		logFont_.lfPitchAndFamily	= VARIABLE_PITCH;
		fontName.copy(logFont_.lfFaceName, fontName.length());
	}

	// 获取字体高度
	/////////////////////////////////////////////////////////////////////////////////
	uint32_t OGLFont::FontHeight() const
	{
		return logFont_.lfHeight;
	}

	// 更新纹理，使用LRU算法
	/////////////////////////////////////////////////////////////////////////////////
	void OGLFont::UpdateTexture(std::wstring const & text)
	{
		::SIZE size;
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
					// 为字体建立 DC 和 bitmap
					HDC hDC(::CreateCompatibleDC(NULL));

					// 建立字体
					HFONT hFont(::CreateFontIndirectW(&logFont_));
					if (NULL == hFont)
					{
						::DeleteDC(hDC);
						THR(E_FAIL);
					}
					HGDIOBJ hOldFont(::SelectObject(hDC, hFont));

					::GetTextExtentPoint32W(hDC, &ch, 1, &size);

					BITMAPINFO bmi;
					std::memset(&bmi.bmiHeader, 0, sizeof(bmi.bmiHeader));
					bmi.bmiHeader.biSize		= sizeof(bmi.bmiHeader);
					bmi.bmiHeader.biWidth		= size.cx;
					bmi.bmiHeader.biHeight		= -size.cy;
					bmi.bmiHeader.biPlanes		= 1;
					bmi.bmiHeader.biBitCount	= 32;
					bmi.bmiHeader.biCompression = BI_RGB;

					uint32_t* bitmapBits(NULL);
					HBITMAP hBitmap(::CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS,
						reinterpret_cast<void**>(&bitmapBits), NULL, 0));
					if ((NULL == hBitmap) || (NULL == bitmapBits))
					{
						::DeleteObject(::SelectObject(hDC, hOldFont));
						::DeleteDC(hDC);
						THR(E_FAIL);
					}

					HGDIOBJ hOldBitmap(::SelectObject(hDC, hBitmap));

					// 设置文字属性
					::SetTextColor(hDC, RGB(255, 255, 255));
					::SetBkColor(hDC, RGB(0, 0, 0));
					::SetTextAlign(hDC, TA_TOP);

					::TextOutW(hDC, 0, 0, &ch, 1);

					::RECT charRect;
					CharInfo charInfo;
					if ((curX_ < theTexture_->Width()) && (curY_ < theTexture_->Height()))
					{
						// 纹理还有空间
						charRect.left	= curX_;
						charRect.top	= curY_;
						charRect.right	= curX_ + size.cx;
						charRect.bottom = curY_ + size.cy;

						charInfo.texRect.left()		= static_cast<float>(charRect.left) / theTexture_->Width();
						charInfo.texRect.top()		= static_cast<float>(charRect.top) / theTexture_->Height();
						charInfo.texRect.right()	= static_cast<float>(charRect.right) / theTexture_->Width();
						charInfo.texRect.bottom()	= static_cast<float>(charRect.bottom) / theTexture_->Height();
						charInfo.width				= size.cx;

						curX_ += this->FontHeight();
						if (curX_ >= theTexture_->Width())
						{
							curX_ = 0;
							curY_ += this->FontHeight();
						}
					}
					else
					{
						// 找到使用最长时间没有使用的字
						CharInfoMapType::iterator iter(charInfoMap_.find(charLRU_.back()));
						assert(iter != charInfoMap_.end());

						// 用当前字符替换
						charInfo.texRect	= iter->second.texRect;
						charInfo.width		= size.cx;

						charLRU_.pop_back();
						charInfoMap_.erase(iter);

						charRect.left	= static_cast<long>(charInfo.texRect.left() * theTexture_->Width());
						charRect.top	= static_cast<long>(charInfo.texRect.top() * theTexture_->Height());
						charRect.right	= charRect.left + this->FontHeight();
						charRect.bottom	= charRect.top + this->FontHeight();
					}

					std::vector<uint8_t> dst;
					dst.reserve(this->FontHeight() * this->FontHeight());
					// 锁定表面，把 alpha 值写入纹理
					for (long y = charRect.top; y < charRect.bottom; ++ y)
					{
						for (long x = charRect.left; x < charRect.right; ++ x, ++ bitmapBits)
						{
							dst.push_back(static_cast<uint8_t>(*bitmapBits) & 0xF0 | 0x0F);
						}
					}
					theTexture_->CopyMemoryToTexture(&dst[0], PF_A4L4, charRect.right - charRect.left,
						charRect.bottom - charRect.top, charRect.left, charRect.top);

					// 已经更新了纹理，清除对象
					::DeleteObject(::SelectObject(hDC, hOldBitmap));
					::DeleteObject(::SelectObject(hDC, hOldFont));
					::DeleteDC(hDC);

					charInfoMap_.insert(std::make_pair(ch, charInfo));
					charLRU_.push_front(ch);
				}
			}
		}
	}

	// 在指定位置画出文字
	/////////////////////////////////////////////////////////////////////////////////
	RenderablePtr OGLFont::RenderText(float sx, float sy, Color const & clr, 
		std::wstring const & text, uint32_t flags)
	{
		return this->RenderText(sx, sy, 0.5f, 1, 1, clr, text, flags);
	}

	// 在指定位置画出放缩的文字
	/////////////////////////////////////////////////////////////////////////////////
	RenderablePtr OGLFont::RenderText(float sx, float sy, float sz,
		float xScale, float yScale, Color const & clr,
		std::wstring const & text, uint32_t flags)
	{
		if (text.empty())
		{
			return RenderablePtr();
		}

		this->UpdateTexture(text);

		uint8_t r, g, b, a;
		clr.RGBA(r, g, b, a);

		boost::shared_ptr<OGLFontRenderable> renderable(new OGLFontRenderable(effect_, rb_));
		renderable->RenderText(this->FontHeight(), charInfoMap_,
			sx, sy, sz, xScale, yScale, (a << 24) + (b << 16) + (g << 8) + (r), text, flags);
		return renderable;
	}
}

