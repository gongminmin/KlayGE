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
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/Half.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KFL/AABBox.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/LZMACodec.hpp>

#include <algorithm>
#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include <boost/assert.hpp>
#include <boost/functional/hash.hpp>

#include <kfont/kfont.hpp>

#include <KlayGE/Font.hpp>

namespace KlayGE
{
	class FontRenderable : public RenderableHelper
	{
	public:
		explicit FontRenderable(shared_ptr<KFont> const & kfl)
				: RenderableHelper(L"Font"),
					dirty_(false),
					three_dim_(false),
					kfont_loader_(kfl),
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

			uint32_t const kfont_char_size = kfont_loader_->CharSize();

			RenderEngine const & renderEngine = rf.RenderEngineInstance();
			RenderDeviceCaps const & caps = renderEngine.DeviceCaps();
			uint32_t size = std::min<uint32_t>(2048U, std::min<uint32_t>(caps.max_texture_width, caps.max_texture_height)) / kfont_char_size * kfont_char_size;
			dist_texture_ = rf.MakeTexture2D(size, size, 1, 1, EF_R8, 1, 0, EAH_GPU_Read, nullptr);
			a_char_texture_ = rf.MakeTexture2D(kfont_char_size, kfont_char_size, 1, 1, EF_R8, 1, 0, EAH_CPU_Write, nullptr);

			char_free_list_.push_back(std::make_pair(0, size * size / kfont_char_size / kfont_char_size));

			effect_ = SyncLoadRenderEffect("Font.fxml");
			*(effect_->ParameterByName("distance_tex")) = dist_texture_;
			*(effect_->ParameterByName("distance_base_scale")) = float2(kfont_loader_->DistBase() / 32768.0f * 32 + 1, (kfont_loader_->DistScale() / 32768.0f + 1.0f) * 32);

			half_width_height_ep_ = effect_->ParameterByName("half_width_height");
			mvp_ep_ = effect_->ParameterByName("mvp");

			vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, nullptr);
			rl_->BindVertexStream(vb_, make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F),
											vertex_element(VEU_Diffuse, 0, EF_ABGR8),
											vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			ib_ = rf.MakeIndexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, nullptr);
			rl_->BindIndexStream(ib_, EF_R16UI);

			pos_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		}

		RenderTechniquePtr const & GetRenderTechnique() const
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

		void UpdateBuffers()
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

		void OnRenderBegin()
		{
			if (!three_dim_)
			{
				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				float const half_width = re.CurFrameBuffer()->Width() / 2.0f;
				float const half_height = re.CurFrameBuffer()->Height() / 2.0f;

				*half_width_height_ep_ = float2(half_width, half_height);
			}
		}

		void OnRenderEnd()
		{
			vertices_.resize(0);
			indices_.resize(0);

			pos_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		}

		void Render()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			this->OnRenderBegin();
			re.Render(*this->GetRenderTechnique(), *rl_);
			this->OnRenderEnd();
		}

		Size_T<float> CalcSize(std::wstring const & text, float font_size)
		{
			this->UpdateTexture(text);

			KFont& kl = *kfont_loader_;

			float const rel_size = font_size / kl.CharSize();

			std::vector<float> lines(1, 0);

			typedef KLAYGE_DECLTYPE(text) TextType;
			KLAYGE_FOREACH(TextType::const_reference ch, text)
			{
				if (ch != L'\n')
				{
					uint32_t advance = kl.CharAdvance(ch);
					lines.back() += (advance & 0xFFFF) * rel_size;
				}
				else
				{
					lines.push_back(0);
				}
			}

			return Size_T<float>(*std::max_element(lines.begin(), lines.end()),
				font_size * lines.size());
		}

		void AddText2D(float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, float font_size)
		{
			three_dim_ = false;

			this->AddText(sx, sy, sz, xScale, yScale, clr, text, font_size);
		}

		void AddText2D(Rect const & rc, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, float font_size, uint32_t align)
		{
			three_dim_ = false;

			this->AddText(rc, sz, xScale, yScale, clr, text, font_size, align);
		}

		void AddText3D(float4x4 const & mvp, Color const & clr, std::wstring const & text, float font_size)
		{
			three_dim_ = true;
			*mvp_ep_ = mvp;

			this->AddText(0, 0, 0, 1, 1, clr, text, font_size);
		}

	private:
		void AddText(Rect const & rc, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, float font_size, uint32_t align)
		{
			this->UpdateTexture(text);

			KFont& kl = *kfont_loader_;
			KLAYGE_DECLTYPE(char_info_map_)& cim = char_info_map_;
			KLAYGE_DECLTYPE(vertices_)& verts = vertices_;
			KLAYGE_DECLTYPE(indices_)& inds = indices_;

			float const h = font_size * yScale;
			float const rel_size = font_size / kl.CharSize();
			float const rel_size_x = rel_size * xScale;
			float const rel_size_y = rel_size * yScale;

			std::vector<std::pair<float, std::wstring> > lines(1, std::make_pair(0.0f, L""));

			typedef KLAYGE_DECLTYPE(text) TextType;
			KLAYGE_FOREACH(TextType::const_reference ch, text)
			{
				if (ch != L'\n')
				{
					uint32_t advance = kl.CharAdvance(ch);
					lines.back().first += (advance & 0xFFFF) * rel_size * xScale;
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
					typedef KLAYGE_DECLTYPE(lines) LinesType;
					KLAYGE_FOREACH(LinesType::const_reference p, lines)
					{
						sx.push_back(rc.right() - p.first);
					}
				}
				else
				{
					// Font::FA_Hor_Center
					typedef KLAYGE_DECLTYPE(lines) LinesType;
					KLAYGE_FOREACH(LinesType::const_reference p, lines)
					{
						sx.push_back((rc.left() + rc.right()) / 2 - p.first / 2);
					}
				}
			}

			if (align & Font::FA_Ver_Top)
			{
				for (KLAYGE_AUTO(iter, lines.begin()); iter != lines.end(); ++ iter)
				{
					sy.push_back(rc.top() + (iter - lines.begin()) * h);
				}
			}
			else
			{
				if (align & Font::FA_Ver_Bottom)
				{
					for (KLAYGE_AUTO(iter, lines.begin()); iter != lines.end(); ++ iter)
					{
						sy.push_back(rc.bottom() - (lines.size() - (iter - lines.begin())) * h);
					}
				}
				else
				{
					// Font::FA_Ver_Middle
					for (KLAYGE_AUTO(iter, lines.begin()); iter != lines.end(); ++ iter)
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

				typedef KLAYGE_DECLTYPE(lines[i].second) LinesType;
				KLAYGE_FOREACH(LinesType::const_reference ch, lines[i].second)
				{
					std::pair<int32_t, uint32_t> const & offset_adv = kl.CharIndexAdvance(ch);
					if (offset_adv.first != -1)
					{
						KFont::font_info const & ci = kl.CharInfo(offset_adv.first);

						float left = ci.left * rel_size_x;
						float top = ci.top * rel_size_y;
						float width = ci.width * rel_size_x;
						float height = ci.height * rel_size_y;

						KLAYGE_AUTO(cmiter, cim.find(ch));
						Rect const & texRect(cmiter->second.rc);

						Rect pos_rc(x + left, y + top, x + left + width, y + top + height);
						Rect intersect_rc = pos_rc & rc;
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

					x += (offset_adv.second & 0xFFFF) * rel_size_x;
					y += (offset_adv.second >> 16) * rel_size_y;
				}

				pos_aabb_ |= AABBox(float3(sx[i], sy[i], sz), float3(sx[i] + lines[i].first, sy[i] + h, sz + 0.1f));
			}
		}

		void AddText(float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, float font_size)
		{
			this->UpdateTexture(text);

			KFont& kl = *kfont_loader_;
			KLAYGE_DECLTYPE(char_info_map_)& cim = char_info_map_;
			KLAYGE_DECLTYPE(vertices_)& verts = vertices_;
			KLAYGE_DECLTYPE(indices_)& inds = indices_;

			uint32_t const clr32 = clr.ABGR();
			float const h = font_size * yScale;
			float const rel_size = font_size / kl.CharSize();
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

			typedef KLAYGE_DECLTYPE(text) TextType;
			KLAYGE_FOREACH(TextType::const_reference ch, text)
			{
				if (ch != L'\n')
				{
					std::pair<int32_t, uint32_t> const & offset_adv = kl.CharIndexAdvance(ch);
					if (offset_adv.first != -1)
					{
						KFont::font_info const & ci = kl.CharInfo(offset_adv.first);

						float left = ci.left * rel_size_x;
						float top = ci.top * rel_size_y;
						float width = ci.width * rel_size_x;
						float height = ci.height * rel_size_y;

						KLAYGE_AUTO(cmiter, cim.find(ch));
						Rect const & texRect(cmiter->second.rc);
						Rect pos_rc(x + left, y + top, x + left + width, y + top + height);

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

					x += (offset_adv.second & 0xFFFF) * rel_size_x;
					y += (offset_adv.second >> 16) * rel_size_y;

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

			pos_aabb_ |= AABBox(float3(sx, sy, sz), float3(maxx, maxy, sz + 0.1f));
		}

		// 更新纹理，使用LRU算法
		/////////////////////////////////////////////////////////////////////////////////
		void UpdateTexture(std::wstring const & text)
		{
			++ tick_;

			uint32_t const tex_size = dist_texture_->Width(0);

			KFont& kl = *kfont_loader_;
			KLAYGE_DECLTYPE(char_info_map_)& cim = char_info_map_;

			uint32_t const kfont_char_size = kl.CharSize();

			uint32_t const num_chars_a_row = tex_size / kfont_char_size;
			uint32_t const num_total_chars = num_chars_a_row * num_chars_a_row;

			typedef KLAYGE_DECLTYPE(text) TextType;
			KLAYGE_FOREACH(TextType::const_reference ch, text)
			{
				int32_t offset = kl.CharIndex(ch);
				if (offset != -1)
				{
					KLAYGE_AUTO(cmiter, cim.find(ch));
					if (cmiter != cim.end())
					{
						// 在现有纹理中找到了

						cmiter->second.tick = tick_;
					}
					else
					{
						// 在现有纹理中找不到，所以得在现有纹理中添加新字

						KFont::font_info const & ci = kl.CharInfo(offset);

						uint32_t width = ci.width;
						uint32_t height = ci.height;

						int2 char_pos;
						CharInfo charInfo;
						if (cim.size() < num_total_chars)
						{
							// 纹理还有空间

							uint32_t const s = char_free_list_.front().first;
						
							char_pos.y() = s / num_chars_a_row;
							char_pos.x() = s - char_pos.y() * num_chars_a_row;

							char_pos.x() *= kfont_char_size;
							char_pos.y() *= kfont_char_size;

							charInfo.rc.left() = static_cast<float>(char_pos.x()) / tex_size;
							charInfo.rc.top() = static_cast<float>(char_pos.y()) / tex_size;

							++ char_free_list_.front().first;
							if (char_free_list_.front().first == char_free_list_.front().second)
							{
								char_free_list_.pop_front();
							}
						}
						else
						{
							// 找到使用最长时间没有使用的字

							uint64_t min_tick = cim.begin()->second.tick;
							KLAYGE_AUTO(min_chiter, cim.begin());
							for (KLAYGE_AUTO(chiter, cim.begin()); chiter != cim.end(); ++ chiter)
							{
								if (chiter->second.tick < min_tick)
								{
									min_tick = chiter->second.tick;
									min_chiter = chiter;
								}
							}

							char_pos.x() = static_cast<int32_t>(min_chiter->second.rc.left() * tex_size);
							char_pos.y() = static_cast<int32_t>(min_chiter->second.rc.top() * tex_size);
							charInfo.rc.left() = min_chiter->second.rc.left();
							charInfo.rc.top() = min_chiter->second.rc.top();

							for (KLAYGE_AUTO(chiter, cim.begin()); chiter != cim.end(); ++ chiter)
							{
								if (chiter->second.tick == min_tick)
								{
									cim.erase(chiter);

									uint32_t const x = static_cast<int32_t>(chiter->second.rc.left() * tex_size);
									uint32_t const y = static_cast<int32_t>(chiter->second.rc.top() * tex_size);
									uint32_t const id = y * num_chars_a_row + x;
									KLAYGE_AUTO(freeiter, char_free_list_.begin());
									while ((freeiter != char_free_list_.end()) && (freeiter->second <= id))
									{
										++ freeiter;
									}
									char_free_list_.insert(freeiter, std::make_pair(id, id + 1));
								}
							}
							for (KLAYGE_AUTO(freeiter, char_free_list_.begin()); freeiter != char_free_list_.end();)
							{
								KLAYGE_AUTO(nextiter, freeiter);
								++ nextiter;

								if (freeiter->second == nextiter->first)
								{
									freeiter->second = nextiter->second;
									char_free_list_.erase(nextiter);
								}
								else
								{
									++ freeiter;
								}
							}
						}

						charInfo.rc.right()		= charInfo.rc.left() + static_cast<float>(width) / tex_size;
						charInfo.rc.bottom()	= charInfo.rc.top() + static_cast<float>(height) / tex_size;
						charInfo.tick			= tick_;

						{
							Texture::Mapper mapper(*a_char_texture_, 0, 0, TMA_Write_Only,
								0, 0, kfont_char_size, kfont_char_size);
							kl.GetDistanceData(mapper.Pointer<uint8_t>(), mapper.RowPitch(), offset);
						}

						a_char_texture_->CopyToSubTexture2D(*dist_texture_,
							0, 0, char_pos.x(), char_pos.y(), kfont_char_size, kfont_char_size,
							0, 0, 0, 0, kfont_char_size, kfont_char_size);

						cim.insert(std::make_pair(ch, charInfo));
					}
				}
			}
		}

	private:
		struct CharInfo
		{
			Rect rc;
			uint64_t tick;
		};

#ifdef KLAYGE_HAS_STRUCT_PACK
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
#ifdef KLAYGE_HAS_STRUCT_PACK
	#pragma pack(pop)
#endif

		bool restart_;
		bool dirty_;

		unordered_map<wchar_t, CharInfo> char_info_map_;
		std::list<std::pair<uint32_t, uint32_t> > char_free_list_;

		bool three_dim_;

		std::vector<FontVert>	vertices_;
		std::vector<uint16_t>	indices_;

		GraphicsBufferPtr vb_;
		GraphicsBufferPtr ib_;

		TexturePtr		dist_texture_;
		TexturePtr		a_char_texture_;
		RenderEffectPtr	effect_;

		RenderEffectParameterPtr half_width_height_ep_;
		RenderEffectParameterPtr mvp_ep_;

		shared_ptr<KFont> kfont_loader_;

		uint64_t tick_;
	};

	class FontObject : public SceneObjectHelper
	{
	public:
		FontObject(RenderablePtr const & renderable, uint32_t attrib)
			: SceneObjectHelper(renderable, attrib)
		{
		}

		void MainThreadUpdate(float /*app_time*/, float /*elapsed_time*/)
		{
			checked_pointer_cast<FontRenderable>(renderable_)->UpdateBuffers();
		}
	};
}

namespace
{
	using namespace KlayGE;

	class FontLoadingDesc : public ResLoadingDesc
	{
	private:
		struct FontDesc
		{
			std::string res_name;
			uint32_t flag;

			shared_ptr<KFont> kfont_loader;
		};

	public:
		FontLoadingDesc(std::string const & res_name, uint32_t flag)
		{
			font_desc_.res_name = res_name;
			font_desc_.flag = flag;
			font_desc_.kfont_loader = MakeSharedPtr<KFont>();
		}

		uint64_t Type() const
		{
			static uint64_t const type = static_cast<uint64_t>(boost::hash_value("FontLoadingDesc"));
			return type;
		}

		bool StateLess() const
		{
			return true;
		}

		void SubThreadStage()
		{
			ResIdentifierPtr kfont_input = ResLoader::Instance().Open(font_desc_.res_name);
			font_desc_.kfont_loader->Load(kfont_input->input_stream());
		}

		shared_ptr<void> MainThreadStage()
		{
			shared_ptr<FontRenderable> fr = MakeSharedPtr<FontRenderable>(font_desc_.kfont_loader);
			return static_pointer_cast<void>(MakeSharedPtr<Font>(fr, font_desc_.flag));
		}

		bool HasSubThreadStage() const
		{
			return true;
		}

		bool Match(ResLoadingDesc const & rhs) const
		{
			if (this->Type() == rhs.Type())
			{
				FontLoadingDesc const & fld = static_cast<FontLoadingDesc const &>(rhs);
				return (font_desc_.res_name == fld.font_desc_.res_name)
					&& (font_desc_.flag == fld.font_desc_.flag);
			}
			return false;
		}

		void CopyDataFrom(ResLoadingDesc const & rhs)
		{
			BOOST_ASSERT(this->Type() == rhs.Type());

			FontLoadingDesc const & fld = static_cast<FontLoadingDesc const &>(rhs);
			font_desc_.res_name = fld.font_desc_.res_name;
			font_desc_.flag = fld.font_desc_.flag;
			font_desc_.kfont_loader = fld.font_desc_.kfont_loader;
		}

		shared_ptr<void> CloneResourceFrom(shared_ptr<void> const & resource)
		{
			return resource;
		}

	private:
		FontDesc font_desc_;
	};
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	Font::Font(shared_ptr<FontRenderable> const & fr)
			: font_renderable_(fr)
	{
		fso_attrib_ = SceneObject::SOA_Overlay;
	}

	Font::Font(shared_ptr<FontRenderable> const & fr, uint32_t flags)
			: font_renderable_(fr)
	{
		fso_attrib_ = SceneObject::SOA_Overlay;
		if (flags & Font::FS_Cullable)
		{
			fso_attrib_ |= SceneObject::SOA_Cullable;
		}
	}

	// 计算文字大小
	/////////////////////////////////////////////////////////////////////////////////
	Size_T<float> Font::CalcSize(std::wstring const & text, float font_size)
	{
		if (!text.empty())
		{
			return font_renderable_->CalcSize(text, font_size);
		}
		else
		{
			return Size_T<float>(0, 0);
		}
	}

	// 在指定位置画出文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float sx, float sy, Color const & clr,
		std::wstring const & text, float font_size)
	{
		this->RenderText(sx, sy, 0, 1, 1, clr, text, font_size);
	}

	// 在指定位置画出放缩的文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float x, float y, float z,
		float xScale, float yScale, Color const & clr,
		std::wstring const & text, float font_size)
	{
		if (!text.empty())
		{
			shared_ptr<FontObject> font_obj = MakeSharedPtr<FontObject>(font_renderable_, fso_attrib_);
			font_renderable_->AddText2D(x, y, z, xScale, yScale, clr, text, font_size);
			font_obj->AddToSceneManager();
		}
	}

	// 在指定矩形区域内画出放缩的文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(Rect const & rc, float z,
		float xScale, float yScale, Color const & clr,
		std::wstring const & text, float font_size, uint32_t align)
	{
		if (!text.empty())
		{
			shared_ptr<FontObject> font_obj = MakeSharedPtr<FontObject>(font_renderable_, fso_attrib_);
			font_renderable_->AddText2D(rc, z, xScale, yScale, clr, text, font_size, align);
			font_obj->AddToSceneManager();
		}
	}

	// 在指定位置画出3D的文字
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float4x4 const & mvp, Color const & clr, std::wstring const & text, float font_size)
	{
		if (!text.empty())
		{
			shared_ptr<FontObject> font_obj = MakeSharedPtr<FontObject>(font_renderable_, fso_attrib_);
			font_renderable_->AddText3D(mvp, clr, text, font_size);
			font_obj->AddToSceneManager();
		}
	}


	FontPtr SyncLoadFont(std::string const & font_name, uint32_t flags)
	{
		return ResLoader::Instance().SyncQueryT<Font>(MakeSharedPtr<FontLoadingDesc>(font_name, flags));
	}

	function<FontPtr()> ASyncLoadFont(std::string const & font_name, uint32_t flags)
	{
		return ResLoader::Instance().ASyncQueryT<Font>(MakeSharedPtr<FontLoadingDesc>(font_name, flags));
	}
}
