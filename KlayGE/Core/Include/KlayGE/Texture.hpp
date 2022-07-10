// Texture.hpp
// KlayGE ������ ͷ�ļ�
// Ver 3.11.0
// ��Ȩ����(C) ������, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Remove Texture::Bpp (2010.12.17)
// Add Texture::CopyToSubTexture* (2010.12.17)
//
// 3.9.0
// ������TextureLoader (2009.4.9)
// ֧��Texture Array�Ķ�д (2009.10.15)
//
// 3.8.0
// ������access_hint (2008.9.20)
// ������ElementInitData (2008.10.1)
// ���߳��������� (2009.1.22)
//
// 3.6.0
// ����ͨ��Mapֱ�ӷ����������� (2007.7.7)
//
// 3.3.0
// ʹ��ElementFormat (2006.6.8)
//
// 3.2.0
// ֧��sRGB (2006.4.24)
//
// 3.0.0
// ȥ���˹��캯����usage (2005.10.5)
//
// 2.7.0
// ���Ի�ȡMipmap��ÿ��Ŀ���� (2005.6.8)
// ������AddressingMode, Filtering��Anisotropy (2005.6.27)
// ������MaxMipLevel��MipMapLodBias (2005.6.28)
//
// 2.4.0
// ������DXTn��֧�� (2005.3.6)
// ������1D/2D/3D/cube��֧�� (2005.3.8)
//
// 2.3.0
// �����˶Ը��������ʽ��֧�� (2005.1.25)
// ������CopyToMemory (2005.2.6)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef KLAYGE_CORE_TEXTURE_HPP
#define KLAYGE_CORE_TEXTURE_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KFL/CXX20/span.hpp>

#include <atomic>
#include <string>
#include <vector>
#include <boost/assert.hpp>

namespace KlayGE
{
#ifdef KLAYGE_SHIP
#define KLAYGE_TEXTURE_DEBUG_NAME(texture)
#else
#define KLAYGE_TEXTURE_DEBUG_NAME(texture) texture->DebugName(L ## #texture)
#endif

	enum TextureMapAccess
	{
		TMA_Read_Only,
		TMA_Write_Only,
		TMA_Read_Write
	};

	enum class TextureFilter
	{
		Point,
		Linear,
	};

	// Abstract class representing a Texture resource.
	// @remarks
	// The actual concrete subclass which will exist for a texture
	// is dependent on the rendering system in use (Direct3D, OpenGL etc).
	// This class represents the commonalities, and is the one 'used'
	// by programmers even though the real implementation could be
	// different in reality.
	class KLAYGE_CORE_API Texture : public std::enable_shared_from_this<Texture>, boost::noncopyable
	{
	public:
		// Enum identifying the texture type
		enum TextureType
		{
			// 1D texture, used in combination with 1D texture coordinates
			TT_1D,
			// 2D texture, used in combination with 2D texture coordinates
			TT_2D,
			// 3D texture, used in combination with 3D texture coordinates
			TT_3D,
			// cube map, used in combination with 3D texture coordinates
			TT_Cube
		};

		enum CubeFaces
		{
			CF_Positive_X = 0,
			CF_Negative_X = 1,
			CF_Positive_Y = 2,
			CF_Negative_Y = 3,
			CF_Positive_Z = 4,
			CF_Negative_Z = 5
		};

	public:
		class KLAYGE_CORE_API Mapper final : boost::noncopyable
		{
			friend class Texture;

		public:
			Mapper(Texture& tex, uint32_t array_index, uint32_t level, TextureMapAccess tma,
				uint32_t x_offset, uint32_t width);
			Mapper(Texture& tex, uint32_t array_index, uint32_t level, TextureMapAccess tma,
				uint32_t x_offset, uint32_t y_offset,
				uint32_t width, uint32_t height);
			Mapper(Texture& tex, uint32_t array_index, uint32_t level, TextureMapAccess tma,
				uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
				uint32_t width, uint32_t height, uint32_t depth);
			Mapper(Texture& tex, uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
				uint32_t x_offset, uint32_t y_offset,
				uint32_t width, uint32_t height);

			~Mapper();

			template <typename T>
			T const * Pointer() const
			{
				return static_cast<T*>(data_);
			}
			template <typename T>
			T* Pointer()
			{
				return static_cast<T*>(data_);
			}

			uint32_t RowPitch() const
			{
				return row_pitch_;
			}

			uint32_t SlicePitch() const
			{
				return slice_pitch_;
			}

		private:
			Texture& tex_;

			void* data_;
			uint32_t row_pitch_, slice_pitch_;

			uint32_t mapped_array_index_;
			CubeFaces mapped_face_;
			uint32_t mapped_level_;
		};

	public:
		Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
		virtual ~Texture() noexcept;

		virtual std::wstring const& Name() const = 0;
#ifndef KLAYGE_SHIP
		virtual void DebugName(std::wstring_view name)
		{
			KFL_UNUSED(name);
		}
#endif

		// Gets the number of mipmaps to be used for this texture.
		uint32_t NumMipMaps() const;
		// Gets the size of texture array
		uint32_t ArraySize() const;

		// Returns the width of the texture.
		virtual uint32_t Width(uint32_t level) const = 0;
		// Returns the height of the texture.
		virtual uint32_t Height(uint32_t level) const = 0;
		// Returns the depth of the texture (only for 3D texture).
		virtual uint32_t Depth(uint32_t level) const = 0;

		// Returns the pixel format for the texture surface.
		ElementFormat Format() const;

		// Returns the texture type of the texture.
		TextureType Type() const;

		uint32_t SampleCount() const;
		uint32_t SampleQuality() const;

		uint32_t AccessHint() const;

		// Copies (and maybe scales to fit) the contents of this texture to another texture.
		virtual void CopyToTexture(Texture& target, TextureFilter filter) = 0;
		virtual void CopyToSubTexture1D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
			uint32_t dst_width, uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width,
			TextureFilter filter) = 0;
		virtual void CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
			uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level,
			uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter) = 0;
		virtual void CopyToSubTexture3D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
			uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset,
			uint32_t src_width, uint32_t src_height, uint32_t src_depth, TextureFilter filter) = 0;
		virtual void CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level,
			uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index,
			CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
			TextureFilter filter) = 0;

		void BuildMipSubLevels(TextureFilter filter);

		virtual void Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width,
			void*& data) = 0;
		virtual void Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch) = 0;
		virtual void Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch) = 0;
		virtual void MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch) = 0;

		virtual void Unmap1D(uint32_t array_index, uint32_t level) = 0;
		virtual void Unmap2D(uint32_t array_index, uint32_t level) = 0;
		virtual void Unmap3D(uint32_t array_index, uint32_t level) = 0;
		virtual void UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level) = 0;

		virtual void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) = 0;
		virtual void DeleteHWResource() = 0;
		virtual bool HWResourceReady() const = 0;

		virtual void UpdateSubresource1D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t width,
			void const * data) = 0;
		virtual void UpdateSubresource2D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void const * data, uint32_t row_pitch) = 0;
		virtual void UpdateSubresource3D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void const * data, uint32_t row_pitch, uint32_t slice_pitch) = 0;
		virtual void UpdateSubresourceCube(uint32_t array_index, CubeFaces face, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void const * data, uint32_t row_pitch) = 0;

	protected:
		void ResizeTexture1D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width,
			TextureFilter filter);
		void ResizeTexture2D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
			TextureFilter filter);
		void ResizeTexture3D(Texture& target,
			uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth,
			TextureFilter filter);
		void ResizeTextureCube(Texture& target,
			uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
			uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
			TextureFilter filter);

		virtual bool HwBuildMipSubLevels(TextureFilter filter)
		{
			KFL_UNUSED(filter);
			return false;
		}

	protected:
		uint32_t		num_mip_maps_;
		uint32_t		array_size_;

		ElementFormat	format_;
		TextureType		type_;
		uint32_t		sample_count_, sample_quality_;
		uint32_t		access_hint_;
	};

	class KLAYGE_CORE_API SoftwareTexture : public Texture
	{
	public:
		SoftwareTexture(TextureType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mipmaps, uint32_t array_size,
			ElementFormat format, bool ref_only);

		std::wstring const & Name() const override;

		uint32_t Width(uint32_t level) const override;
		uint32_t Height(uint32_t level) const override;
		uint32_t Depth(uint32_t level) const override;

		void CopyToTexture(Texture& target, TextureFilter filter) override;
		void CopyToSubTexture1D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_width,
			uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width, TextureFilter filter) override;
		void CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset,
			uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter) override;
		void CopyToSubTexture3D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
			uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth, uint32_t src_array_index,
			uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_z_offset, uint32_t src_width,
			uint32_t src_height, uint32_t src_depth, TextureFilter filter) override;
		void CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level, uint32_t dst_x_offset,
			uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, CubeFaces src_face,
			uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
			TextureFilter filter) override;

		void Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t width,
			void*& data) override;
		void Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch) override;
		void Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void*& data, uint32_t& row_pitch, uint32_t& slice_pitch) override;
		void MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void*& data, uint32_t& row_pitch) override;

		void Unmap1D(uint32_t array_index, uint32_t level) override;
		void Unmap2D(uint32_t array_index, uint32_t level) override;
		void Unmap3D(uint32_t array_index, uint32_t level) override;
		void UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level) override;

		void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;
		void DeleteHWResource() override;
		bool HWResourceReady() const override;

		void UpdateSubresource1D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t width,
			void const * data) override;
		void UpdateSubresource2D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void const * data, uint32_t row_pitch) override;
		void UpdateSubresource3D(uint32_t array_index, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
			uint32_t width, uint32_t height, uint32_t depth,
			void const * data, uint32_t row_pitch, uint32_t slice_pitch) override;
		void UpdateSubresourceCube(uint32_t array_index, CubeFaces face, uint32_t level,
			uint32_t x_offset, uint32_t y_offset, uint32_t width, uint32_t height,
			void const * data, uint32_t row_pitch) override;

		std::vector<ElementInitData> const & SubresourceData() const
		{
			return subres_data_;
		}
		std::vector<uint8_t> const & DataBlock() const
		{
			return data_block_;
		}

	private:
		bool ref_only_;

		uint32_t width_;
		uint32_t height_;
		uint32_t depth_;

		std::vector<ElementInitData> subres_data_;
		std::vector<uint8_t> data_block_;
		std::vector<std::unique_ptr<std::atomic<bool>>> mapped_;
	};

	KLAYGE_CORE_API void GetImageInfo(std::string_view tex_name, Texture::TextureType& type,
		uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
		ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch);

	KLAYGE_CORE_API TexturePtr LoadSoftwareTexture(std::string_view tex_name);
	KLAYGE_CORE_API TexturePtr SyncLoadTexture(std::string_view tex_name, uint32_t access_hint);
	KLAYGE_CORE_API TexturePtr ASyncLoadTexture(std::string_view tex_name, uint32_t access_hint);

	KLAYGE_CORE_API void SaveTexture(TexturePtr const & texture, std::string const & tex_name);

	KLAYGE_CORE_API void ResizeTexture(void* dst_data, uint32_t dst_row_pitch, uint32_t dst_slice_pitch, ElementFormat dst_format,
		uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
		void const * src_data, uint32_t src_row_pitch, uint32_t src_slice_pitch, ElementFormat src_format,
		uint32_t src_width, uint32_t src_height, uint32_t src_depth,
		TextureFilter filter);

	// return the lookat and up vector in cubemap view
	//////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	std::pair<Vector_T<T, 3>, Vector_T<T, 3>> CubeMapViewVector(Texture::CubeFaces face);
}

#endif			// KLAYGE_CORE_TEXTURE_HPP
