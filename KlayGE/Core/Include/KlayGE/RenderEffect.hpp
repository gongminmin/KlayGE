// RenderEffect.hpp
// KlayGE ��ȾЧ���ű��� ͷ�ļ�
// Ver 3.11.0
// ��Ȩ����(C) ������, 2003-2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// Add RenderTechnique::Transparent() (2010.9.12)
//
// 3.9.0
// ֱ�Ӵ�fxml�ļ���ȡ��Ч�ű� (2009.4.21)
//
// 3.8.0
// ֧��CBuffer (2008.10.6)
//
// 3.6.0
// ������Clone (2007.6.11)
//
// 3.5.0
// ���û���xml����Ч��ʽ (2006.10.21)
//
// 3.4.0
// ��д��parameter�Ĵ洢�ṹ (2006.9.15)
//
// 3.2.0
// ֧����bool���� (2006.3.8)
//
// 3.0.0
// ������RenderTechnique��RenderPass (2005.9.4)
//
// 2.8.0
// ������Do*������ʹ��ģ�巽��ģʽ (2005.7.24)
// ʹ���µ��Զ����²����ķ��� (2005.7.25)
//
// 2.5.0
// ȥ����Clone (2005.4.16)
// SetTechnique�ķ������͸�Ϊbool (2005.4.25)
//
// 2.1.2
// ������Parameter (2004.5.26)
//
// 2.0.3
// �޸���SetTexture�Ĳ��� (2004.3.6)
// ������SetMatrixArray/GetMatrixArray (2004.3.11)
//
// 2.0.0
// ���ν��� (2003.8.15)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDEREFFECT_HPP
#define _RENDEREFFECT_HPP

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

	struct KLAYGE_CORE_API TextureSubresource
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

	class KLAYGE_CORE_API RenderVariable
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
		virtual RenderVariable& operator=(std::function<TexturePtr()> const & value);
		virtual RenderVariable& operator=(SamplerStateObjectPtr const & value);
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
		virtual void Value(SamplerStateObjectPtr& val) const;
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

		virtual RenderVariablePtr Clone() KLAYGE_OVERRIDE
		{
			std::shared_ptr<RenderVariableConcrete<T>> ret = MakeSharedPtr<RenderVariableConcrete<T>>();
			if (in_cbuff_)
			{
				if (!ret->in_cbuff_)
				{
					ret->RetriveT().~T();
				}
				ret->data_ = data_;
			}
			ret->in_cbuff_ = in_cbuff_;
			T val;
			this->Value(val);
			*ret = val;
			return ret;
		}

		virtual RenderVariable& operator=(T const & value) KLAYGE_OVERRIDE
		{
			if (in_cbuff_)
			{
				T& val_in_cbuff = *(data_.cbuff_desc.cbuff->template VariableInBuff<T>(data_.cbuff_desc.offset));
				if (val_in_cbuff != value)
				{
					val_in_cbuff = value;
					data_.cbuff_desc.cbuff->Dirty(true);
				}
			}
			else
			{
				this->RetriveT() = value;
			}
			return *this;
		}

		virtual void Value(T& val) const KLAYGE_OVERRIDE
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
				uint32_t stride) KLAYGE_OVERRIDE
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

		virtual void RebindToCBuffer(RenderEffectConstantBuffer* cbuff) KLAYGE_OVERRIDE
		{
			BOOST_ASSERT(in_cbuff_);
			data_.cbuff_desc.cbuff = cbuff;
		}

		virtual bool InCBuffer() const KLAYGE_OVERRIDE
		{
			return in_cbuff_;
		}
		virtual uint32_t CBufferOffset() const KLAYGE_OVERRIDE
		{
			return data_.cbuff_desc.offset;
		}
		virtual uint32_t Stride() const KLAYGE_OVERRIDE
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
		virtual RenderVariablePtr Clone() KLAYGE_OVERRIDE;

		virtual RenderVariable& operator=(float4x4 const & value) KLAYGE_OVERRIDE;
		virtual void Value(float4x4& val) const KLAYGE_OVERRIDE;
	};

	template <typename T>
	class RenderVariableArray : public RenderVariableConcrete<std::vector<T>>
	{
	public:
		virtual RenderVariablePtr Clone() KLAYGE_OVERRIDE
		{
			std::shared_ptr<RenderVariableArray<T>> ret = MakeSharedPtr<RenderVariableArray<T>>();
			if (this->in_cbuff_)
			{
				if (!ret->in_cbuff_)
				{
					ret->RetriveT().~vector();
				}
				ret->RenderVariableConcrete<std::vector<T>>::data_ = this->data_;
			}
			ret->RenderVariableConcrete<std::vector<T>>::in_cbuff_ = this->in_cbuff_;
			std::vector<T> val;
			this->Value(val);
			*ret = val;
			return ret;
		}

		virtual RenderVariable& operator=(std::vector<T> const & value) KLAYGE_OVERRIDE
		{
			if (this->in_cbuff_)
			{
				uint8_t* target = this->data_.cbuff_desc.cbuff->template VariableInBuff<uint8_t>(this->data_.cbuff_desc.offset);

				size_ = static_cast<uint32_t>(value.size());
				for (size_t i = 0; i < value.size(); ++ i)
				{
					memcpy(target + i * this->data_.cbuff_desc.stride, &value[i], sizeof(value[i]));
				}

				this->data_.cbuff_desc.cbuff->Dirty(true);
			}
			else
			{
				this->RetriveT() = value;
			}
			return *this;
		}

		virtual void Value(std::vector<T>& val) const KLAYGE_OVERRIDE
		{
			if (this->in_cbuff_)
			{
				uint8_t const * src = this->data_.cbuff_desc.cbuff->template VariableInBuff<uint8_t>(this->data_.cbuff_desc.offset);

				val.resize(size_);
				for (size_t i = 0; i < size_; ++ i)
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
		virtual RenderVariablePtr Clone() KLAYGE_OVERRIDE;

		virtual RenderVariable& operator=(std::vector<float4x4> const & value) KLAYGE_OVERRIDE;
		virtual void Value(std::vector<float4x4>& val) const KLAYGE_OVERRIDE;

	private:
		uint32_t size_;
	};

	class RenderVariableTexture : public RenderVariable
	{
	public:
		virtual RenderVariablePtr Clone();

		virtual RenderVariable& operator=(TexturePtr const & value);
		virtual RenderVariable& operator=(TextureSubresource const & value);
		virtual RenderVariable& operator=(std::function<TexturePtr()> const & value);
		virtual RenderVariable& operator=(std::string const & value);

		virtual void Value(TexturePtr& val) const;
		virtual void Value(TextureSubresource& val) const;
		virtual void Value(std::string& val) const;

	protected:
		std::function<TexturePtr()> tl_;
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
	typedef RenderVariableConcrete<SamplerStateObjectPtr> RenderVariableSampler;
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


	class KLAYGE_CORE_API RenderEffectAnnotation
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

		std::shared_ptr<RenderVariable> var_;
	};

	class KLAYGE_CORE_API RenderShaderFunc
	{
	public:
		void Load(XMLNodePtr const & node);

		void StreamIn(ResIdentifierPtr const & res);
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

	// ��ȾЧ��
	//////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API RenderEffect
	{
	public:
		RenderEffect();

		void Load(std::string const & name);

		bool StreamIn(ResIdentifierPtr const & source);
		void StreamOut(std::ostream& os);

		RenderEffectPtr Clone();

		std::string const & ResName() const
		{
			return *res_name_;
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

		uint32_t NumShaders() const
		{
			return shaders_ ? static_cast<uint32_t>(shaders_->size()) : 0;
		}
		RenderShaderFunc const & ShaderByIndex(uint32_t n) const
		{
			BOOST_ASSERT(n < this->NumShaders());
			return (*shaders_)[n];
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
		std::shared_ptr<std::vector<RenderShaderFunc>> shaders_;

		RenderEffectPtr prototype_effect_;

		std::shared_ptr<std::vector<ShaderDesc>> shader_descs_;
	};

	class KLAYGE_CORE_API RenderTechnique : boost::noncopyable
	{
	public:
		explicit RenderTechnique(RenderEffect& effect)
			: effect_(effect)
		{
		}

		void Load(XMLNodePtr const & node, uint32_t tech_index);

		bool StreamIn(ResIdentifierPtr const & res, uint32_t tech_index);
		void StreamOut(std::ostream& os, uint32_t tech_index);

		RenderTechniquePtr Clone(RenderEffect& effect);

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
		std::shared_ptr<std::string> name_;
		size_t name_hash_;

		std::vector<RenderPassPtr> passes_;
		std::shared_ptr<std::vector<RenderEffectAnnotationPtr>> annotations_;
		std::shared_ptr<std::vector<std::pair<std::string, std::string>>> macros_;

		float weight_;
		bool transparent_;

		bool is_validate_;
		bool has_discard_;
		bool has_tessellation_;
	};

	class KLAYGE_CORE_API RenderPass : boost::noncopyable
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

		bool StreamIn(ResIdentifierPtr const & res, uint32_t tech_index, uint32_t pass_index);
		void StreamOut(std::ostream& os, uint32_t tech_index, uint32_t pass_index);

		RenderPassPtr Clone(RenderEffect& effect);

		std::string const & Name() const
		{
			return *name_;
		}
		size_t NameHash() const
		{
			return name_hash_;
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
		RenderEffect& effect_;

		std::shared_ptr<std::string> name_;
		size_t name_hash_;
		std::shared_ptr<std::vector<RenderEffectAnnotationPtr>> annotations_;
		std::shared_ptr<std::vector<std::pair<std::string, std::string>>> macros_;
		std::shared_ptr<std::vector<uint32_t>> shader_desc_ids_;

		RasterizerStateObjectPtr rasterizer_state_obj_;
		DepthStencilStateObjectPtr depth_stencil_state_obj_;
		uint16_t front_stencil_ref_, back_stencil_ref_;
		BlendStateObjectPtr blend_state_obj_;
		Color blend_factor_;
		uint32_t sample_mask_;
		ShaderObjectPtr shader_obj_;

		bool is_validate_;
	};

	class KLAYGE_CORE_API RenderEffectConstantBuffer : boost::noncopyable
	{
	public:
		RenderEffectConstantBuffer();
		~RenderEffectConstantBuffer();

		void Load(std::string const & name);

		void StreamIn(ResIdentifierPtr const & res);
		void StreamOut(std::ostream& os);

		RenderEffectConstantBufferPtr Clone(RenderEffect& src_effect, RenderEffect& dst_effect);

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

		void Dirty(bool dirty)
		{
			dirty_ = dirty;
		}
		bool Dirty() const
		{
			return dirty_;
		}

		void Update();
		GraphicsBufferPtr const & HWBuff() const
		{
			return hw_buff_;
		}
		void BindHWBuff(GraphicsBufferPtr const & buff);

	private:
		std::shared_ptr<std::string> name_;
		size_t name_hash_;
		std::shared_ptr<std::vector<uint32_t>> param_indices_;

		GraphicsBufferPtr hw_buff_;
		std::vector<uint8_t> buff_;
		bool dirty_;
	};

	class KLAYGE_CORE_API RenderEffectParameter : boost::noncopyable
	{
	public:
		explicit RenderEffectParameter();
		~RenderEffectParameter();

		void Load(XMLNodePtr const & node);

		void StreamIn(ResIdentifierPtr const & res);
		void StreamOut(std::ostream& os);

		RenderEffectParameterPtr Clone();

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

	KLAYGE_CORE_API RenderEffectPtr SyncLoadRenderEffect(std::string const & effect_name);
	KLAYGE_CORE_API std::function<RenderEffectPtr()> ASyncLoadRenderEffect(std::string const & effect_name);
}

#endif		// _RENDEREFFECT_HPP
