// D3D9Font.cpp
// KlayGE D3D9Font类 实现文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/SharePtr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Engine.hpp>

#include <cassert>
#include <algorithm>
#include <vector>

#include <KlayGE/D3D9/D3D9Font.hpp>

namespace
{
	using namespace KlayGE;

	String fontEffectStr("\
		texture texFont;\
		int halfWidth;\
		int halfHeight;\
		\
		void FontVS(float4 position : POSITION,\
			float2 texCoord : TEXCOORD0,\
			float4 color : DIFFUSE,\
		\
			out float4 oPosition : POSITION,\
			out float2 oTexCoord : TEXCOORD0,\
			out float4 oColor : COLOR)\
		{\
			oPosition.x = (position.x - halfWidth) / halfWidth;\
			oPosition.y = (halfHeight - position.y) / halfHeight;\
			oPosition.zw = position.zw;\
		\
			oColor = color;\
			oTexCoord = texCoord;\
		}\
		\
		technique fontTec\
		{\
			pass p0\
			{\
				Lighting = false;\
		\
				AlphaBlendEnable = true;\
				SrcBlend = SrcColor;\
				DestBlend = DestColor;\
				AlphaTestEnable = true;\
				AlphaRef = 0x08;\
				AlphaFunc = GreaterEqual;\
		\
				FillMode = Solid;\
				CullMode = CCW;\
				Stencilenable = false;\
				Clipping = true;\
				ClipPlaneEnable = 0;\
				VertexBlend = Disable;\
				IndexedVertexBlendEnable = false;\
				FogEnable = false;\
				ColorWriteEnable = RED | GREEN | BLUE | ALPHA;\
		\
				Texture[0] = <texFont>;\
				TextureTransformFlags[0] = Disable;\
				ColorOp[0] = Modulate;\
				ColorArg1[0] = Texture;\
				ColorArg2[0] = Diffuse;\
				AlphaOp[0] = Modulate;\
				AlphaArg1[0] = Texture;\
				AlphaArg2[0] = Diffuse;\
				MinFilter[0] = Point;\
				MagFilter[0] = Point;\
				MipFilter[0] = None;\
		\
				ColorOp[1] = Disable;\
				AlphaOp[1] = Disable;\
		\
				VertexShader = compile vs_1_1 FontVS();\
			}\
		}\
		");


	class D3D9FontRenderable : public Renderable
	{
	public:
		D3D9FontRenderable(const RenderEffectPtr& effect)
			: fontVB_(new VertexBuffer),
				fontEffect_(effect)
		{
			fontVB_->type = VertexBuffer::BT_TriangleList;
			fontVB_->numTextureCoordSets = 1;
			fontVB_->numTextureDimensions[0] = 2;
			fontVB_->useIndices = true;
			fontVB_->vertexOptions = (VertexBuffer::VO_Diffuses | VertexBuffer::VO_TextureCoords);
		}

		const WString& Name() const
		{
			static WString name(L"Direct3D9 Font");
			return name;
		}

		RenderEffectPtr GetRenderEffect()
		{
			return fontEffect_;
		}

		VertexBufferPtr GetVertexBuffer()
		{
			if (!xyzs_.empty())
			{
				fontVB_->numVertices	= xyzs_.size() / 3;
				fontVB_->pVertices		= &xyzs_[0];
				fontVB_->pDiffuses		= &clrs_[0];
				fontVB_->pTexCoords[0]	= &texs_[0];

				fontVB_->pIndices	= &indices_[0];
				fontVB_->numIndices = indices_.size();
			}

			return fontVB_;
		}

		void RenderText(U32 fontHeight, D3D9Font::CharInfoMapType& charInfoMap, float sx, float sy, float sz,
			float xScale, float yScale, U32 d3dclr, const WString& text, U32 flags)
		{
			// 设置过滤属性
			if (flags & Font::FA_Filtered)
			{
				Engine::RenderFactoryInstance().RenderEngineInstance().TextureFiltering(0, RenderEngine::TF_Bilinear);
			}

			const float h(fontHeight * yScale);
			const size_t maxSize(text.length() - std::count(text.begin(), text.end(), L'\n'));
			float x(sx), y(sy);

			clrs_.resize(maxSize * 4, d3dclr);

			xyzs_.clear();
			texs_.clear();
			indices_.clear();
			xyzs_.reserve(maxSize * 3 * 4);
			texs_.reserve(maxSize * 2 * 4);
			indices_.reserve(maxSize * 6);

			U16 lastIndex(0);
			for (WString::const_iterator citer = text.begin(); citer != text.end(); ++ citer)
			{
				const wchar_t& ch(*citer);
				const float w(charInfoMap[ch].width * xScale);

				if (ch != L'\n')
				{
					const Rect_T<float>& texRect(charInfoMap[ch].texRect);

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
				}
				else
				{
					y += h;
					x = sx;
				}
			}
		}

	private:
		RenderEffectPtr	fontEffect_;
		VertexBufferPtr fontVB_;

		std::vector<float, alloc<float> >	xyzs_;
		std::vector<U32, alloc<U32> >		clrs_;
		std::vector<float, alloc<float> >	texs_;

		std::vector<U16, alloc<U16> >		indices_;
	};
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D9Font::D3D9Font(const WString& fontName, U32 height, U32 flags)
				: curX_(0), curY_(0),
					theTexture_(Engine::RenderFactoryInstance().MakeTexture(1024, 1024, 1, PF_A4R4G4B4)),
					effect_(Engine::RenderFactoryInstance().MakeRenderEffect(fontEffectStr))
	{
		effect_->SetTexture("texFont", theTexture_);

		RenderEngine& renderEngine(Engine::RenderFactoryInstance().RenderEngineInstance());
		Viewport viewport((*renderEngine.ActiveRenderTarget())->GetViewport());
		effect_->SetInt("halfWidth", viewport.width / 2);
		effect_->SetInt("halfHeight", viewport.height / 2);

		effect_->Technique("fontTec");
		effect_->Validate();

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
	U32 D3D9Font::FontHeight() const
	{
		return logFont_.lfHeight;
	}

	// 更新纹理，使用LRU算法
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9Font::UpdateTexture(const WString& text)
	{
		::SIZE size;
		for (WString::const_iterator citer = text.begin(); citer != text.end(); ++ citer)
		{
			const wchar_t& ch(*citer);

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
					Engine::MemoryInstance().Zero(&bmi.bmiHeader, sizeof(bmi.bmiHeader));
					bmi.bmiHeader.biSize		= sizeof(bmi.bmiHeader);
					bmi.bmiHeader.biWidth		= size.cx;
					bmi.bmiHeader.biHeight		= -size.cy;
					bmi.bmiHeader.biPlanes		= 1;
					bmi.bmiHeader.biBitCount	= 32;
					bmi.bmiHeader.biCompression = BI_RGB;

					U32* pBitmapBits(NULL);
					HBITMAP hBitmap(::CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS,
						reinterpret_cast<PVOID*>(&pBitmapBits), NULL, 0));
					if ((NULL == hBitmap) || (NULL == pBitmapBits))
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

					std::vector<U16, alloc<U16> > dst;
					dst.reserve(this->FontHeight() * this->FontHeight());
					// 锁定表面，把 alpha 值写入纹理
					for (long y = charRect.top; y < charRect.bottom; ++ y)
					{
						for (long x = charRect.left; x < charRect.right; ++ x, ++ pBitmapBits)
						{
							dst.push_back(static_cast<U16>(*pBitmapBits) & 0xF000 | 0x0FFF);
						}
					}
					theTexture_->CopyMemoryToTexture(&dst[0], PF_A4R4G4B4, charRect.right - charRect.left,
						charRect.bottom - charRect.top, 0,
						charRect.left, charRect.top);

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
	RenderablePtr D3D9Font::RenderText(float sx, float sy, const Color& clr, 
		const WString& text, U32 flags)
	{
		return this->RenderText(sx, sy, 0.5f, 1, 1, clr, text, flags);
	}

	// 在指定位置画出放缩的文字
	/////////////////////////////////////////////////////////////////////////////////
	RenderablePtr D3D9Font::RenderText(float sx, float sy, float sz,
		float xScale, float yScale, const Color& clr,
		const WString& text, U32 flags)
	{
		if (text.empty())
		{
			return RenderablePtr(NULL);
		}

		this->UpdateTexture(text);

		U8 a, r, g, b;
		clr.RGBA(r, g, b, a);

		SharePtr<D3D9FontRenderable> ret(new D3D9FontRenderable(effect_));
		ret->RenderText(this->FontHeight(), charInfoMap_, sx, sy, sz, xScale, yScale,
			(a << 24) | (r << 16) | (g << 8) | b, text, flags);
		return ret;
	}
/*
	// 用3D方式画出文字
	/////////////////////////////////////////////////////////////////////////////////
	void C3DFont::RenderText(const WString& text, U32 flags)
	{
		if (text.empty())
		{
			return;
		}

		this->UpdateCharMap(wstrText);


		this->pd3dDevice_->CaptureStateBlock(this->SavedStateBlock_);
		this->pd3dDevice_->ApplyStateBlock(this->DrawTextStateBlock_);

		// 设置过滤属性
		if (dwFlags & FONT_FILTERED)
		{
			this->pd3dDevice_->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			this->pd3dDevice_->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		}

		if (dwFlags & FONT_TWOSIDED)
		{
			this->pd3dDevice_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		}

		const float h(static_cast<float>(this->GetFontHeight()));
		float x(0), y(0);
		vector<FONT3DVERTEX, alloc<FONT3DVERTEX> > Vert;
		Vert.reserve((wstrText.length() - std::count(wstrText.begin(), wstrText.end(), L'\n')) * 6);
		for (wstring::const_iterator iter = wstrText.begin(); iter != wstrText.end(); ++ iter)
		{
			const wchar_t& ch(*iter);
			const float w(static_cast<float>(charMap_[ch].width));

			if (ch != L'\n')
			{
				const Rect_T<float>& texRect(charMap_[ch].texRect);

				Vert.push_back(FONT3DVERTEX(x,		y,		0.9f, 0, 0, 1, texRect.left(),	texRect.top()));
				Vert.push_back(FONT3DVERTEX(x + w,	y,		0.9f, 0, 0, 1, texRect.right(),	texRect.top()));
				Vert.push_back(FONT3DVERTEX(x + w,	y + h,	0.9f, 0, 0, 1, texRect.right(),	texRect.bottom()));
				Vert.push_back(FONT3DVERTEX(x + w,	y + h,	0.9f, 0, 0, 1, texRect.right(),	texRect.bottom()));
				Vert.push_back(FONT3DVERTEX(x,		y + h,	0.9f, 0, 0, 1, texRect.left(),	texRect.bottom()));
				Vert.push_back(FONT3DVERTEX(x,		y,		0.9f, 0, 0, 1, texRect.left(),	texRect.top()));

				x += w;
			}
			else
			{
				y += h;
				x = 0;
			}
		}

		DrawVertices(this->pd3dDevice_, this->pCharTexture_, Vert);

		this->pd3dDevice_->ApplyStateBlock(this->SavedStateBlock_);
	}*/
}
