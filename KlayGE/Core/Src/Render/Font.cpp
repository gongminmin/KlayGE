// Font.cpp
// KlayGE Font类 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// RenderText速度增加50% (2010.3.9)
//
// 3.9.0
// 增加了KFontLoader (2009.10.16)
// kfont升级到2.0格式，支持LZMA压缩 (2009.12.13)
//
// 3.7.0
// 新的基于distance的字体格式 (2008.2.13)
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
#include <KlayGE/half.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Box.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/MapVector.hpp>
#include <KlayGE/LZMACodec.hpp>

#include <algorithm>
#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/Font.hpp>

using namespace std;

namespace KlayGE
{
#ifdef KLAYGE_PLATFORM_WINDOWS
	#pragma pack(push, 1)
#endif
	struct kfont_header
	{
		uint32_t fourcc;
		uint32_t version;
		uint32_t start_ptr;
		uint32_t validate_chars;
		uint32_t non_empty_chars;
		uint32_t char_size;

		int16_t base;
		int16_t scale;
	};
#ifdef KLAYGE_PLATFORM_WINDOWS
	#pragma pack(pop)
#endif

	KFontLoader::KFontLoader(std::string const & font_name)
	{
		ResIdentifierPtr kfont_input = ResLoader::Instance().Load(font_name);
		BOOST_ASSERT(kfont_input);

		kfont_header header;
		kfont_input->read(&header, sizeof(header));
		BOOST_ASSERT((MakeFourCC<'K', 'F', 'N', 'T'>::value == header.fourcc));
		BOOST_ASSERT((2 == header.version));

		char_size_ = header.char_size;
		dist_base_ = header.base;
		dist_scale_ = header.scale;

		kfont_input->seekg(header.start_ptr, std::ios_base::beg);

		std::vector<std::pair<int32_t, int32_t> > temp_char_index(header.non_empty_chars);
		kfont_input->read(&temp_char_index[0], static_cast<std::streamsize>(temp_char_index.size() * sizeof(temp_char_index[0])));
		std::vector<std::pair<int32_t, Vector_T<uint16_t, 2> > > temp_char_advance(header.validate_chars);
		kfont_input->read(&temp_char_advance[0], static_cast<std::streamsize>(temp_char_advance.size() * sizeof(temp_char_advance[0])));

		BOOST_FOREACH(BOOST_TYPEOF(temp_char_index)::reference ci, temp_char_index)
		{
			char_index_advance_.insert(std::make_pair(ci.first, std::make_pair(ci.second, Vector_T<uint16_t, 2>(0, 0))));
		}
		BOOST_FOREACH(BOOST_TYPEOF(temp_char_advance)::reference ca, temp_char_advance)
		{
			BOOST_AUTO(iter, char_index_advance_.find(ca.first));
			if (iter != char_index_advance_.end())
			{
				iter->second.second = ca.second;
			}
			else
			{
				char_index_advance_[ca.first] = std::make_pair(-1, ca.second);
			}
		}

		char_info_.resize(header.non_empty_chars);
		kfont_input->read(&char_info_[0], static_cast<std::streamsize>(char_info_.size() * sizeof(char_info_[0])));

		distances_addr_.resize(header.non_empty_chars + 1);

		std::vector<uint8_t> dist;
		for (uint32_t i = 0; i < header.non_empty_chars; ++ i)
		{
			distances_addr_[i] = distances_lzma_.size();

			uint64_t len;
			kfont_input->read(&len, sizeof(len));
			distances_lzma_.resize(static_cast<size_t>(distances_lzma_.size() + len));

			kfont_input->read(&distances_lzma_[distances_addr_[i]], static_cast<size_t>(len));
		}

		distances_addr_[header.non_empty_chars] = distances_lzma_.size();
	}

	uint32_t KFontLoader::CharSize() const
	{
		return char_size_;
	}

	int16_t KFontLoader::DistBase() const
	{
		return dist_base_;
	}

	int16_t KFontLoader::DistScale() const
	{
		return dist_scale_;
	}

	std::pair<int32_t, Vector_T<uint16_t, 2> > KFontLoader::CharIndexAdvance(wchar_t ch) const
	{
		BOOST_AUTO(iter, char_index_advance_.find(ch));
		if (iter != char_index_advance_.end())
		{
			return iter->second;
		}
		else
		{
			return std::make_pair(-1, Vector_T<uint16_t, 2>(0, 0));
		}
	}

	int32_t KFontLoader::CharIndex(wchar_t ch) const
	{
		return CharIndexAdvance(ch).first;
	}

	Vector_T<uint16_t, 2> KFontLoader::CharAdvance(wchar_t ch) const
	{
		return CharIndexAdvance(ch).second;
	}

	KFontLoader::font_info const & KFontLoader::CharInfo(int32_t offset) const
	{
		return char_info_[offset];
	}

	void KFontLoader::DistanceData(uint8_t* p, uint32_t pitch, int32_t offset) const
	{
		LZMACodec lzma_dec;
		std::vector<uint8_t> decoded;
		lzma_dec.Decode(decoded, &distances_lzma_[distances_addr_[offset]],
			distances_addr_[offset + 1] - distances_addr_[offset], char_size_ * char_size_);

		uint8_t const * char_data = &decoded[0];
		for (uint32_t y = 0; y < char_size_; ++ y)
		{
			std::memcpy(p, char_data, char_size_);
			p += pitch;
			char_data += char_size_;
		}
	}


	FontRenderable::FontRenderable(std::string const & font_name)
			: RenderableHelper(L"Font"),
                dirty_(false),
				curX_(0), curY_(0),
				three_dim_(false),
				kfont_loader_(font_name),
				tick_(0)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		restart_ = rf.RenderEngineInstance().DeviceCaps().primitive_restart_support;

		rl_ = rf.MakeRenderLayout();
		if (restart_)
		{
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);
		}
		else
		{
			rl_->TopologyType(RenderLayout::TT_TriangleList);
		}

		uint32_t const kfont_char_size = kfont_loader_.CharSize();

		RenderEngine const & renderEngine = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = renderEngine.DeviceCaps();
		dist_texture_ = rf.MakeTexture2D(std::min<uint32_t>(2048, caps.max_texture_width) / kfont_char_size * kfont_char_size,
			std::min<uint32_t>(2048, caps.max_texture_height) / kfont_char_size * kfont_char_size, 1, 1, EF_R8, 1, 0, EAH_GPU_Read, NULL);
		a_char_texture_ = rf.MakeTexture2D(kfont_char_size, kfont_char_size, 1, 1, EF_R8, 1, 0, EAH_CPU_Write, NULL);

		effect_ = rf.LoadEffect("Font.fxml");
		*(effect_->ParameterByName("distance_tex")) = dist_texture_;
		*(effect_->ParameterByName("distance_base_scale")) = float2(kfont_loader_.DistBase() / 32768.0f * 32 + 1, (kfont_loader_.DistScale() / 32768.0f + 1.0f) * 32);

		half_width_height_ep_ = effect_->ParameterByName("half_width_height");
		texel_to_pixel_offset_ep_ = effect_->ParameterByName("texel_to_pixel_offset");
		mvp_ep_ = effect_->ParameterByName("mvp");

		vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
		rl_->BindVertexStream(vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F),
										vertex_element(VEU_Diffuse, 0, EF_ABGR8),
										vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

		ib_ = rf.MakeIndexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
		rl_->BindIndexStream(ib_, EF_R16UI);

		box_ = Box(float3(0, 0, 0), float3(0, 0, 0));
	}

	RenderTechniquePtr const & FontRenderable::GetRenderTechnique() const
	{
		if (three_dim_)
		{
			return effect_->TechniqueByName("Font3DTec");
		}
		else
		{
			return effect_->TechniqueByName("Font2DTec");
		}
	}

	void FontRenderable::UpdateBuffers()
	{
		if (dirty_)
		{
			if (!vertices_.empty() && !indices_.empty())
			{
				vb_->Resize(static_cast<uint32_t>(vertices_.size() * sizeof(vertices_[0])));
				{
					GraphicsBuffer::Mapper mapper(*vb_, BA_Write_Only);
					std::copy(vertices_.begin(), vertices_.end(), mapper.Pointer<FontVert>());
				}

				ib_->Resize(static_cast<uint32_t>(indices_.size() * sizeof(indices_[0])));
				{
					GraphicsBuffer::Mapper mapper(*ib_, BA_Write_Only);
					std::copy(indices_.begin(), indices_.end(), mapper.Pointer<uint16_t>());
				}
			}

			dirty_ = false;
		}
	}

	void FontRenderable::OnRenderBegin()
	{
		if (!three_dim_)
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float const half_width = re.CurFrameBuffer()->Width() / 2.0f;
			float const half_height = re.CurFrameBuffer()->Height() / 2.0f;

			*half_width_height_ep_ = float2(half_width, half_height);

			float4 texel_to_pixel = re.TexelToPixelOffset();
			texel_to_pixel.x() /= half_width;
			texel_to_pixel.y() /= half_height;
			*texel_to_pixel_offset_ep_ = texel_to_pixel;
		}
	}

	void FontRenderable::OnRenderEnd()
	{
		vertices_.resize(0);
		indices_.resize(0);

		box_ = Box(float3(0, 0, 0), float3(0, 0, 0));
	}

	void FontRenderable::Render()
	{
		RenderEngine& renderEngine = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		this->OnRenderBegin();
		renderEngine.Render(*this->GetRenderTechnique(), *rl_);
		this->OnRenderEnd();
	}

	Size_T<uint32_t> FontRenderable::CalcSize(std::wstring const & text, uint32_t font_height)
	{
		this->UpdateTexture(text);

		BOOST_TYPEOF(kfont_loader_)& kl = kfont_loader_;

		float const rel_size = static_cast<float>(font_height) / kl.CharSize();

		std::vector<uint32_t> lines(1, 0);

		BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
		{
			if (ch != L'\n')
			{
				Vector_T<uint16_t, 2> advance = kl.CharAdvance(ch);
				lines.back() += static_cast<uint32_t>(advance.x() * rel_size);
			}
			else
			{
				lines.push_back(0);
			}
		}

		return Size_T<uint32_t>(*std::max_element(lines.begin(), lines.end()),
			static_cast<uint32_t>(font_height * lines.size()));
	}

	void FontRenderable::AddText2D(float sx, float sy, float sz,
		float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height)
	{
		three_dim_ = false;

		this->AddText(sx, sy, sz, xScale, yScale, clr, text, font_height);
	}

	void FontRenderable::AddText2D(Rect const & rc, float sz,
		float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height, uint32_t align)
	{
		three_dim_ = false;

		this->AddText(rc, sz, xScale, yScale, clr, text, font_height, align);
	}

	void FontRenderable::AddText3D(float4x4 const & mvp, Color const & clr, std::wstring const & text, uint32_t font_height)
	{
		three_dim_ = true;
		*mvp_ep_ = mvp;

		this->AddText(0, 0, 0, 1, 1, clr, text, font_height);
	}

	void FontRenderable::AddText(Rect const & rc, float sz,
		float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height, uint32_t align)
	{
		this->UpdateTexture(text);

		BOOST_TYPEOF(kfont_loader_)& kl = kfont_loader_;
		BOOST_TYPEOF(charInfoMap_)& cim = charInfoMap_;
		BOOST_TYPEOF(vertices_)& verts = vertices_;
		BOOST_TYPEOF(indices_)& inds = indices_;

		float const h = font_height * yScale;
		float const rel_size = static_cast<float>(font_height) / kl.CharSize();
		float const rel_size_x = rel_size * xScale;
		float const rel_size_y = rel_size * yScale;

		std::vector<std::pair<float, std::wstring> > lines(1, std::make_pair(0.0f, L""));

		BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
		{
			if (ch != L'\n')
			{
				Vector_T<uint16_t, 2> advance = kl.CharAdvance(ch);
				lines.back().first += advance.x() * rel_size * xScale;
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
					sx.push_back(rc.right() - p.first);
				}
			}
			else
			{
				// Font::FA_Hor_Center
				BOOST_FOREACH(BOOST_TYPEOF(lines)::const_reference p, lines)
				{
					sx.push_back((rc.left() + rc.right()) / 2 - p.first / 2);
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
					sy.push_back((rc.top() + rc.bottom()) / 2
						- lines.size() * h / 2 + (iter - lines.begin()) * h);
				}
			}
		}

		int index_per_char;
		if (restart_)
		{
			index_per_char = 5;
		}
		else
		{
			index_per_char = 6;
		}

		dirty_ = true;

		uint32_t const clr32 = clr.ABGR();
		for (size_t i = 0; i < sx.size(); ++ i)
		{
			size_t const maxSize = lines[i].second.length();
			float x = sx[i], y = sy[i];

			verts.reserve(verts.size() + maxSize * 4);
			inds.reserve(inds.size() + maxSize * index_per_char);

			uint16_t lastIndex(static_cast<uint16_t>(verts.size()));

			BOOST_FOREACH(BOOST_TYPEOF(lines[i].second)::const_reference ch, lines[i].second)
			{
				std::pair<int32_t, Vector_T<uint16_t, 2> > offset_adv = kl.CharIndexAdvance(ch);
				if (offset_adv.first != -1)
				{
					KFontLoader::font_info const & ci = kl.CharInfo(offset_adv.first);

					float left = ci.left * rel_size_x;
					float top = ci.top * rel_size_y;
					float width = ci.width * rel_size_x;
					float height = ci.height * rel_size_y;

					BOOST_AUTO(cmiter, cim.find(ch));
					Rect_T<float> const & texRect(cmiter->second.rc);

					Rect_T<float> pos_rc(x + left, y + top, x + left + width, y + top + height);
					Rect_T<float> intersect_rc = pos_rc & rc;
					if ((intersect_rc.Width() > 0) && (intersect_rc.Height() > 0))
					{
						verts.push_back(FontVert(float3(pos_rc.left(), pos_rc.top(), sz),
												clr32,
												float2(texRect.left(), texRect.top())));
						verts.push_back(FontVert(float3(pos_rc.right(), pos_rc.top(), sz),
												clr32,
												float2(texRect.right(), texRect.top())));
						verts.push_back(FontVert(float3(pos_rc.right(), pos_rc.bottom(), sz),
												clr32,
												float2(texRect.right(), texRect.bottom())));
						verts.push_back(FontVert(float3(pos_rc.left(), pos_rc.bottom(), sz),
												clr32,
												float2(texRect.left(), texRect.bottom())));

						inds.push_back(lastIndex + 0);
						inds.push_back(lastIndex + 1);
						if (restart_)
						{
							inds.push_back(lastIndex + 3);
							inds.push_back(lastIndex + 2);
							inds.push_back(0xFFFF);
						}
						else
						{
							inds.push_back(lastIndex + 2);
							inds.push_back(lastIndex + 2);
							inds.push_back(lastIndex + 3);
							inds.push_back(lastIndex + 0);
						}
						lastIndex += 4;
					}
				}

				x += offset_adv.second.x() * rel_size_x;
				y += offset_adv.second.y() * rel_size_y;
			}

			box_ |= Box(float3(sx[i], sy[i], sz), float3(sx[i] + lines[i].first, sy[i] + h, sz + 0.1f));
		}
	}

	void FontRenderable::AddText(float sx, float sy, float sz,
		float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height)
	{
		this->UpdateTexture(text);

		BOOST_TYPEOF(kfont_loader_)& kl = kfont_loader_;
		BOOST_TYPEOF(charInfoMap_)& cim = charInfoMap_;
		BOOST_TYPEOF(vertices_)& verts = vertices_;
		BOOST_TYPEOF(indices_)& inds = indices_;

		uint32_t const clr32 = clr.ABGR();
		float const h = font_height * yScale;
		float const rel_size = static_cast<float>(font_height) / kl.CharSize();
		float const rel_size_x = rel_size * xScale;
		float const rel_size_y = rel_size * yScale;
		size_t const maxSize = text.length() - std::count(text.begin(), text.end(), L'\n');
		float x = sx, y = sy;
		float maxx = sx, maxy = sy;

		int index_per_char;
		if (restart_)
		{
			index_per_char = 5;
		}
		else
		{
			index_per_char = 6;
		}

		dirty_ = true;

		verts.reserve(verts.size() + maxSize * 4);
		inds.reserve(inds.size() + maxSize * index_per_char);

		uint16_t lastIndex(static_cast<uint16_t>(verts.size()));

		BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
		{
			if (ch != L'\n')
			{
				std::pair<int32_t, Vector_T<uint16_t, 2> > offset_adv = kl.CharIndexAdvance(ch);
				if (offset_adv.first != -1)
				{
					KFontLoader::font_info const & ci = kl.CharInfo(offset_adv.first);

					float left = ci.left * rel_size_x;
					float top = ci.top * rel_size_y;
					float width = ci.width * rel_size_x;
					float height = ci.height * rel_size_y;

					BOOST_AUTO(cmiter, cim.find(ch));
					Rect_T<float> const & texRect(cmiter->second.rc);
					Rect_T<float> pos_rc(x + left, y + top, x + left + width, y + top + height);

					verts.push_back(FontVert(float3(pos_rc.left(), pos_rc.top(), sz),
											clr32,
											float2(texRect.left(), texRect.top())));
					verts.push_back(FontVert(float3(pos_rc.right(), pos_rc.top(), sz),
											clr32,
											float2(texRect.right(), texRect.top())));
					verts.push_back(FontVert(float3(pos_rc.right(), pos_rc.bottom(), sz),
											clr32,
											float2(texRect.right(), texRect.bottom())));
					verts.push_back(FontVert(float3(pos_rc.left(), pos_rc.bottom(), sz),
											clr32,
											float2(texRect.left(), texRect.bottom())));

					inds.push_back(lastIndex + 0);
					inds.push_back(lastIndex + 1);
					if (restart_)
					{
						inds.push_back(lastIndex + 3);
						inds.push_back(lastIndex + 2);
						inds.push_back(0xFFFF);
					}
					else
					{
						inds.push_back(lastIndex + 2);
						inds.push_back(lastIndex + 2);
						inds.push_back(lastIndex + 3);
						inds.push_back(lastIndex + 0);
					}
					lastIndex += 4;
				}

				x += offset_adv.second.x() * rel_size_x;
				y += offset_adv.second.y() * rel_size_y;

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
	void FontRenderable::UpdateTexture(std::wstring const & text)
	{
		++ tick_;

		uint32_t const tex_width = dist_texture_->Width(0);
		uint32_t const tex_height = dist_texture_->Height(0);

		BOOST_TYPEOF(kfont_loader_)& kl = kfont_loader_;
		BOOST_TYPEOF(charInfoMap_)& cim = charInfoMap_;

		BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
		{
			int32_t offset = kl.CharIndex(ch);
			if (offset != -1)
			{
				BOOST_AUTO(cmiter, cim.find(ch));
				if (cmiter != cim.end())
				{
					// 在现有纹理中找到了

					cmiter->second.tick = tick_;
				}
				else
				{
					// 在现有纹理中找不到，所以得在现有纹理中添加新字

					KFontLoader::font_info const & ci = kl.CharInfo(offset);
					uint32_t const kfont_char_size = kl.CharSize();

					uint32_t width = ci.width;
					uint32_t height = ci.height;

					Vector_T<int32_t, 2> char_pos;
					CharInfo charInfo;
					if (curX_ + kfont_char_size >= tex_width)
					{
						curX_ = 0;
						curY_ += kfont_char_size;
					}
					if ((curX_ < tex_width) && (curY_ < tex_height) && (curY_ + kfont_char_size < tex_height))
					{
						// 纹理还有空间
						char_pos = Vector_T<int32_t, 2>(curX_, curY_);

						curX_ += kfont_char_size;
					}
					else
					{
						// 找到使用最长时间没有使用的字

						uint64_t min_tick = cim.begin()->second.tick;
						BOOST_AUTO(min_chiter, cim.begin());
						for (BOOST_AUTO(chiter, cim.begin()); chiter != cim.end(); ++ chiter)
						{
							if (chiter->second.tick < min_tick)
							{
								min_tick = chiter->second.tick;
								min_chiter = chiter;
							}
						}

						char_pos.x() = static_cast<int32_t>(min_chiter->second.rc.left() * tex_width);
						char_pos.y() = static_cast<int32_t>(min_chiter->second.rc.top() * tex_height);

						cim.erase(min_chiter);
					}

					charInfo.rc.left()		= static_cast<float>(char_pos.x()) / tex_width;
					charInfo.rc.top()		= static_cast<float>(char_pos.y()) / tex_height;
					charInfo.rc.right()		= charInfo.rc.left() + static_cast<float>(width) / tex_width;
					charInfo.rc.bottom()	= charInfo.rc.top() + static_cast<float>(height) / tex_height;
					charInfo.tick			= tick_;

					{
						Texture::Mapper mapper(*a_char_texture_, 0, TMA_Write_Only,
							0, 0, kfont_char_size, kfont_char_size);
						kl.DistanceData(mapper.Pointer<uint8_t>(), mapper.RowPitch(), offset);
					}

					a_char_texture_->CopyToTexture2D(*dist_texture_, 0,
						kfont_char_size, kfont_char_size, char_pos.x(), char_pos.y(),
						kfont_char_size, kfont_char_size, 0, 0);

					cim.insert(std::make_pair(ch, charInfo));
				}
			}
		}
	}

	class FontObject : public SceneObjectHelper
	{
	public:
		FontObject(RenderablePtr const & renderable, uint32_t attrib)
			: SceneObjectHelper(renderable, attrib)
		{
		}

		void Update()
		{
			checked_pointer_cast<FontRenderable>(renderable_)->UpdateBuffers();
		}
	};


	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	Font::Font(RenderablePtr const & font_renderable, uint32_t flags)
			: font_renderable_(font_renderable)
	{
		fso_attrib_ = SceneObject::SOA_Overlay;
		if (flags & Font::FS_Cullable)
		{
			fso_attrib_ |= SceneObject::SOA_Cullable;
		}
	}

	// 计算文字大小
	/////////////////////////////////////////////////////////////////////////////////
	Size_T<uint32_t> Font::CalcSize(std::wstring const & text, uint32_t font_size)
	{
		if (!text.empty())
		{
			return checked_pointer_cast<FontRenderable>(font_renderable_)->CalcSize(text, font_size);
		}
		else
		{
			return Size_T<int32_t>(0, 0);
		}
	}

	// 在指定位置画出文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float sx, float sy, Color const & clr,
		std::wstring const & text, uint32_t font_size)
	{
		this->RenderText(sx, sy, 0, 1, 1, clr, text, font_size);
	}

	// 在指定位置画出放缩的文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float x, float y, float z,
		float xScale, float yScale, Color const & clr,
		std::wstring const & text, uint32_t font_size)
	{
		if (!text.empty())
		{
			boost::shared_ptr<FontObject> font_obj = MakeSharedPtr<FontObject>(font_renderable_, fso_attrib_);
			checked_pointer_cast<FontRenderable>(font_renderable_)->AddText2D(
				x, y, z, xScale, yScale, clr, text, font_size);
			font_obj->AddToSceneManager();
		}
	}

	// 在指定矩形区域内画出放缩的文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(Rect const & rc, float z,
		float xScale, float yScale, Color const & clr,
		std::wstring const & text, uint32_t font_size, uint32_t align)
	{
		if (!text.empty())
		{
			boost::shared_ptr<FontObject> font_obj = MakeSharedPtr<FontObject>(font_renderable_, fso_attrib_);
			checked_pointer_cast<FontRenderable>(font_renderable_)->AddText2D(
				rc, z, xScale, yScale, clr, text, font_size, align);
			font_obj->AddToSceneManager();
		}
	}

	// 在指定位置画出3D的文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float4x4 const & mvp, Color const & clr, std::wstring const & text, uint32_t font_size)
	{
		if (!text.empty())
		{
			boost::shared_ptr<FontObject> font_obj = MakeSharedPtr<FontObject>(font_renderable_, fso_attrib_);
			checked_pointer_cast<FontRenderable>(font_renderable_)->AddText3D(mvp, clr, text, font_size);
			font_obj->AddToSceneManager();
		}
	}
}
