// RenderEffect.hpp
// KlayGE 渲染效果脚本类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2003-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 支持CBuffer (2008.10.6)
//
// 3.6.0
// 增加了Clone (2007.6.11)
//
// 3.5.0
// 改用基于xml的特效格式 (2006.10.21)
//
// 3.4.0
// 重写了parameter的存储结构 (2006.9.15)
//
// 3.2.0
// 支持了bool类型 (2006.3.8)
//
// 3.0.0
// 增加了RenderTechnique和RenderPass (2005.9.4)
//
// 2.8.0
// 增加了Do*函数，使用模板方法模式 (2005.7.24)
// 使用新的自动更新参数的方法 (2005.7.25)
//
// 2.5.0
// 去掉了Clone (2005.4.16)
// SetTechnique的返回类型改为bool (2005.4.25)
//
// 2.1.2
// 增加了Parameter (2004.5.26)
//
// 2.0.3
// 修改了SetTexture的参数 (2004.3.6)
// 增加了SetMatrixArray/GetMatrixArray (2004.3.11)
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDEREFFECT_HPP
#define _RENDEREFFECT_HPP

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <KlayGE/PreDeclare.hpp>
#include <vector>
#include <string>
#include <algorithm>

#include <boost/utility.hpp>

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/Math.hpp>

namespace KlayGE
{
	enum RenderEffectDataType
	{
		REDT_bool = 0,
		REDT_dword,
		REDT_string,
		REDT_texture1D,
		REDT_texture2D,
		REDT_texture3D,
		REDT_textureCUBE,
		REDT_sampler,
		REDT_shader,
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
		REDT_float4x4
	};

	class KLAYGE_CORE_API RenderVariable
	{
	public:
		RenderVariable();
		virtual ~RenderVariable() = 0;

		virtual RenderVariablePtr Clone() = 0;

		virtual RenderVariable& operator=(bool const & value);
		virtual RenderVariable& operator=(int32_t const & value);
		virtual RenderVariable& operator=(float const & value);
		virtual RenderVariable& operator=(float2 const & value);
		virtual RenderVariable& operator=(float3 const & value);
		virtual RenderVariable& operator=(float4 const & value);
		virtual RenderVariable& operator=(float4x4 const & value);
		virtual RenderVariable& operator=(TexturePtr const & value);
		virtual RenderVariable& operator=(SamplerStateObjectPtr const & value);
		virtual RenderVariable& operator=(std::string const & value);
		virtual RenderVariable& operator=(shader_desc const & value);
		virtual RenderVariable& operator=(std::vector<bool> const & value);
		virtual RenderVariable& operator=(std::vector<int32_t> const & value);
		virtual RenderVariable& operator=(std::vector<float> const & value);
		virtual RenderVariable& operator=(std::vector<float4> const & value);
		virtual RenderVariable& operator=(std::vector<float4x4> const & value);

		virtual void Value(bool& val) const;
		virtual void Value(int32_t& val) const;
		virtual void Value(float& val) const;
		virtual void Value(float2& val) const;
		virtual void Value(float3& val) const;
		virtual void Value(float4& val) const;
		virtual void Value(float4x4& val) const;
		virtual void Value(TexturePtr& val) const;
		virtual void Value(SamplerStateObjectPtr& val) const;
		virtual void Value(std::string& val) const;
		virtual void Value(shader_desc& val) const;
		virtual void Value(std::vector<bool>& val) const;
		virtual void Value(std::vector<int32_t>& val) const;
		virtual void Value(std::vector<float>& val) const;
		virtual void Value(std::vector<float4>& val) const;
		virtual void Value(std::vector<float4x4>& val) const;
	};

	template <typename T>
	class RenderVariableConcrete : public RenderVariable
	{
	public:
		RenderVariablePtr Clone()
		{
			RenderVariablePtr ret = MakeSharedPtr<RenderVariableConcrete<T> >();
			T val;
			this->Value(val);
			*ret = val;
			return ret;
		}

		RenderVariableConcrete& operator=(T const & value)
		{
			val_ = value;
			return *this;
		}

		void Value(T& val) const
		{
			val = val_;
		}

	protected:
		T val_;
	};

	typedef RenderVariableConcrete<bool> RenderVariableBool;
	typedef RenderVariableConcrete<int32_t> RenderVariableInt;
	typedef RenderVariableConcrete<float> RenderVariableFloat;
	typedef RenderVariableConcrete<float2> RenderVariableFloat2;
	typedef RenderVariableConcrete<float3> RenderVariableFloat3;
	typedef RenderVariableConcrete<float4> RenderVariableFloat4;
	typedef RenderVariableConcrete<float4x4> RenderVariableFloat4x4;
	typedef RenderVariableConcrete<TexturePtr> RenderVariableTexture;
	typedef RenderVariableConcrete<SamplerStateObjectPtr> RenderVariableSampler;
	typedef RenderVariableConcrete<std::string> RenderVariableString;
	typedef RenderVariableConcrete<shader_desc> RenderVariableShader;
	typedef RenderVariableConcrete<std::vector<bool> > RenderVariableBoolArray;
	typedef RenderVariableConcrete<std::vector<int32_t> > RenderVariableIntArray;
	typedef RenderVariableConcrete<std::vector<float> >  RenderVariableFloatArray;
	typedef RenderVariableConcrete<std::vector<float4> >  RenderVariableFloat4Array;
	typedef RenderVariableConcrete<std::vector<float4x4> >  RenderVariableFloat4x4Array;


	class KLAYGE_CORE_API RenderEffectAnnotation
	{
	public:
		void Load(ResIdentifierPtr const & source);

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

		boost::shared_ptr<RenderVariable> var_;
	};

	class KLAYGE_CORE_API RenderShaderFunc
	{
	public:
		void Load(ResIdentifierPtr const & source);

		std::string const & str() const
		{
			return str_;
		}

	private:
		std::string str_;
	};

	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API RenderEffect
	{
	public:
		RenderEffect();

		void Load(ResIdentifierPtr const & source);
		RenderEffectPtr Clone();

		void PrototypeEffect(RenderEffectPtr prototype_effect)
		{
			prototype_effect_ = prototype_effect;
		}
		RenderEffectPtr const & PrototypeEffect() const
		{
			return prototype_effect_;
		}

		static RenderEffectPtr const & NullObject();

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

		std::string const & TypeName(uint32_t code) const;

	protected:
		std::vector<RenderEffectParameterPtr> params_;
		boost::shared_ptr<std::vector<std::pair<std::string, std::vector<uint32_t> > > > cbuffers_;
		std::vector<RenderTechniquePtr> techniques_;

		boost::shared_ptr<std::vector<RenderShaderFunc> > shaders_;

		RenderEffectPtr prototype_effect_;
	};

	class KLAYGE_CORE_API RenderTechnique : boost::noncopyable
	{
	public:
		explicit RenderTechnique(RenderEffect& effect)
			: effect_(effect)
		{
		}

		void Load(ResIdentifierPtr const & source);
		RenderTechniquePtr Clone(RenderEffect& effect);

		static RenderTechniquePtr const & NullObject();

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

	protected:
		RenderEffect& effect_;
		boost::shared_ptr<std::string> name_;

		std::vector<RenderPassPtr> passes_;
		boost::shared_ptr<std::vector<RenderEffectAnnotationPtr> > annotations_;

		float weight_;

		bool is_validate_;
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

		static RenderPassPtr const & NullObject();

		void Load(ResIdentifierPtr const & source);
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

	protected:
		RenderEffect& effect_;

		boost::shared_ptr<std::string> name_;
		boost::shared_ptr<std::vector<RenderEffectAnnotationPtr> > annotations_;
		boost::shared_ptr<std::vector<shader_desc> > shader_descs_;

		RasterizerStateObjectPtr rasterizer_state_obj_;
		DepthStencilStateObjectPtr depth_stencil_state_obj_;
		uint16_t front_stencil_ref_, back_stencil_ref_;
		BlendStateObjectPtr blend_state_obj_;
		Color blend_factor_;
		uint32_t sample_mask_;
		ShaderObjectPtr shader_obj_;

		bool is_validate_;
	};

	class KLAYGE_CORE_API RenderEffectParameter : boost::noncopyable
	{
	public:
		explicit RenderEffectParameter(RenderEffect& effect);
		~RenderEffectParameter();

		void Load(ResIdentifierPtr const & source);
		RenderEffectParameterPtr Clone(RenderEffect& effect);

		uint32_t type() const
		{
			return type_;
		}

		RenderVariablePtr const & var() const
		{
			return var_;
		}

		uint32_t ArraySize() const
		{
			return array_size_;
		}

		boost::shared_ptr<std::string> const & Name() const
		{
			return name_;
		}
		boost::shared_ptr<std::string> const & Semantic() const
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

	protected:
		RenderEffect& effect_;

		boost::shared_ptr<std::string> name_;
		boost::shared_ptr<std::string> semantic_;

		uint32_t type_;
		boost::shared_ptr<RenderVariable> var_;
		uint32_t array_size_;

		boost::shared_ptr<std::vector<RenderEffectAnnotationPtr> > annotations_;
	};
}

#endif		// _RENDEREFFECT_HPP
