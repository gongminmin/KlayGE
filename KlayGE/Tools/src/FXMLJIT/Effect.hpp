/**
 * @file Effect.hpp
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

#ifndef _KLAYGE_FXMLJIT_EFFECT_HPP
#define _KLAYGE_FXMLJIT_EFFECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <vector>
#include <string>
#include <algorithm>

#include <boost/noncopyable.hpp>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KFL/Math.hpp>

namespace KlayGE
{
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

		virtual RenderVariablePtr Clone() = 0;

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
		virtual RenderVariable& operator=(function<TexturePtr()> const & value);
		virtual RenderVariable& operator=(SamplerStateObjectPtr const & value);
		virtual RenderVariable& operator=(GraphicsBufferPtr const & value);
		virtual RenderVariable& operator=(std::string const & value);
		virtual RenderVariable& operator=(shader_desc const & value);
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
		virtual void Value(SamplerStateObjectPtr& val) const;
		virtual void Value(GraphicsBufferPtr& value) const;
		virtual void Value(std::string& val) const;
		virtual void Value(shader_desc& val) const;
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
	};

	template <typename T>
	class RenderVariableConcrete : public RenderVariable
	{
	public:
		virtual RenderVariablePtr Clone()
		{
			RenderVariablePtr ret = MakeSharedPtr<RenderVariableConcrete<T> >();
			T val;
			this->Value(val);
			*ret = val;
			return ret;
		}

		virtual RenderVariable& operator=(T const & value)
		{
			val_ = value;
			return *this;
		}

		virtual void Value(T& val) const
		{
			val = val_;
		}

	protected:
		T val_;
	};

	class RenderVariableTexture : public RenderVariable
	{
	public:
		virtual RenderVariablePtr Clone();

		virtual RenderVariable& operator=(TexturePtr const & value);
		virtual RenderVariable& operator=(TextureSubresource const & value);
		virtual RenderVariable& operator=(function<TexturePtr()> const & value);
		virtual RenderVariable& operator=(std::string const & value);

		virtual void Value(TexturePtr& val) const;
		virtual void Value(TextureSubresource& val) const;
		virtual void Value(std::string& val) const;

	protected:
		function<TexturePtr()> tl_;
		mutable TextureSubresource val_;
		std::string elem_type_;
	};

	class RenderVariableBuffer : public RenderVariable
	{
	public:
		RenderVariablePtr Clone();

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
		virtual RenderVariablePtr Clone();

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
	typedef RenderVariableConcrete<float4x4> RenderVariableFloat4x4;
	typedef RenderVariableConcrete<SamplerStateObjectPtr> RenderVariableSampler;
	typedef RenderVariableConcrete<std::string> RenderVariableString;
	typedef RenderVariableConcrete<shader_desc> RenderVariableShader;
	typedef RenderVariableConcrete<std::vector<bool> > RenderVariableBoolArray;
	typedef RenderVariableConcrete<std::vector<uint32_t> > RenderVariableUIntArray;
	typedef RenderVariableConcrete<std::vector<int32_t> > RenderVariableIntArray;
	typedef RenderVariableConcrete<std::vector<float> >  RenderVariableFloatArray;
	typedef RenderVariableConcrete<std::vector<int2> >  RenderVariableInt2Array;
	typedef RenderVariableConcrete<std::vector<int3> >  RenderVariableInt3Array;
	typedef RenderVariableConcrete<std::vector<int4> >  RenderVariableInt4Array;
	typedef RenderVariableConcrete<std::vector<float2> >  RenderVariableFloat2Array;
	typedef RenderVariableConcrete<std::vector<float3> >  RenderVariableFloat3Array;
	typedef RenderVariableConcrete<std::vector<float4> >  RenderVariableFloat4Array;
	typedef RenderVariableConcrete<std::vector<float4x4> >  RenderVariableFloat4x4Array;


	class RenderEffectAnnotation
	{
	public:
		void Load(XMLNodePtr const & node);

		void StreamIn(ResIdentifierPtr const & res);
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

		shared_ptr<RenderVariable> var_;
	};

	class RenderShaderFunc
	{
	public:
		void Load(XMLNodePtr const & node);

		void StreamIn(ResIdentifierPtr const & res);
		void StreamOut(std::ostream& os);

		ShaderObject::ShaderType Type() const
		{
			return type_;
		}

		uint32_t Version() const
		{
			return version_;
		}

		std::string const & str() const
		{
			return str_;
		}

	private:
		ShaderObject::ShaderType type_;
		uint32_t version_;
		std::string str_;
	};

	// äÖÈ¾Ð§¹û
	//////////////////////////////////////////////////////////////////////////////////
	class RenderEffect
	{
	public:
		RenderEffect();

		void Load(std::string const & name, std::pair<std::string, std::string>* macros);

		bool StreamIn(ResIdentifierPtr const & source, std::pair<std::string, std::string>* predefined_macros,
			std::vector<std::vector<std::vector<uint8_t> > >& native_shader_blocks);
		void StreamOut(std::ostream& os, std::vector<std::vector<std::vector<uint8_t> > > const & native_shader_blocks);

		RenderEffectPtr Clone();

		std::string const & ResName() const
		{
			return *res_name_;
		}
		uint64_t Timestamp() const
		{
			return timestamp_;
		}
		uint64_t PredefinedMacrosHash() const
		{
			return predefined_macros_hash_;
		}

		void PrototypeEffect(RenderEffectPtr const & prototype_effect)
		{
			prototype_effect_ = prototype_effect;
		}
		RenderEffectPtr const & PrototypeEffect() const
		{
			return prototype_effect_;
		}

		uint32_t NumParameters() const
		{
			return static_cast<uint32_t>(params_.size());
		}
		RenderEffectParameterPtr ParameterBySemantic(std::string const & semantic) const;
		RenderEffectParameterPtr ParameterByName(std::string const & name) const;
		RenderEffectParameterPtr ParameterByIndex(uint32_t n) const
		{
			BOOST_ASSERT(n < this->NumParameters());
			return params_[n];
		}

		std::vector<std::pair<std::string, std::vector<uint32_t> > > const & CBuffers() const
		{
			return *cbuffers_;
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

		uint32_t NumShaders() const
		{
			return shaders_ ? static_cast<uint32_t>(shaders_->size()) : 0;
		}
		RenderShaderFunc const & ShaderByIndex(uint32_t n) const
		{
			BOOST_ASSERT(n < this->NumShaders());
			return (*shaders_)[n];
		}

		uint32_t AddShaderDesc(shader_desc const & sd);
		shader_desc& GetShaderDesc(uint32_t id);
		shader_desc const & GetShaderDesc(uint32_t id) const;

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

	private:
		shared_ptr<std::string> res_name_;
		uint64_t timestamp_;
		uint64_t predefined_macros_hash_;

		std::vector<RenderEffectParameterPtr> params_;
		shared_ptr<std::vector<std::pair<std::string, std::vector<uint32_t> > > > cbuffers_;
		std::vector<RenderTechniquePtr> techniques_;

		shared_ptr<std::vector<std::pair<std::pair<std::string, std::string>, bool> > > macros_;
		shared_ptr<std::vector<RenderShaderFunc> > shaders_;

		RenderEffectPtr prototype_effect_;

		shared_ptr<std::vector<shader_desc> > shader_descs_;
	};

	class RenderTechnique : boost::noncopyable
	{
	public:
		explicit RenderTechnique(RenderEffect& effect)
			: effect_(effect)
		{
		}

		void Load(XMLNodePtr const & node, uint32_t tech_index);

		bool StreamIn(ResIdentifierPtr const & res, uint32_t tech_index,
			std::vector<std::vector<std::vector<uint8_t> > >& native_shader_blocks);
		void StreamOut(std::ostream& os, uint32_t tech_index,
			std::vector<std::vector<std::vector<uint8_t> > > const & native_shader_blocks);

		RenderTechniquePtr Clone(RenderEffect& effect);

		std::string const & Name() const
		{
			return *name_;
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

		bool HasDiscard() const
		{
			return has_discard_;
		}
		bool HasTessellation() const
		{
			return has_tessellation_;
		}

	private:
		RenderEffect& effect_;
		shared_ptr<std::string> name_;

		std::vector<RenderPassPtr> passes_;
		shared_ptr<std::vector<RenderEffectAnnotationPtr> > annotations_;

		float weight_;
		bool transparent_;

		bool is_validate_;
		bool has_discard_;
		bool has_tessellation_;
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

		bool StreamIn(ResIdentifierPtr const & res, uint32_t tech_index, uint32_t pass_index,
			std::vector<std::vector<std::vector<uint8_t> > >& native_shader_blocks);
		void StreamOut(std::ostream& os, uint32_t tech_index, uint32_t pass_index,
			std::vector<std::vector<std::vector<uint8_t> > > const & native_shader_blocks);

		RenderPassPtr Clone(RenderEffect& effect);

		std::string const & Name() const
		{
			return *name_;
		}

		void Bind();
		void Unbind();

		bool Validate() const
		{
			return is_validate_;
		}

		RasterizerStateObjectPtr const & GetRasterizerStateObject() const
		{
			return rasterizer_state_obj_;
		}
		DepthStencilStateObjectPtr const & GetDepthStencilStateObject() const
		{
			return depth_stencil_state_obj_;
		}
		BlendStateObjectPtr const & GetBlendStateObject() const
		{
			return blend_state_obj_;
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

	private:
		RenderEffect& effect_;

		shared_ptr<std::string> name_;
		shared_ptr<std::vector<RenderEffectAnnotationPtr> > annotations_;
		shared_ptr<std::vector<uint32_t> > shader_desc_ids_;

		RasterizerStateObjectPtr rasterizer_state_obj_;
		DepthStencilStateObjectPtr depth_stencil_state_obj_;
		uint16_t front_stencil_ref_, back_stencil_ref_;
		BlendStateObjectPtr blend_state_obj_;
		Color blend_factor_;
		uint32_t sample_mask_;
		ShaderObjectPtr shader_obj_;

		bool is_validate_;
	};

	class RenderEffectParameter : boost::noncopyable
	{
	public:
		explicit RenderEffectParameter(RenderEffect& effect);
		~RenderEffectParameter();

		void Load(XMLNodePtr const & node);

		void StreamIn(ResIdentifierPtr const & res);
		void StreamOut(std::ostream& os);

		RenderEffectParameterPtr Clone(RenderEffect& effect);

		uint32_t Type() const
		{
			return type_;
		}

		RenderVariablePtr const & Var() const
		{
			return var_;
		}

		shared_ptr<std::string> const & ArraySize() const
		{
			return array_size_;
		}

		shared_ptr<std::string> const & Name() const
		{
			return name_;
		}
		shared_ptr<std::string> const & Semantic() const
		{
			return semantic_;
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

	private:
		RenderEffect& effect_;

		shared_ptr<std::string> name_;
		shared_ptr<std::string> semantic_;

		uint32_t type_;
		shared_ptr<RenderVariable> var_;
		shared_ptr<std::string> array_size_;

		shared_ptr<std::vector<RenderEffectAnnotationPtr> > annotations_;
	};
}

#endif		// _KLAYGE_FXMLJIT_EFFECT_HPP
