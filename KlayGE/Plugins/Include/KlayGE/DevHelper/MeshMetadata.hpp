/**
 * @file MeshMetadata.hpp
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

#ifndef KLAYGE_PLUGINS_MESH_METADATA_HPP
#define KLAYGE_PLUGINS_MESH_METADATA_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KFL/Math.hpp>

#include <string>
#include <vector>

#include <KlayGE/DevHelper/DevHelper.hpp>

namespace KlayGE
{
	class KLAYGE_DEV_HELPER_API MeshMetadata final
	{
	public:
		MeshMetadata();
		explicit MeshMetadata(std::string_view name);

		void Load(std::string_view name);
		void Save(std::string const & name) const;

		bool AutoCenter() const
		{
			return auto_center_;
		}
		void AutoCenter(bool auto_center)
		{
			auto_center_ = auto_center;
		}

		float3 const & Pivot() const
		{
			return pivot_;
		}
		void Pivot(float3 const & pivot)
		{
			pivot_ = pivot;
		}
		float3 const & Translation() const
		{
			return translation_;
		}
		void Translation(float3 const & trans)
		{
			translation_ = trans;
		}
		Quaternion const & Rotation() const
		{
			return rotation_;
		}
		void Rotation(Quaternion const & rot)
		{
			rotation_ = rot;
		}
		float3 const & Scale() const
		{
			return scale_;
		}
		void Scale(float3 const & scale)
		{
			scale_ = scale;
		}

		uint8_t AxisMapping(uint32_t axis) const
		{
			return axis_mapping_[axis];
		}
		void AxisMapping(uint32_t axis, uint8_t mapping)
		{
			axis_mapping_[axis] = mapping;
		}

		bool FlipWindingOrder() const
		{
			return flip_winding_order_;
		}
		void FlipWindingOrder(bool flip_winding_order)
		{
			flip_winding_order_ = flip_winding_order;
		}

		uint32_t NumLods() const;
		void NumLods(uint32_t lods);
		std::string_view LodFileName(uint32_t lod) const;
		void LodFileName(uint32_t lod, std::string_view lod_name);

		uint32_t NumMaterials() const;
		void NumMaterials(uint32_t materials);
		std::string_view MaterialFileName(uint32_t mtl_index) const;
		void MaterialFileName(uint32_t mtl_index, std::string_view mtlml_name);

		float4x4 const & Transform() const
		{
			return transform_;
		}
		float4x4 const & TransformIT() const
		{
			return transform_it_;
		}

		void UpdateTransforms();

	private:
		bool auto_center_ = false;

		float3 pivot_ = float3::Zero();
		float3 translation_ = float3::Zero();
		Quaternion rotation_ = Quaternion::Identity();
		float3 scale_ = float3(1, 1, 1);
		uint8_t axis_mapping_[3] = { 0, 1, 2 };
		bool flip_winding_order_ = false;
		std::vector<std::string> lod_file_names_;
		std::vector<std::string> material_file_names_;

		float4x4 transform_ = float4x4::Identity();
		float4x4 transform_it_ = float4x4::Identity();
	};
}

#endif		// KLAYGE_PLUGINS_MESH_METADATA_HPP
