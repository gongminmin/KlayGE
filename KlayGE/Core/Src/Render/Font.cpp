// Font.cpp
// KlayGE Font类 实现文件
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
#include <KlayGE/Sampler.hpp>
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
#include <KlayGE/ClosedHashMap.hpp>
#include <KlayGE/MapVector.hpp>

#include <algorithm>
#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <boost/functional/hash.hpp>

#include <KlayGE/Font.hpp>

using namespace std;

namespace
{
	using namespace KlayGE;

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

	struct font_info
	{
		int16_t top;
		int16_t left;
		int16_t width;
		int16_t height;
	};
#ifdef KLAYGE_PLATFORM_WINDOWS
	#pragma pack(pop)
#endif

	class FontRenderable : public RenderableHelper
	{
	public:
		explicit FontRenderable(std::string const & fontName)
			: RenderableHelper(L"Font"),
				curX_(0), curY_(0),
				three_dim_(false)
		{
			std::ifstream kfont_input(ResLoader::Instance().Locate(fontName).c_str(), std::ios_base::binary);
			BOOST_ASSERT(kfont_input);

			kfont_header header;
			kfont_input.read(reinterpret_cast<char*>(&header), sizeof(header));
			BOOST_ASSERT((MakeFourCC<'K', 'F', 'N', 'T'>::value == header.fourcc));
			BOOST_ASSERT((1 == header.version));

			kfont_char_size_ = header.char_size;
			dist_base_ = header.base;
			dist_scale_ = header.scale;

			kfont_input.seekg(header.start_ptr, std::ios_base::beg);

			std::vector<std::pair<int32_t, int32_t> > temp_char_index(header.non_empty_chars);
			kfont_input.read(reinterpret_cast<char*>(&temp_char_index[0]),
				static_cast<std::streamsize>(temp_char_index.size() * sizeof(temp_char_index[0])));
			char_index_ = MapVector<int32_t, int32_t>(temp_char_index.begin(), temp_char_index.end());

			std::vector<std::pair<int32_t, Vector_T<uint16_t, 2> > > temp_char_advance(header.validate_chars);
			kfont_input.read(reinterpret_cast<char*>(&temp_char_advance[0]),
				static_cast<std::streamsize>(temp_char_advance.size() * sizeof(temp_char_advance[0])));
			char_advance_ = MapVector<int32_t, Vector_T<uint16_t, 2> >(temp_char_advance.begin(), temp_char_advance.end());

			char_info_.resize(header.non_empty_chars);
			kfont_input.read(reinterpret_cast<char*>(&char_info_[0]),
				static_cast<std::streamsize>(char_info_.size() * sizeof(char_info_[0])));

			distances_.resize(header.non_empty_chars * header.char_size * header.char_size);
			kfont_input.read(reinterpret_cast<char*>(&distances_[0]),
				static_cast<std::streamsize>(distances_.size() * sizeof(distances_[0])));

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			RenderEngine const & renderEngine = rf.RenderEngineInstance();
			RenderDeviceCaps const & caps = renderEngine.DeviceCaps();
			dist_texture_ = rf.MakeTexture2D(std::min<uint32_t>(2048, caps.max_texture_width) / kfont_char_size_ * kfont_char_size_,
				std::min<uint32_t>(2048, caps.max_texture_height) / kfont_char_size_ * kfont_char_size_, 1, EF_L8);

			effect_ = rf.LoadEffect("Font.kfx");
			*(effect_->ParameterByName("distance_sampler")) = dist_texture_;
			*(effect_->ParameterByName("distance_base_scale")) = float2(dist_base_ / 32768.0f, dist_scale_ / 32768.0f + 1.0f);

			vb_ = rf.MakeVertexBuffer(BU_Dynamic);
			rl_->BindVertexStream(vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F),
											vertex_element(VEU_Diffuse, 0, EF_ARGB8),
											vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			ib_ = rf.MakeIndexBuffer(BU_Dynamic);
			rl_->BindIndexStream(ib_, EF_R16);

			box_ = Box(float3(0, 0, 0), float3(0, 0, 0));
		}

		RenderTechniquePtr GetRenderTechnique() const
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
				RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
				float const half_width = re.CurFrameBuffer()->Width() / 2.0f;
				float const half_height = re.CurFrameBuffer()->Height() / 2.0f;

				*(effect_->ParameterByName("half_width_height")) = float2(half_width, half_height);

				float4 texel_to_pixel = re.TexelToPixelOffset();
				texel_to_pixel.x() /= half_width;
				texel_to_pixel.y() /= half_height;
				*(effect_->ParameterByName("texel_to_pixel_offset")) = texel_to_pixel;
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
			renderEngine.Render(*this->GetRenderTechnique(), *rl_);
			this->OnRenderEnd();
		}

		Size_T<uint32_t> CalcSize(std::wstring const & text, uint32_t font_height)
		{
			this->UpdateTexture(text);

			float const rel_size = static_cast<float>(font_height) / kfont_char_size_;

			std::vector<uint32_t> lines(1, 0);

			BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
			{
				if (ch != L'\n')
				{
					BOOST_AUTO(iter, char_advance_.find(ch));
					if (iter != char_advance_.end())
					{
						lines.back() += static_cast<uint32_t>(iter->second.x() * rel_size);
					}
				}
				else
				{
					lines.push_back(0);
				}
			}

			return Size_T<uint32_t>(*std::max_element(lines.begin(), lines.end()),
				static_cast<uint32_t>(font_height * lines.size()));
		}

		void AddText2D(float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height)
		{
			three_dim_ = false;

			this->AddText(sx, sy, sz, xScale, yScale, clr, text, font_height);
		}

		void AddText2D(Rect const & rc, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height, uint32_t align)
		{
			three_dim_ = false;

			this->AddText(rc, sz, xScale, yScale, clr, text, font_height, align);
		}

		void AddText3D(float4x4 const & mvp, Color const & clr, std::wstring const & text, uint32_t font_height)
		{
			three_dim_ = true;
			*(effect_->ParameterByName("mvp")) = mvp;

			this->AddText(0, 0, 0, 1, 1, clr, text, font_height);
		}

	private:
		void AddText(Rect const & rc, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height, uint32_t align)
		{
			this->UpdateTexture(text);

			float const h = font_height * yScale;
			float const rel_size = static_cast<float>(font_height) / kfont_char_size_;

			std::vector<std::pair<float, std::wstring> > lines(1, std::make_pair(0.0f, L""));

			BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
			{
				if (ch != L'\n')
				{
					BOOST_AUTO(iter, char_advance_.find(ch));
					if (iter != char_advance_.end())
					{
						lines.back().first += iter->second.x() * rel_size * xScale;
					}
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

			uint32_t const clr32 = clr.ABGR();
			for (size_t i = 0; i < sx.size(); ++ i)
			{
				size_t const maxSize = lines[i].second.length();
				float x = sx[i], y = sy[i];

				vertices_.reserve(vertices_.size() + maxSize * 4);
				indices_.reserve(indices_.size() + maxSize * 6);

				uint16_t lastIndex(static_cast<uint16_t>(vertices_.size()));

				BOOST_FOREACH(BOOST_TYPEOF(lines[i].second)::const_reference ch, lines[i].second)
				{
					BOOST_AUTO(iter, char_index_.find(ch));
					if (iter != char_index_.end())
					{
						font_info const & ci = char_info_[iter->second];

						float left = ci.left * rel_size * xScale;
						float top = ci.top * rel_size * yScale;
						float width = ci.width * rel_size * xScale;
						float height = ci.height * rel_size * yScale;
					
						BOOST_AUTO(cmiter, charInfoMap_.find(ch));
						Rect_T<float> const & texRect(cmiter->second);

						Rect_T<float> pos_rc(x + left, y + top, x + left + width, y + top + height);
						Rect_T<float> intersect_rc = pos_rc & rc;
						if ((intersect_rc.Width() > 0) && (intersect_rc.Height() > 0))
						{
							vertices_.push_back(FontVert(float3(pos_rc.left(), pos_rc.top(), sz),
													clr32,
													float2(texRect.left(), texRect.top())));
							vertices_.push_back(FontVert(float3(pos_rc.right(), pos_rc.top(), sz),
													clr32,
													float2(texRect.right(), texRect.top())));
							vertices_.push_back(FontVert(float3(pos_rc.right(), pos_rc.bottom(), sz),
													clr32,
													float2(texRect.right(), texRect.bottom())));
							vertices_.push_back(FontVert(float3(pos_rc.left(), pos_rc.bottom(), sz),
													clr32,
													float2(texRect.left(), texRect.bottom())));

							indices_.push_back(lastIndex + 0);
							indices_.push_back(lastIndex + 1);
							indices_.push_back(lastIndex + 2);
							indices_.push_back(lastIndex + 2);
							indices_.push_back(lastIndex + 3);
							indices_.push_back(lastIndex + 0);
							lastIndex += 4;
						}
					}

					BOOST_AUTO(aiter, char_advance_.find(ch));
					if (aiter != char_advance_.end())
					{
						x += aiter->second.x() * rel_size * xScale;
						y += aiter->second.y() * rel_size * yScale;
					}
				}

				box_ |= Box(float3(sx[i], sy[i], sz), float3(sx[i] + lines[i].first, sy[i] + h, sz + 0.1f));
			}
		}

		void AddText(float sx, float sy, float sz,
			float xScale, float yScale, Color const & clr, std::wstring const & text, uint32_t font_height)
		{
			this->UpdateTexture(text);

			uint32_t const clr32 = clr.ABGR();
			float const h = font_height * yScale;
			float const rel_size = static_cast<float>(font_height) / kfont_char_size_;
			size_t const maxSize = text.length() - std::count(text.begin(), text.end(), L'\n');
			float x = sx, y = sy;
			float maxx = sx, maxy = sy;

			vertices_.reserve(vertices_.size() + maxSize * 4);
			indices_.reserve(indices_.size() + maxSize * 6);

			uint16_t lastIndex(static_cast<uint16_t>(vertices_.size()));

			BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
			{
				if (ch != L'\n')
				{
					BOOST_AUTO(iter, char_index_.find(ch));
					if (iter != char_index_.end())
					{
						font_info const & ci = char_info_[iter->second];

						float left = ci.left * rel_size * xScale;
						float top = ci.top * rel_size * yScale;
						float width = ci.width * rel_size * xScale;
						float height = ci.height * rel_size * yScale;

						BOOST_AUTO(cmiter, charInfoMap_.find(ch));
						Rect_T<float> const & texRect(cmiter->second);
						Rect_T<float> pos_rc(x + left, y + top, x + left + width, y + top + height);

						vertices_.push_back(FontVert(float3(pos_rc.left(), pos_rc.top(), sz),
												clr32,
												float2(texRect.left(), texRect.top())));
						vertices_.push_back(FontVert(float3(pos_rc.right(), pos_rc.top(), sz),
												clr32,
												float2(texRect.right(), texRect.top())));
						vertices_.push_back(FontVert(float3(pos_rc.right(), pos_rc.bottom(), sz),
												clr32,
												float2(texRect.right(), texRect.bottom())));
						vertices_.push_back(FontVert(float3(pos_rc.left(), pos_rc.bottom(), sz),
												clr32,
												float2(texRect.left(), texRect.bottom())));

						indices_.push_back(lastIndex + 0);
						indices_.push_back(lastIndex + 1);
						indices_.push_back(lastIndex + 2);
						indices_.push_back(lastIndex + 2);
						indices_.push_back(lastIndex + 3);
						indices_.push_back(lastIndex + 0);
						lastIndex += 4;
					}

					BOOST_AUTO(aiter, char_advance_.find(ch));
					if (aiter != char_advance_.end())
					{
						x += aiter->second.x() * rel_size * xScale;
						y += aiter->second.y() * rel_size * yScale;
					}

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
			uint32_t const tex_width = dist_texture_->Width(0);
			uint32_t const tex_height = dist_texture_->Height(0);

			BOOST_FOREACH(BOOST_TYPEOF(text)::const_reference ch, text)
			{
				BOOST_AUTO(iter, char_index_.find(ch));
				if (iter != char_index_.end())
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

						font_info const & ci = char_info_[iter->second];

						uint32_t width = ci.width;
						uint32_t height = ci.height;

						Vector_T<int32_t, 2> char_pos;
						CharInfo charInfo;
						if ((curX_ < tex_width) && (curY_ < tex_height) && (curY_ + kfont_char_size_ < tex_height))
						{
							if (curX_ + width > tex_width)
							{
								curX_ = 0;
								curY_ += kfont_char_size_;
							}

							// 纹理还有空间
							char_pos = Vector_T<int32_t, 2>(curX_, curY_);

							charInfo.left()		= static_cast<float>(curX_) / tex_width;
							charInfo.top()		= static_cast<float>(curY_) / tex_height;
							charInfo.right()	= static_cast<float>(curX_ + width) / tex_width;
							charInfo.bottom()	= static_cast<float>(curY_ + height) / tex_height;

							curX_ += kfont_char_size_;
						}
						else
						{
							// 找到使用最长时间没有使用的字
							BOOST_AUTO(iter, charInfoMap_.find(charLRU_.back()));
							BOOST_ASSERT(iter != charInfoMap_.end());

							char_pos.x() = static_cast<int32_t>(iter->second.left() * tex_width);
							char_pos.y() = static_cast<int32_t>(iter->second.top() * tex_height);

							charInfo.left()		= iter->second.left();
							charInfo.top()		= iter->second.top();
							charInfo.right()	= charInfo.left() + static_cast<float>(width) / tex_width;
							charInfo.bottom()	= charInfo.top() + static_cast<float>(height) / tex_height;

							charLRU_.pop_back();
							charInfoMap_.erase(iter);
						}

						{
							Texture::Mapper mapper(*dist_texture_, 0, TMA_Write_Only,
								char_pos.x(), char_pos.y(), kfont_char_size_, kfont_char_size_);
							uint8_t* tex_data = mapper.Pointer<uint8_t>();
							uint8_t const * char_data = &distances_[iter->second * kfont_char_size_ * kfont_char_size_];
							for (uint32_t y = 0; y < kfont_char_size_; ++ y)
							{
								std::memcpy(tex_data, char_data, kfont_char_size_);
								tex_data += mapper.RowPitch();
								char_data += kfont_char_size_;
							}
						}

						charInfoMap_.insert(std::make_pair(ch, charInfo));
						charLRU_.push_front(ch);
					}
				}
			}
		}

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
		RenderEffectPtr	effect_;

		uint32_t kfont_char_size_;
		int16_t dist_base_;
		int16_t dist_scale_;
		MapVector<int32_t, int32_t> char_index_;
		MapVector<int32_t, Vector_T<uint16_t, 2> > char_advance_;
		std::vector<font_info> char_info_;
		std::vector<uint8_t> distances_;
	};

	class FontObject : public SceneObjectHelper
	{
	public:
		FontObject(RenderablePtr renderable, uint32_t attrib)
			: SceneObjectHelper(renderable, attrib)
		{
		}
	};


	class FontRenderableSet
	{
	public:
		static FontRenderableSet& Instance()
		{
			static FontRenderableSet ret;
			return ret;
		}

		RenderablePtr LookupFontRenderable(std::string const & font_name)
		{
			BOOST_AUTO(iter, font_renderables_.find(font_name));
			if (iter != font_renderables_.end())
			{
				return iter->second;
			}
			else
			{
				RenderablePtr ret(new FontRenderable(font_name));
				font_renderables_.insert(std::make_pair(font_name, ret));
				return ret;
			}
		}

	private:
		FontRenderableSet()
		{
		}

		FontRenderableSet(FontRenderableSet const &);
		FontRenderableSet& operator=(FontRenderableSet const &);

	private:
		std::map<std::string, RenderablePtr> font_renderables_;
	};
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	Font::Font(std::string const & fontName, uint32_t height, uint32_t flags)
			: font_renderable_(FontRenderableSet::Instance().LookupFontRenderable(fontName)),
					font_height_(height)
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
		return font_height_;
	}

	// 计算文字大小
	/////////////////////////////////////////////////////////////////////////////////
	Size_T<uint32_t> Font::CalcSize(std::wstring const & text)
	{
		if (!text.empty())
		{
			return checked_pointer_cast<FontRenderable>(font_renderable_)->CalcSize(text, font_height_);
		}
		else
		{
			return Size_T<int32_t>(0, 0);
		}
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
			checked_pointer_cast<FontRenderable>(font_renderable_)->AddText2D(
				x, y, z, xScale, yScale, clr, text, font_height_);
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
			checked_pointer_cast<FontRenderable>(font_renderable_)->AddText2D(
				rc, z, xScale, yScale, clr, text, font_height_, align);
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
			checked_pointer_cast<FontRenderable>(font_renderable_)->AddText3D(mvp, clr, text, font_height_);
			font_obj->AddToSceneManager();
		}
	}
}
