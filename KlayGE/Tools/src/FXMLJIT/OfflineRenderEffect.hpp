/**
* @file OfflineRenderEffect.hpp
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

#ifndef _OFFLINERENDEREFFECT_HPP
#define _OFFLINERENDEREFFECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <vector>
#include <string>
#include <algorithm>

#include <boost/noncopyable.hpp>

#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KFL/Math.hpp>

#include "OfflineShaderObject.hpp"

namespace KlayGE
{
	namespace Offline
	{
		class RenderVariable;
		typedef std::shared_ptr<RenderVariable> RenderVariablePtr;
		class RenderEffectConstantBuffer;
		typedef std::shared_ptr<RenderEffectConstantBuffer> RenderEffectConstantBufferPtr;
		class RenderEffectParameter;
		typedef std::shared_ptr<RenderEffectParameter> RenderEffectParameterPtr;
		class RenderTechnique;
		typedef std::shared_ptr<RenderTechnique> RenderTechniquePtr;
		class RenderEffect;
		typedef std::shared_ptr<RenderEffect> RenderEffectPtr;
		class RenderEffectAnnotation;
		typedef std::shared_ptr<RenderEffectAnnotation> RenderEffectAnnotationPtr;
		class RenderPass;
		typedef std::shared_ptr<RenderPass> RenderPassPtr;

		inline bool operator==(SamplerStateDesc const & lhs, SamplerStateDesc const & rhs)
		{
			return 0 == memcmp(&lhs, &rhs, sizeof(lhs));
		}
		inline bool operator!=(SamplerStateDesc const & lhs, SamplerStateDesc const & rhs)
		{
			return !(lhs == rhs);
		}

		enum RenderEffectDataType
		{
			REDT_bool = 0,
			REDT_string,
			REDT_texture1D,
			REDT_texture2D,
			REDT_texture3D,
			REDT_textureCUBE,
			REDT_texture1DArray,
			REDT_texture2DArray,
			REDT_texture3DArray,
			REDT_textureCUBEArray,
			REDT_sampler,
			REDT_shader,
			REDT_uint,
			REDT_uint2,
			REDT_uint3,
			REDT_uint4,
			REDT_int,
			REDT_int2,
			REDT_int3,
			REDT_int4,
			REDT_float,
			REDT_float2,
			REDT_float2x2,
			REDT_float2x3,
			REDT_float2x4,
			REDT_float3,
			REDT_float3x2,
			REDT_float3x3,
			REDT_float3x4,
			REDT_float4,
			REDT_float4x2,
			REDT_float4x3,
			REDT_float4x4,
			REDT_buffer,
			REDT_structured_buffer,
			REDT_byte_address_buffer,
			REDT_rw_buffer,
			REDT_rw_structured_buffer,
			REDT_rw_texture1D,
			REDT_rw_texture2D,
			REDT_rw_texture3D,
			REDT_rw_texture1DArray,
			REDT_rw_texture2DArray,
			REDT_rw_byte_address_buffer,
			REDT_append_structured_buffer,
			REDT_consume_structured_buffer
		};

		struct TextureSubresource
		{
			TexturePtr tex;
			uint32_t first_array_index;
			uint32_t num_items;
			uint32_t first_level;
			uint32_t num_levels;

			TextureSubresource()
			{
			}

			TextureSubresource(TexturePtr const & t, uint32_t fai, uint32_t ni, uint32_t fl, uint32_t nl)
				: tex(t), first_array_index(fai), num_items(ni), first_level(fl), num_levels(nl)
			{
			}
		};

		class RenderVariable
		{
		public:
			RenderVariable();
			virtual ~RenderVariable() = 0;

			virtual RenderVariable& operator=(bool const & value);
			virtual RenderVariable& operator=(uint32_t const & value);
			virtual RenderVariable& operator=(int32_t const & value);
			virtual RenderVariable& operator=(float const & value);
			virtual RenderVariable& operator=(uint2 const & value);
			virtual RenderVariable& operator=(uint3 const & value);
			virtual RenderVariable& operator=(uint4 const & value);
			virtual RenderVariable& operator=(int2 const & value);
			virtual RenderVariable& operator=(int3 const & value);
			virtual RenderVariable& operator=(int4 const & value);
			virtual RenderVariable& operator=(float2 const & value);
			virtual RenderVariable& operator=(float3 const & value);
			virtual RenderVariable& operator=(float4 const & value);
			virtual RenderVariable& operator=(float4x4 const & value);
			virtual RenderVariable& operator=(TexturePtr const & value);
			virtual RenderVariable& operator=(TextureSubresource const & value);
			virtual RenderVariable& operator=(SamplerStateDesc const & value);
			virtual RenderVariable& operator=(GraphicsBufferPtr const & value);
			virtual RenderVariable& operator=(std::string const & value);
			virtual RenderVariable& operator=(ShaderDesc const & value);
			virtual RenderVariable& operator=(std::vector<bool> const & value);
			virtual RenderVariable& operator=(std::vector<uint32_t> const & value);
			virtual RenderVariable& operator=(std::vector<int32_t> const & value);
			virtual RenderVariable& operator=(std::vector<float> const & value);
			virtual RenderVariable& operator=(std::vector<uint2> const & value);
			virtual RenderVariable& operator=(std::vector<uint3> const & value);
			virtual RenderVariable& operator=(std::vector<uint4> const & value);
			virtual RenderVariable& operator=(std::vector<int2> const & value);
			virtual RenderVariable& operator=(std::vector<int3> const & value);
			virtual RenderVariable& operator=(std::vector<int4> const & value);
			virtual RenderVariable& operator=(std::vector<float2> const & value);
			virtual RenderVariable& operator=(std::vector<float3> const & value);
			virtual RenderVariable& operator=(std::vector<float4> const & value);
			virtual RenderVariable& operator=(std::vector<float4x4> const & value);

			virtual void Value(bool& val) const;
			virtual void Value(uint32_t& val) const;
			virtual void Value(int32_t& val) const;
			virtual void Value(float& val) const;
			virtual void Value(uint2& val) const;
			virtual void Value(uint3& val) const;
			virtual void Value(uint4& val) const;
			virtual void Value(int2& val) const;
			virtual void Value(int3& val) const;
			virtual void Value(int4& val) const;
			virtual void Value(float2& val) const;
			virtual void Value(float3& val) const;
			virtual void Value(float4& val) const;
			virtual void Value(float4x4& val) const;
			virtual void Value(TexturePtr& val) const;
			virtual void Value(TextureSubresource& val) const;
			virtual void Value(SamplerStateDesc& val) const;
			virtual void Value(GraphicsBufferPtr& value) const;
			virtual void Value(std::string& val) const;
			virtual void Value(ShaderDesc& val) const;
			virtual void Value(std::vector<bool>& val) const;
			virtual void Value(std::vector<uint32_t>& val) const;
			virtual void Value(std::vector<int32_t>& val) const;
			virtual void Value(std::vector<float>& val) const;
			virtual void Value(std::vector<uint2>& val) const;
			virtual void Value(std::vector<uint3>& val) const;
			virtual void Value(std::vector<uint4>& val) const;
			virtual void Value(std::vector<int2>& val) const;
			virtual void Value(std::vector<int3>& val) const;
			virtual void Value(std::vector<int4>& val) const;
			virtual void Value(std::vector<float2>& val) const;
			virtual void Value(std::vector<float3>& val) const;
			virtual void Value(std::vector<float4>& val) const;
			virtual void Value(std::vector<float4x4>& val) const;

			virtual void BindToCBuffer(RenderEffectConstantBuffer* cbuff, uint32_t offset, uint32_t stride);
			virtual void RebindToCBuffer(RenderEffectConstantBuffer* cbuff);
			virtual bool InCBuffer() const
			{
				return false;
			}
			virtual uint32_t CBufferOffset() const
			{
				return 0;
			}
			virtual uint32_t Stride() const
			{
				return 0;
			}
		};

		template <typename T>
		class RenderVariableConcrete : public RenderVariable
		{
		public:
			RenderVariableConcrete()
				: in_cbuff_(false)
			{
				new (data_.val) T;
			}
			virtual ~RenderVariableConcrete()
			{
				if (!in_cbuff_)
				{
					this->RetriveT().~T();
				}
			}

			virtual RenderVariable& operator=(T const & value) override
			{
				if (in_cbuff_)
				{
					T& val_in_cbuff = *(data_.cbuff_desc.cbuff->template VariableInBuff<T>(data_.cbuff_desc.offset));
					if (val_in_cbuff != value)
					{
						val_in_cbuff = value;
					}
				}
				else
				{
					this->RetriveT() = value;
				}
				return *this;
			}

			virtual void Value(T& val) const override
			{
				if (in_cbuff_)
				{
					val = *(data_.cbuff_desc.cbuff->template VariableInBuff<T>(data_.cbuff_desc.offset));
				}
				else
				{
					val = this->RetriveT();
				}
			}

			virtual void BindToCBuffer(RenderEffectConstantBuffer* cbuff, uint32_t offset,
				uint32_t stride) override
			{
				if (!in_cbuff_)
				{
					T val;
					this->Value(val);
					this->RetriveT().~T();
					in_cbuff_ = true;
					data_.cbuff_desc.cbuff = cbuff;
					data_.cbuff_desc.offset = offset;
					data_.cbuff_desc.stride = stride;
					this->operator=(val);
				}
			}

			virtual void RebindToCBuffer(RenderEffectConstantBuffer* cbuff) override
			{
				BOOST_ASSERT(in_cbuff_);
				data_.cbuff_desc.cbuff = cbuff;
			}

			virtual bool InCBuffer() const override
			{
				return in_cbuff_;
			}
			virtual uint32_t CBufferOffset() const override
			{
				return data_.cbuff_desc.offset;
			}
			virtual uint32_t Stride() const override
			{
				return data_.cbuff_desc.stride;
			}

		protected:
			T& RetriveT()
			{
				union Raw2T
				{
					uint8_t* raw;
					T* t;
				} r2t;
				r2t.raw = data_.val;
				return *r2t.t;
			}
			T const & RetriveT() const
			{
				union Raw2T
				{
					uint8_t const * raw;
					T const * t;
				} r2t;
				r2t.raw = data_.val;
				return *r2t.t;
			}

		protected:
			bool in_cbuff_;
			union VarData
			{
				struct CBufferDesc
				{
					RenderEffectConstantBuffer* cbuff;
					uint32_t offset;
					uint32_t stride;
				};

				CBufferDesc cbuff_desc;
				uint8_t val[sizeof(T)];
			};
			VarData data_;
		};

		class RenderVariableFloat4x4 : public RenderVariableConcrete<float4x4>
		{
		public:
			virtual RenderVariable& operator=(float4x4 const & value) override;
			virtual void Value(float4x4& val) const override;
		};

		template <typename T>
		class RenderVariableArray : public RenderVariableConcrete<std::vector<T>>
		{
		public:
			RenderVariableArray()
				: size_(0)
			{
			}

			virtual RenderVariable& operator=(std::vector<T> const & value) override
			{
				if (this->in_cbuff_)
				{
					uint8_t* target = this->data_.cbuff_desc.cbuff->template VariableInBuff<uint8_t>(this->data_.cbuff_desc.offset);

					size_ = static_cast<uint32_t>(value.size());
					for (size_t i = 0; i < value.size(); ++i)
					{
						memcpy(target + i * this->data_.cbuff_desc.stride, &value[i], sizeof(value[i]));
					}
				}
				else
				{
					this->RetriveT() = value;
				}
				return *this;
			}

			virtual void Value(std::vector<T>& val) const override
			{
				if (this->in_cbuff_)
				{
					uint8_t const * src = this->data_.cbuff_desc.cbuff->template VariableInBuff<uint8_t>(this->data_.cbuff_desc.offset);

					val.resize(size_);
					for (size_t i = 0; i < size_; ++i)
					{
						memcpy(&val[i], src + i * this->data_.cbuff_desc.stride, sizeof(val[i]));
					}
				}
				else
				{
					val = this->RetriveT();
				}
			}

		private:
			uint32_t size_;
		};

		class RenderVariableFloat4x4Array : public RenderVariableConcrete<std::vector<float4x4>>
		{
		public:
			RenderVariableFloat4x4Array()
				: size_(0)
			{
			}

			virtual RenderVariable& operator=(std::vector<float4x4> const & value) override;
			virtual void Value(std::vector<float4x4>& val) const override;

		private:
			uint32_t size_;
		};

		class RenderVariableTexture : public RenderVariable
		{
		public:
			virtual RenderVariable& operator=(TexturePtr const & value);
			virtual RenderVariable& operator=(TextureSubresource const & value);
			virtual RenderVariable& operator=(std::string const & value);

			virtual void Value(TexturePtr& val) const;
			virtual void Value(TextureSubresource& val) const;
			virtual void Value(std::string& val) const;

		protected:
			mutable TextureSubresource val_;
			std::string elem_type_;
		};

		class RenderVariableBuffer : public RenderVariable
		{
		public:
			virtual RenderVariable& operator=(GraphicsBufferPtr const & value);
			virtual RenderVariable& operator=(std::string const & value);

			virtual void Value(GraphicsBufferPtr& val) const;
			virtual void Value(std::string& val) const;

		protected:
			GraphicsBufferPtr val_;
			std::string elem_type_;
		};

		class RenderVariableByteAddressBuffer : public RenderVariable
		{
		public:
			virtual RenderVariable& operator=(GraphicsBufferPtr const & value);
			virtual RenderVariable& operator=(std::string const & value);

			virtual void Value(GraphicsBufferPtr& val) const;
			virtual void Value(std::string& val) const;

		protected:
			GraphicsBufferPtr val_;
			std::string elem_type_;
		};

		typedef RenderVariableConcrete<bool> RenderVariableBool;
		typedef RenderVariableConcrete<uint32_t> RenderVariableUInt;
		typedef RenderVariableConcrete<int32_t> RenderVariableInt;
		typedef RenderVariableConcrete<float> RenderVariableFloat;
		typedef RenderVariableConcrete<uint2> RenderVariableUInt2;
		typedef RenderVariableConcrete<uint3> RenderVariableUInt3;
		typedef RenderVariableConcrete<uint4> RenderVariableUInt4;
		typedef RenderVariableConcrete<int2> RenderVariableInt2;
		typedef RenderVariableConcrete<int3> RenderVariableInt3;
		typedef RenderVariableConcrete<int4> RenderVariableInt4;
		typedef RenderVariableConcrete<float2> RenderVariableFloat2;
		typedef RenderVariableConcrete<float3> RenderVariableFloat3;
		typedef RenderVariableConcrete<float4> RenderVariableFloat4;
		typedef RenderVariableConcrete<SamplerStateDesc> RenderVariableSampler;
		typedef RenderVariableConcrete<std::string> RenderVariableString;
		typedef RenderVariableConcrete<ShaderDesc> RenderVariableShader;
		typedef RenderVariableArray<bool> RenderVariableBoolArray;
		typedef RenderVariableArray<uint32_t> RenderVariableUIntArray;
		typedef RenderVariableArray<int32_t> RenderVariableIntArray;
		typedef RenderVariableArray<float> RenderVariableFloatArray;
		typedef RenderVariableArray<int2> RenderVariableInt2Array;
		typedef RenderVariableArray<int3> RenderVariableInt3Array;
		typedef RenderVariableArray<int4> RenderVariableInt4Array;
		typedef RenderVariableArray<float2> RenderVariableFloat2Array;
		typedef RenderVariableArray<float3> RenderVariableFloat3Array;
		typedef RenderVariableArray<float4> RenderVariableFloat4Array;


		class RenderEffectAnnotation
		{
		public:
			RenderEffectAnnotation()
				: type_()
			{
			}

			void Load(XMLNodePtr const & node);

			void StreamOut(std::ostream& os);

			uint32_t Type() const
			{
				return type_;
			}
			std::string const & Name() const
			{
				return name_;
			}

			template <typename T>
			void Value(T& val) const
			{
				var_->Value(val);
			}

		private:
			uint32_t type_;
			std::string name_;

			std::shared_ptr<RenderVariable> var_;
		};

		class RenderShaderFragment
		{
		public:
			RenderShaderFragment()
				: type_(), ver_(0, 0)
			{
			}

			void Load(XMLNodePtr const & node);

			void StreamOut(std::ostream& os);

			ShaderObject::ShaderType Type() const
			{
				return type_;
			}

			ShaderModel Version() const
			{
				return ver_;
			}

			std::string const & str() const
			{
				return str_;
			}

		private:
			ShaderObject::ShaderType type_;
			ShaderModel ver_;
			std::string str_;
		};

		// äÖÈ¾Ð§¹û
		//////////////////////////////////////////////////////////////////////////////////
		class RenderEffect
		{
		public:
			explicit RenderEffect(OfflineRenderDeviceCaps const & caps);

			void Load(std::string const & name);

			void StreamOut(std::ostream& os);

			std::string const & ResName() const
			{
				return *res_name_;
			}

			OfflineRenderDeviceCaps const & TargetDeviceCaps() const
			{
				return caps_;
			}

			uint32_t NumParameters() const
			{
				return static_cast<uint32_t>(params_.size());
			}
			RenderEffectParameterPtr const & ParameterBySemantic(std::string const & semantic) const;
			RenderEffectParameterPtr const & ParameterByName(std::string const & name) const;
			RenderEffectParameterPtr const & ParameterByIndex(uint32_t n) const
			{
				BOOST_ASSERT(n < this->NumParameters());
				return params_[n];
			}

			uint32_t NumCBuffers() const
			{
				return static_cast<uint32_t>(cbuffers_.size());
			}
			RenderEffectConstantBufferPtr const & CBufferByName(std::string const & name) const;
			RenderEffectConstantBufferPtr const & CBufferByIndex(uint32_t n) const
			{
				BOOST_ASSERT(n < this->NumCBuffers());
				return cbuffers_[n];
			}

			uint32_t NumTechniques() const
			{
				return static_cast<uint32_t>(techniques_.size());
			}
			RenderTechniquePtr const & TechniqueByName(std::string const & name) const;
			RenderTechniquePtr const & TechniqueByIndex(uint32_t n) const
			{
				BOOST_ASSERT(n < this->NumTechniques());
				return techniques_[n];
			}

			uint32_t NumShaderFragments() const
			{
				return shader_frags_ ? static_cast<uint32_t>(shader_frags_->size()) : 0;
			}
			RenderShaderFragment const & ShaderFragmentByIndex(uint32_t n) const
			{
				BOOST_ASSERT(n < this->NumShaderFragments());
				return (*shader_frags_)[n];
			}

			uint32_t AddShaderDesc(ShaderDesc const & sd);
			ShaderDesc& GetShaderDesc(uint32_t id);
			ShaderDesc const & GetShaderDesc(uint32_t id) const;

			uint32_t NumMacros() const
			{
				return macros_ ? static_cast<uint32_t>(macros_->size()) : 0;
			}
			std::pair<std::string, std::string> const & MacroByIndex(uint32_t n) const
			{
				BOOST_ASSERT(n < this->NumMacros());
				return (*macros_)[n].first;
			}

			std::string const & TypeName(uint32_t code) const;

			void GenHLSLShaderText();
			std::string const & HLSLShaderText() const;

		private:
			void RecursiveIncludeNode(XMLNodePtr const & root, std::vector<std::string>& include_names) const;
			void InsertIncludeNodes(XMLDocument& target_doc, XMLNodePtr const & target_root,
				XMLNodePtr const & target_place, XMLNodePtr const & include_root) const;

		private:
			std::shared_ptr<std::string> res_name_;
			uint64_t timestamp_;

			std::vector<RenderEffectParameterPtr> params_;
			std::vector<RenderEffectConstantBufferPtr> cbuffers_;
			std::vector<RenderTechniquePtr> techniques_;

			std::shared_ptr<std::vector<std::pair<std::pair<std::string, std::string>, bool>>> macros_;
			std::shared_ptr<std::vector<RenderShaderFragment>> shader_frags_;
			std::shared_ptr<std::string> hlsl_shader_;

			std::shared_ptr<std::vector<ShaderDesc>> shader_descs_;

			OfflineRenderDeviceCaps caps_;
		};

		class RenderTechnique : boost::noncopyable
		{
		public:
			explicit RenderTechnique(RenderEffect& effect)
				: effect_(effect),
					name_hash_(), weight_(), transparent_(), is_validate_(false)
			{
			}

			void Load(XMLNodePtr const & node, uint32_t tech_index);

			void StreamOut(std::ostream& os, uint32_t tech_index);

			std::string const & Name() const
			{
				return *name_;
			}
			size_t NameHash() const
			{
				return name_hash_;
			}

			RenderEffect& Effect() const
			{
				return effect_;
			}

			uint32_t NumAnnotations() const
			{
				return annotations_ ? static_cast<uint32_t>(annotations_->size()) : 0;
			}
			RenderEffectAnnotationPtr const & Annotation(uint32_t n) const
			{
				BOOST_ASSERT(n < this->NumAnnotations());
				return (*annotations_)[n];
			}

			uint32_t NumMacros() const
			{
				return macros_ ? static_cast<uint32_t>(macros_->size()) : 0;
			}
			std::pair<std::string, std::string> const & MacroByIndex(uint32_t n) const
			{
				BOOST_ASSERT(n < this->NumMacros());
				return (*macros_)[n];
			}

			uint32_t NumPasses() const
			{
				return static_cast<uint32_t>(passes_.size());
			}
			RenderPassPtr const & Pass(uint32_t n) const
			{
				BOOST_ASSERT(n < this->NumPasses());
				return passes_[n];
			}

			bool Validate() const
			{
				return is_validate_;
			}

			float Weight() const
			{
				return weight_;
			}

			bool Transparent() const
			{
				return transparent_;
			}

		private:
			RenderEffect& effect_;
			std::shared_ptr<std::string> name_;
			size_t name_hash_;

			std::vector<RenderPassPtr> passes_;
			std::shared_ptr<std::vector<RenderEffectAnnotationPtr>> annotations_;
			std::shared_ptr<std::vector<std::pair<std::string, std::string>>> macros_;

			float weight_;
			bool transparent_;

			bool is_validate_;
		};

		class RenderPass : boost::noncopyable
		{
		public:
			explicit RenderPass(RenderEffect& effect)
				: effect_(effect),
				front_stencil_ref_(0), back_stencil_ref_(0),
				blend_factor_(1, 1, 1, 1), sample_mask_(0xFFFFFFFF)
			{
			}

			void Load(XMLNodePtr const & node, uint32_t tech_index, uint32_t pass_index, RenderPassPtr const & inherit_pass);
			void Load(uint32_t tech_index, uint32_t pass_index, RenderPassPtr const & inherit_pass);

			void StreamOut(std::ostream& os, uint32_t tech_index, uint32_t pass_index);

			std::string const & Name() const
			{
				return *name_;
			}
			size_t NameHash() const
			{
				return name_hash_;
			}

			bool Validate() const
			{
				return is_validate_;
			}

			ShaderObjectPtr const & GetShaderObject() const
			{
				return shader_obj_;
			}

			uint32_t NumAnnotations() const
			{
				return annotations_ ? static_cast<uint32_t>(annotations_->size()) : 0;
			}
			RenderEffectAnnotationPtr const & Annotation(uint32_t n) const
			{
				BOOST_ASSERT(n < this->NumAnnotations());
				return (*annotations_)[n];
			}

			uint32_t NumMacros() const
			{
				return macros_ ? static_cast<uint32_t>(macros_->size()) : 0;
			}
			std::pair<std::string, std::string> const & MacroByIndex(uint32_t n) const
			{
				BOOST_ASSERT(n < this->NumMacros());
				return (*macros_)[n];
			}

		private:
			void CreateShaderObject();

		private:
			RenderEffect& effect_;

			std::shared_ptr<std::string> name_;
			size_t name_hash_;
			std::shared_ptr<std::vector<RenderEffectAnnotationPtr>> annotations_;
			std::shared_ptr<std::vector<std::pair<std::string, std::string>>> macros_;
			std::shared_ptr<std::vector<uint32_t>> shader_desc_ids_;

			RasterizerStateDesc rasterizer_state_desc_;
			DepthStencilStateDesc depth_stencil_state_desc_;
			uint16_t front_stencil_ref_, back_stencil_ref_;
			BlendStateDesc blend_state_desc_;
			Color blend_factor_;
			uint32_t sample_mask_;
			ShaderObjectPtr shader_obj_;

			bool is_validate_;
		};

		class RenderEffectConstantBuffer : boost::noncopyable
		{
		public:
			RenderEffectConstantBuffer();
			~RenderEffectConstantBuffer();

			void Load(std::string const & name);

			void StreamOut(std::ostream& os);

			std::shared_ptr<std::string> const & Name() const
			{
				return name_;
			}
			size_t NameHash() const
			{
				return name_hash_;
			}

			void AddParameter(uint32_t index);

			uint32_t NumParameters() const
			{
				return param_indices_ ? static_cast<uint32_t>(param_indices_->size()) : 0;
			}
			uint32_t ParameterIndex(uint32_t index) const
			{
				return (*param_indices_)[index];
			}

			void Resize(uint32_t size);

			template <typename T>
			T const * VariableInBuff(uint32_t offset) const
			{
				union Raw2T
				{
					uint8_t const * raw;
					T const * t;
				} r2t;
				r2t.raw = &buff_[offset];
				return r2t.t;
			}
			template <typename T>
			T* VariableInBuff(uint32_t offset)
			{
				union Raw2T
				{
					uint8_t* raw;
					T* t;
				} r2t;
				r2t.raw = &buff_[offset];
				return r2t.t;
			}

		private:
			std::shared_ptr<std::string> name_;
			size_t name_hash_;
			std::shared_ptr<std::vector<uint32_t>> param_indices_;

			std::vector<uint8_t> buff_;
		};

		class RenderEffectParameter : boost::noncopyable
		{
		public:
			explicit RenderEffectParameter();
			~RenderEffectParameter();

			void Load(XMLNodePtr const & node);

			void StreamOut(std::ostream& os);

			uint32_t Type() const
			{
				return type_;
			}

			RenderVariablePtr const & Var() const
			{
				return var_;
			}

			std::shared_ptr<std::string> const & ArraySize() const
			{
				return array_size_;
			}

			std::shared_ptr<std::string> const & Name() const
			{
				return name_;
			}
			size_t NameHash() const
			{
				return name_hash_;
			}
			std::shared_ptr<std::string> const & Semantic() const
			{
				return semantic_;
			}
			size_t SemanticHash() const
			{
				return semantic_hash_;
			}

			uint32_t NumAnnotations() const
			{
				return annotations_ ? static_cast<uint32_t>(annotations_->size()) : 0;
			}
			RenderEffectAnnotationPtr const & Annotation(uint32_t n)
			{
				BOOST_ASSERT(n < this->NumAnnotations());
				return (*annotations_)[n];
			}

			template <typename T>
			RenderEffectParameter& operator=(T const & value)
			{
				*var_ = value;
				return *this;
			}

			template <typename T>
			void Value(T& val) const
			{
				var_->Value(val);
			}

			void BindToCBuffer(RenderEffectConstantBufferPtr const & cbuff, uint32_t offset, uint32_t stride);
			void RebindToCBuffer(RenderEffectConstantBufferPtr const & cbuff);
			RenderEffectConstantBufferPtr const & CBuffer() const
			{
				return cbuff_;
			}
			bool InCBuffer() const
			{
				return var_->InCBuffer();
			}
			uint32_t CBufferOffset() const
			{
				return var_->CBufferOffset();
			}
			uint32_t Stride() const
			{
				return var_->Stride();
			}
			template <typename T>
			T const * MemoryInCBuff() const
			{
				return cbuff_->VariableInBuff<T>(var_->CBufferOffset());
			}
			template <typename T>
			T* MemoryInCBuff()
			{
				return cbuff_->VariableInBuff<T>(var_->CBufferOffset());
			}

		private:
			std::shared_ptr<std::string> name_;
			size_t name_hash_;
			std::shared_ptr<std::string> semantic_;
			size_t semantic_hash_;

			uint32_t type_;
			std::shared_ptr<RenderVariable> var_;
			std::shared_ptr<std::string> array_size_;

			std::shared_ptr<std::vector<RenderEffectAnnotationPtr>> annotations_;

			RenderEffectConstantBufferPtr cbuff_;
		};
	}
}

#endif		// _OFFLINERENDEREFFECT_HPP
