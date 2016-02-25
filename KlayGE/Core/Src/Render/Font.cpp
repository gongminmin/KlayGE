/**
 * @file Font.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

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
#include <KlayGE/TransientBuffer.hpp>

#include <algorithm>
#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <tuple>
#include <type_traits>
#include <boost/assert.hpp>
#include <boost/functional/hash.hpp>

#include <kfont/kfont.hpp>

#include <KlayGE/Font.hpp>

namespace KlayGE
{
	class FontRenderable : public RenderableHelper
	{
	public:
		explicit FontRenderable(std::shared_ptr<KFont> const & kfl)
				: RenderableHelper(L"Font"),
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

			char_free_list_.emplace_back(0, size * size / kfont_char_size / kfont_char_size);

			effect_ = SyncLoadRenderEffect("Font.fxml");
			*(effect_->ParameterByName("distance_tex")) = dist_texture_;
			*(effect_->ParameterByName("distance_base_scale")) = float2(kfont_loader_->DistBase() / 32768.0f * 32 + 1, (kfont_loader_->DistScale() / 32768.0f + 1.0f) * 32);

			half_width_height_ep_ = effect_->ParameterByName("half_width_height");
			mvp_ep_ = effect_->ParameterByName("mvp");

			uint32_t const INDEX_PER_CHAR = restart_ ? 5 : 6;
			uint32_t const INIT_NUM_CHAR = 1024;
			tb_vb_ = MakeSharedPtr<TransientBuffer>(static_cast<uint32_t>(INIT_NUM_CHAR * 4 * sizeof(FontVert)), TransientBuffer::BF_Vertex);
			tb_ib_ = MakeSharedPtr<TransientBuffer>(static_cast<uint32_t>(INIT_NUM_CHAR * INDEX_PER_CHAR * sizeof(uint16_t)), TransientBuffer::BF_Index);

			rl_->BindVertexStream(tb_vb_->GetBuffer(), std::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F),
											vertex_element(VEU_Diffuse, 0, EF_ABGR8),
											vertex_element(VEU_TextureCoord, 0, EF_GR32F)));
			rl_->BindIndexStream(tb_ib_->GetBuffer(), EF_R16UI);

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

		void OnRenderBegin()
		{
			if (!three_dim_)
			{
				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				float const half_width = re.CurFrameBuffer()->Width() / 2.0f;
				float const half_height = re.CurFrameBuffer()->Height() / 2.0f;

				*half_width_height_ep_ = float2(half_width, half_height);
			}

			tb_vb_->EnsureDataReady();
			tb_ib_->EnsureDataReady();

			rl_->SetVertexStream(0, tb_vb_->GetBuffer());
			rl_->BindIndexStream(tb_ib_->GetBuffer(), EF_R16UI);
		}

		void OnRenderEnd()
		{
			pos_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));

			tb_vb_sub_allocs_.clear();
			tb_ib_sub_allocs_.clear();

			tb_vb_->OnPresent();
			tb_ib_->OnPresent();
		}

		void Render()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			this->OnRenderBegin();

			BOOST_ASSERT(tb_vb_sub_allocs_.size() == tb_ib_sub_allocs_.size());

			for (size_t i = 0; i < tb_vb_sub_allocs_.size(); ++ i)
			{
				uint32_t vert_length = tb_vb_sub_allocs_[i].length_;
				uint32_t const ind_offset = tb_ib_sub_allocs_[i].offset_;
				uint32_t ind_length = tb_ib_sub_allocs_[i].length_;

				while ((i + 1 < tb_vb_sub_allocs_.size())
					&& (tb_vb_sub_allocs_[i].offset_ + tb_vb_sub_allocs_[i].length_ == tb_vb_sub_allocs_[i + 1].offset_)
					&& (tb_ib_sub_allocs_[i].offset_ + tb_ib_sub_allocs_[i].length_ == tb_ib_sub_allocs_[i + 1].offset_))
				{
					vert_length += tb_vb_sub_allocs_[i + 1].length_;
					ind_length += tb_ib_sub_allocs_[i + 1].length_;
					++ i;
				}

				rl_->NumVertices(vert_length / sizeof(FontVert));
				rl_->StartIndexLocation(ind_offset / sizeof(uint16_t));
				rl_->NumIndices(ind_length / sizeof(uint16_t));

				re.Render(*this->GetRenderTechnique(), *rl_);
			}

			for (size_t i = 0; i < tb_vb_sub_allocs_.size(); ++ i)
			{
				tb_vb_->Dealloc(tb_vb_sub_allocs_[i]);
				tb_ib_->Dealloc(tb_ib_sub_allocs_[i]);
			}

			this->OnRenderEnd();
		}

		Size_T<float> CalcSize(std::wstring const & text, float font_size)
		{
			this->UpdateTexture(text);

			KFont& kl = *kfont_loader_;

			float const rel_size = font_size / kl.CharSize();

			std::vector<float> lines(1, 0);

			for (auto const & ch : text)
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

			KFont const & kl = *kfont_loader_;
			auto const & cim = char_info_map_;

			std::vector<FontVert> vertices;
			std::vector<uint16_t> indices;

			float const h = font_size * yScale;
			float const rel_size = font_size / kl.CharSize();
			float const rel_size_x = rel_size * xScale;
			float const rel_size_y = rel_size * yScale;

			std::vector<std::pair<float, std::wstring>> lines(1, std::make_pair(0.0f, L""));

			for (auto const & ch : text)
			{
				if (ch != L'\n')
				{
					uint32_t advance = kl.CharAdvance(ch);
					lines.back().first += (advance & 0xFFFF) * rel_size * xScale;
					lines.back().second.push_back(ch);
				}
				else
				{
					lines.emplace_back(0.0f, L"");
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
					for (auto const & p : lines)
					{
						sx.push_back(rc.right() - p.first);
					}
				}
				else
				{
					// Font::FA_Hor_Center
					for (auto const & p : lines)
					{
						sx.push_back((rc.left() + rc.right()) / 2 - p.first / 2);
					}
				}
			}

			if (align & Font::FA_Ver_Top)
			{
				for (auto iter = lines.begin(); iter != lines.end(); ++ iter)
				{
					sy.push_back(rc.top() + (iter - lines.begin()) * h);
				}
			}
			else
			{
				if (align & Font::FA_Ver_Bottom)
				{
					for (auto iter = lines.begin(); iter != lines.end(); ++ iter)
					{
						sy.push_back(rc.bottom() - (lines.size() - (iter - lines.begin())) * h);
					}
				}
				else
				{
					// Font::FA_Ver_Middle
					for (auto iter = lines.begin(); iter != lines.end(); ++ iter)
					{
						sy.push_back((rc.top() + rc.bottom()) / 2
							- lines.size() * h / 2 + (iter - lines.begin()) * h);
					}
				}
			}

			uint32_t const index_per_char = restart_ ? 5 : 6;

			uint32_t const clr32 = clr.ABGR();
			for (size_t i = 0; i < sx.size(); ++ i)
			{
				size_t const maxSize = lines[i].second.length();
				float x = sx[i], y = sy[i];

				vertices.reserve(maxSize * 4);

				for (auto const & ch : lines[i].second)
				{
					std::pair<int32_t, uint32_t> const & offset_adv = kl.CharIndexAdvance(ch);
					if (offset_adv.first != -1)
					{
						KFont::font_info const & ci = kl.CharInfo(offset_adv.first);

						float left = ci.left * rel_size_x;
						float top = ci.top * rel_size_y;
						float width = ci.width * rel_size_x;
						float height = ci.height * rel_size_y;

						auto cmiter = cim.find(ch);
						Rect const & texRect(cmiter->second.rc);

						Rect pos_rc(x + left, y + top, x + left + width, y + top + height);
						Rect intersect_rc = pos_rc & rc;
						if ((intersect_rc.Width() > 0) && (intersect_rc.Height() > 0))
						{
							vertices.push_back(FontVert(float3(pos_rc.left(), pos_rc.top(), sz),
													clr32,
													float2(texRect.left(), texRect.top())));
							vertices.push_back(FontVert(float3(pos_rc.right(), pos_rc.top(), sz),
													clr32,
													float2(texRect.right(), texRect.top())));
							vertices.push_back(FontVert(float3(pos_rc.right(), pos_rc.bottom(), sz),
													clr32,
													float2(texRect.right(), texRect.bottom())));
							vertices.push_back(FontVert(float3(pos_rc.left(), pos_rc.bottom(), sz),
													clr32,
													float2(texRect.left(), texRect.bottom())));
						}
					}

					x += (offset_adv.second & 0xFFFF) * rel_size_x;
					y += (offset_adv.second >> 16) * rel_size_y;
				}

				tb_vb_sub_allocs_.push_back(tb_vb_->Alloc(static_cast<uint32_t>(vertices.size() * sizeof(vertices[0])), &vertices[0]));

				uint16_t last_index = static_cast<uint16_t>(tb_vb_sub_allocs_.back().offset_ / sizeof(FontVert));
				uint32_t const num_chars = static_cast<uint32_t>(vertices.size() / 4);
				indices.reserve(num_chars * index_per_char);
				for (uint32_t c = 0; c < num_chars; ++ c)
				{
					indices.push_back(last_index + 0);
					indices.push_back(last_index + 1);
					if (restart_)
					{
						indices.push_back(last_index + 3);
						indices.push_back(last_index + 2);
						indices.push_back(0xFFFF);
					}
					else
					{
						indices.push_back(last_index + 2);
						indices.push_back(last_index + 2);
						indices.push_back(last_index + 3);
						indices.push_back(last_index + 0);
					}
					last_index += 4;
				}
				BOOST_ASSERT(last_index + 3 <= 0xFFFF);
				tb_ib_sub_allocs_.push_back(tb_ib_->Alloc(static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]));

				pos_aabb_ |= AABBox(float3(sx[i], sy[i], sz), float3(sx[i] + lines[i].first, sy[i] + h, sz + 0.1f));
			}
		}

		void AddText(float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, float font_size)
		{
			this->UpdateTexture(text);

			KFont const & kl = *kfont_loader_;
			auto const & cim = char_info_map_;

			std::vector<FontVert> vertices;
			std::vector<uint16_t> indices;

			uint32_t const clr32 = clr.ABGR();
			float const h = font_size * yScale;
			float const rel_size = font_size / kl.CharSize();
			float const rel_size_x = rel_size * xScale;
			float const rel_size_y = rel_size * yScale;
			size_t const maxSize = text.length() - std::count(text.begin(), text.end(), L'\n');
			float x = sx, y = sy;
			float maxx = sx, maxy = sy;

			uint32_t const index_per_char = restart_ ? 5 : 6;

			vertices.reserve(maxSize * 4);

			for (auto const & ch : text)
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

						auto cmiter = cim.find(ch);
						if (cmiter != cim.end())
						{
							Rect const & texRect(cmiter->second.rc);
							Rect pos_rc(x + left, y + top, x + left + width, y + top + height);

							vertices.push_back(FontVert(float3(pos_rc.left(), pos_rc.top(), sz),
												clr32,
												float2(texRect.left(), texRect.top())));
							vertices.push_back(FontVert(float3(pos_rc.right(), pos_rc.top(), sz),
												clr32,
												float2(texRect.right(), texRect.top())));
							vertices.push_back(FontVert(float3(pos_rc.right(), pos_rc.bottom(), sz),
												clr32,
												float2(texRect.right(), texRect.bottom())));
							vertices.push_back(FontVert(float3(pos_rc.left(), pos_rc.bottom(), sz),
												clr32,
												float2(texRect.left(), texRect.bottom())));
						}
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

			tb_vb_sub_allocs_.push_back(tb_vb_->Alloc(static_cast<uint32_t>(vertices.size() * sizeof(vertices[0])), &vertices[0]));

			uint16_t last_index = static_cast<uint16_t>(tb_vb_sub_allocs_.back().offset_ / sizeof(FontVert));
			uint32_t const num_chars = static_cast<uint32_t>(vertices.size() / 4);
			indices.reserve(num_chars * index_per_char);
			for (uint32_t c = 0; c < num_chars; ++ c)
			{
				indices.push_back(last_index + 0);
				indices.push_back(last_index + 1);
				if (restart_)
				{
					indices.push_back(last_index + 3);
					indices.push_back(last_index + 2);
					indices.push_back(0xFFFF);
				}
				else
				{
					indices.push_back(last_index + 2);
					indices.push_back(last_index + 2);
					indices.push_back(last_index + 3);
					indices.push_back(last_index + 0);
				}
				last_index += 4;
			}
			tb_ib_sub_allocs_.push_back(tb_ib_->Alloc(static_cast<uint32_t>(indices.size() * sizeof(indices[0])), &indices[0]));

			pos_aabb_ |= AABBox(float3(sx, sy, sz), float3(maxx, maxy, sz + 0.1f));
		}

		// ��������ʹ��LRU�㷨
		/////////////////////////////////////////////////////////////////////////////////
		void UpdateTexture(std::wstring const & text)
		{
			++ tick_;

			uint32_t const tex_size = dist_texture_->Width(0);

			KFont& kl = *kfont_loader_;
			auto& cim = char_info_map_;

			uint32_t const kfont_char_size = kl.CharSize();

			uint32_t const num_chars_a_row = tex_size / kfont_char_size;
			uint32_t const num_total_chars = num_chars_a_row * num_chars_a_row;

			for (auto const & ch : text)
			{
				int32_t offset = kl.CharIndex(ch);
				if (offset != -1)
				{
					auto cmiter = cim.find(ch);
					if (cmiter != cim.end())
					{
						// �������������ҵ���

						cmiter->second.tick = tick_;
					}
					else
					{
						// �������������Ҳ��������Ե��������������������

						KFont::font_info const & ci = kl.CharInfo(offset);

						uint32_t width = ci.width;
						uint32_t height = ci.height;

						int2 char_pos;
						CharInfo charInfo;
						if (cim.size() < num_total_chars)
						{
							// �����пռ�

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
							// �ҵ�ʹ���ʱ��û��ʹ�õ���

							uint64_t min_tick = cim.begin()->second.tick;
							auto min_chiter = cim.begin();
							for (auto chiter = cim.begin(); chiter != cim.end(); ++ chiter)
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

							for (auto chiter = cim.begin(); chiter != cim.end(); ++ chiter)
							{
								if (chiter->second.tick == min_tick)
								{
									uint32_t const x = static_cast<int32_t>(chiter->second.rc.left() * tex_size);
									uint32_t const y = static_cast<int32_t>(chiter->second.rc.top() * tex_size);
									uint32_t const id = y * num_chars_a_row + x;
									auto freeiter = char_free_list_.begin();
									while ((freeiter != char_free_list_.end()) && (freeiter->second <= id))
									{
										++ freeiter;
									}
									char_free_list_.emplace(freeiter, id, id + 1);

									cim.erase(chiter);
									break;
								}
							}
							for (auto freeiter = char_free_list_.begin(); freeiter != char_free_list_.end();)
							{
								auto nextiter = freeiter;
								++ nextiter;

								if (nextiter != char_free_list_.end())
								{
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
								else
								{
									break;
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

						cim.emplace(ch, charInfo);
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

		std::unordered_map<wchar_t, CharInfo> char_info_map_;
		std::list<std::pair<uint32_t, uint32_t>> char_free_list_;

		bool three_dim_;

		TransientBufferPtr tb_vb_;
		TransientBufferPtr tb_ib_;
		std::vector<SubAlloc> tb_vb_sub_allocs_;
		std::vector<SubAlloc> tb_ib_sub_allocs_;

		TexturePtr		dist_texture_;
		TexturePtr		a_char_texture_;
		RenderEffectPtr	effect_;

		RenderEffectParameterPtr half_width_height_ep_;
		RenderEffectParameterPtr mvp_ep_;

		std::shared_ptr<KFont> kfont_loader_;

		uint64_t tick_;
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

			std::shared_ptr<KFont> kfont_loader;
			std::shared_ptr<FontPtr> kfont;
		};

	public:
		FontLoadingDesc(std::string const & res_name, uint32_t flag)
		{
			font_desc_.res_name = res_name;
			font_desc_.flag = flag;
			font_desc_.kfont_loader = MakeSharedPtr<KFont>();
			font_desc_.kfont = MakeSharedPtr<FontPtr>();
		}

		uint64_t Type() const
		{
			static uint64_t const type = CT_HASH("FontLoadingDesc");
			return type;
		}

		bool StateLess() const
		{
			return true;
		}

		void SubThreadStage()
		{
			ResIdentifierPtr kfont_input = ResLoader::Instance().Open(font_desc_.res_name);
			font_desc_.kfont_loader->Load(kfont_input);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			if (caps.multithread_res_creating_support)
			{
				this->MainThreadStage();
			}
		}

		std::shared_ptr<void> MainThreadStage()
		{
			if (!*font_desc_.kfont)
			{
				std::shared_ptr<FontRenderable> fr = MakeSharedPtr<FontRenderable>(font_desc_.kfont_loader);
				*font_desc_.kfont = MakeSharedPtr<Font>(fr, font_desc_.flag);
			}
			return std::static_pointer_cast<void>(*font_desc_.kfont);
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
			font_desc_.kfont = fld.font_desc_.kfont;
		}

		std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource)
		{
			return resource;
		}

		virtual std::shared_ptr<void> Resource() const override
		{
			return *font_desc_.kfont;
		}

	private:
		FontDesc font_desc_;
	};
}

namespace KlayGE
{
	// ���캯��
	/////////////////////////////////////////////////////////////////////////////////
	Font::Font(std::shared_ptr<FontRenderable> const & fr)
			: font_renderable_(fr)
	{
		fso_attrib_ = SceneObject::SOA_Overlay;
	}

	Font::Font(std::shared_ptr<FontRenderable> const & fr, uint32_t flags)
			: font_renderable_(fr)
	{
		fso_attrib_ = SceneObject::SOA_Overlay;
		if (flags & Font::FS_Cullable)
		{
			fso_attrib_ |= SceneObject::SOA_Cullable;
		}
	}

	// �������ִ�С
	/////////////////////////////////////////////////////////////////////////////////
	Size_T<float> Font::CalcSize(std::wstring const & text, float font_size)
	{
		if (text.empty())
		{
			return Size_T<float>(0, 0);
		}
		else
		{
			return font_renderable_->CalcSize(text, font_size);
		}
	}

	// ��ָ��λ�û�������
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float sx, float sy, Color const & clr,
		std::wstring const & text, float font_size)
	{
		this->RenderText(sx, sy, 0, 1, 1, clr, text, font_size);
	}

	// ��ָ��λ�û�������������
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float x, float y, float z,
		float xScale, float yScale, Color const & clr,
		std::wstring const & text, float font_size)
	{
		if (!text.empty())
		{
			SceneObjectHelperPtr font_obj = MakeSharedPtr<SceneObjectHelper>(font_renderable_, fso_attrib_);
			font_renderable_->AddText2D(x, y, z, xScale, yScale, clr, text, font_size);
			font_obj->AddToSceneManager();
		}
	}

	// ��ָ�����������ڻ�������������
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(Rect const & rc, float z,
		float xScale, float yScale, Color const & clr,
		std::wstring const & text, float font_size, uint32_t align)
	{
		if (!text.empty())
		{
			SceneObjectHelperPtr font_obj = MakeSharedPtr<SceneObjectHelper>(font_renderable_, fso_attrib_);
			font_renderable_->AddText2D(rc, z, xScale, yScale, clr, text, font_size, align);
			font_obj->AddToSceneManager();
		}
	}

	// ��ָ��λ�û���3D������
	/////////////////////////////////////////////////////////////////////////////////
	void Font::RenderText(float4x4 const & mvp, Color const & clr, std::wstring const & text, float font_size)
	{
		if (!text.empty())
		{
			SceneObjectHelperPtr font_obj = MakeSharedPtr<SceneObjectHelper>(font_renderable_, fso_attrib_);
			font_renderable_->AddText3D(mvp, clr, text, font_size);
			font_obj->AddToSceneManager();
		}
	}


	FontPtr SyncLoadFont(std::string const & font_name, uint32_t flags)
	{
		return ResLoader::Instance().SyncQueryT<Font>(MakeSharedPtr<FontLoadingDesc>(font_name, flags));
	}

	FontPtr ASyncLoadFont(std::string const & font_name, uint32_t flags)
	{
		// TODO: Make it really async
		return ResLoader::Instance().SyncQueryT<Font>(MakeSharedPtr<FontLoadingDesc>(font_name, flags));
	}
}
