// RenderEffect.hpp
// KlayGE 渲染效果脚本类 头文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
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

#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4100 4512)
#endif
#include <boost/utility.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/Math.hpp>

namespace KlayGE
{
	enum RenderEffectTechFlags
	{
		RETF_RestoreDefalut = 0x01
	};

	class type_define
	{
	public:
		enum code
		{
			TC_bool = 0,
			TC_dword,
			TC_string,
			TC_sampler,
			TC_shader,
			TC_int,
			TC_int2,
			TC_int3,
			TC_int4,
			TC_float,
			TC_float2,
			TC_float2x2,
			TC_float2x3,
			TC_float2x4,
			TC_float3,
			TC_float3x2,
			TC_float3x3,
			TC_float3x4,
			TC_float4,
			TC_float4x2,
			TC_float4x3,
			TC_float4x4
		};

	public:
		static type_define& instance();

		uint32_t type_code(std::string const & name) const;
		std::string const & type_name(uint32_t code) const;

	private:
		type_define();

	private:
		std::vector<std::string> types_;
	};

	class render_states_define
	{
	public:
		static render_states_define& instance();

		RenderEngine::RenderStateType state_code(std::string const & name) const;
		std::string const & state_name(uint32_t code) const;

	private:
		render_states_define();

	private:
		std::vector<std::pair<std::string, std::string> > states_;
	};

	struct shader_desc
	{
		std::string profile;
		std::string func_name;

		friend bool operator==(shader_desc const & lhs, shader_desc const & rhs)
		{
			return (lhs.profile == rhs.profile) && (lhs.func_name == rhs.func_name);
		}
		friend bool operator!=(shader_desc const & lhs, shader_desc const & rhs)
		{
			return !(lhs == rhs);
		}
	};

	class RenderVariable
	{
	public:
		RenderVariable();
		virtual ~RenderVariable() = 0;

		void Dirty(bool dirty)
		{
			dirty_ = dirty;
		}
		bool IsDirty() const
		{
			return dirty_;
		}

		virtual RenderVariable& operator=(bool const & value);
		virtual RenderVariable& operator=(int const & value);
		virtual RenderVariable& operator=(float const & value);
		virtual RenderVariable& operator=(float2 const & value);
		virtual RenderVariable& operator=(float3 const & value);
		virtual RenderVariable& operator=(float4 const & value);
		virtual RenderVariable& operator=(float4x4 const & value);
		virtual RenderVariable& operator=(SamplerPtr const & value);
		virtual RenderVariable& operator=(std::string const & value);
		virtual RenderVariable& operator=(shader_desc const & value);
		virtual RenderVariable& operator=(std::vector<bool> const & value);
		virtual RenderVariable& operator=(std::vector<int> const & value);
		virtual RenderVariable& operator=(std::vector<float> const & value);
		virtual RenderVariable& operator=(std::vector<float4> const & value);
		virtual RenderVariable& operator=(std::vector<float4x4> const & value);

		virtual void Value(bool& val) const;
		virtual void Value(int& val) const;
		virtual void Value(float& val) const;
		virtual void Value(float2& val) const;
		virtual void Value(float3& val) const;
		virtual void Value(float4& val) const;
		virtual void Value(float4x4& val) const;
		virtual void Value(SamplerPtr& val) const;
		virtual void Value(std::string& val) const;
		virtual void Value(shader_desc& val) const;
		virtual void Value(std::vector<bool>& val) const;
		virtual void Value(std::vector<int>& val) const;
		virtual void Value(std::vector<float>& val) const;
		virtual void Value(std::vector<float4>& val) const;
		virtual void Value(std::vector<float4x4>& val) const;

	protected:
		bool dirty_;
	};

	template <typename T>
	class RenderVariableConcrete : public RenderVariable
	{
	public:
		RenderVariableConcrete& operator=(T const & value)
		{
			if (val_ != value)
			{
				val_ = value;
				this->Dirty(true);
			}
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
	typedef RenderVariableConcrete<int> RenderVariableInt;
	typedef RenderVariableConcrete<float> RenderVariableFloat;
	typedef RenderVariableConcrete<float2> RenderVariableFloat2;
	typedef RenderVariableConcrete<float3> RenderVariableFloat3;
	typedef RenderVariableConcrete<float4> RenderVariableFloat4;
	typedef RenderVariableConcrete<float4x4> RenderVariableFloat4x4;
	typedef RenderVariableConcrete<SamplerPtr> RenderVariableSampler;
	typedef RenderVariableConcrete<std::string> RenderVariableString;
	typedef RenderVariableConcrete<shader_desc> RenderVariableShader;
	typedef RenderVariableConcrete<std::vector<bool> > RenderVariableBoolArray;
	typedef RenderVariableConcrete<std::vector<int> > RenderVariableIntArray;
	typedef RenderVariableConcrete<std::vector<float> >  RenderVariableFloatArray;
	typedef RenderVariableConcrete<std::vector<float4> >  RenderVariableFloat4Array;
	typedef RenderVariableConcrete<std::vector<float4x4> >  RenderVariableFloat4x4Array;


	class RenderEffectAnnotation
	{
	public:
		void Load(ResIdentifierPtr const & source);

	private:
		uint32_t type_;
		std::string name_;

		boost::shared_ptr<RenderVariable> var_;
	};

	class RenderShaderFunc
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
	class RenderEffect
	{
	protected:
		typedef std::vector<RenderEffectParameterPtr> params_type;
		typedef std::vector<RenderTechniquePtr> techniques_type;

	public:
		RenderEffect();
		virtual ~RenderEffect()
			{ }

		void Load(ResIdentifierPtr const & source);

		static RenderEffectPtr NullObject();

		uint32_t NumParameters() const
		{
			return static_cast<uint32_t>(params_.size());
		}
		RenderEffectParameterPtr ParameterBySemantic(std::string const & semantic);
		RenderEffectParameterPtr ParameterByName(std::string const & name);
		RenderEffectParameterPtr ParameterByIndex(uint32_t n)
		{
			BOOST_ASSERT(n < params_.size());
			return params_[n];
		}

		uint32_t NumTechniques() const
		{
			return static_cast<uint32_t>(techniques_.size());
		}
		RenderTechniquePtr TechniqueByName(std::string const & name);
		RenderTechniquePtr TechniqueByIndex(uint32_t n)
		{
			BOOST_ASSERT(n < techniques_.size());
			return techniques_[n];
		}

		uint32_t NumShaders() const
		{
			return static_cast<uint32_t>(shaders_.size());
		}
		RenderShaderFunc ShaderByIndex(uint32_t n)
		{
			BOOST_ASSERT(n < shaders_.size());
			return shaders_[n];
		}

	protected:
		virtual RenderTechniquePtr MakeRenderTechnique() = 0;

	protected:
		params_type params_;
		techniques_type techniques_;

		std::vector<RenderShaderFunc> shaders_;
	};

	class RenderTechnique : boost::noncopyable
	{
	public:
		explicit RenderTechnique(RenderEffect& effect)
			: effect_(effect)
		{
		}
		virtual ~RenderTechnique()
		{
		}

		void Load(ResIdentifierPtr const & source);

		static RenderTechniquePtr NullObject();

		std::string const & Name() const
		{
			return name_;
		}

		RenderEffect& Effect() const
		{
			return effect_;
		}

		uint32_t NumPasses() const
		{
			return static_cast<uint32_t>(passes_.size());
		}
		RenderPassPtr Pass(uint32_t n)
		{
			BOOST_ASSERT(n < passes_.size());
			return passes_[n];
		}

		void Begin(uint32_t flags = RETF_RestoreDefalut);
		void End();

		bool Validate() const
		{
			return is_validate_;
		}

	protected:
		virtual RenderPassPtr MakeRenderPass() = 0;

		virtual void DoBegin(uint32_t flags) = 0;
		virtual void DoEnd() = 0;

	protected:
		RenderEffect& effect_;
		std::string name_;

		typedef std::vector<RenderPassPtr> passes_type;
		passes_type passes_;

		std::vector<boost::shared_ptr<RenderEffectAnnotation> > annotations_;
		float weight_;

		std::vector<RenderEngine::RenderStateType> changed_states_;

		bool is_validate_;
		bool restore_default_;
	};

	class RenderEffectState
	{
	public:
		void Load(ResIdentifierPtr const & source);

		uint32_t Type() const
		{
			return type_;
		}

		RenderEngine::RenderStateType State() const
		{
			return state_;
		}

		boost::shared_ptr<RenderVariable> const & Var() const
		{
			return var_;
		}

	private:
		uint32_t type_;
		RenderEngine::RenderStateType state_;

		boost::shared_ptr<RenderVariable> var_;
	};

	class RenderPass : boost::noncopyable
	{
	public:
		explicit RenderPass(RenderEffect& effect)
			: effect_(effect)
		{
		}
		virtual ~RenderPass()
		{
		}

		static RenderPassPtr NullObject();

		void Load(ResIdentifierPtr const & source);

		std::string const & Name() const
		{
			return name_;
		}

		uint32_t NumStates() const;
		RenderEffectState const & State(uint32_t state_id) const;

		void Begin();
		void End();

		bool Validate() const
		{
			return is_validate_;
		}

	private:
		virtual void DoRead() = 0;
		virtual void DoBegin() = 0;
		virtual void DoEnd() = 0;

	protected:
		RenderEffect& effect_;

		std::string name_;
		std::vector<boost::shared_ptr<RenderEffectAnnotation> > annotations_;
		std::vector<RenderEffectStatePtr> render_states_;

		bool is_validate_;
	};

	class RenderEffectParameter : boost::noncopyable
	{
	public:
		explicit RenderEffectParameter(RenderEffect& effect);
		virtual ~RenderEffectParameter();

		static RenderEffectParameterPtr NullObject();

		void Load(ResIdentifierPtr const & source);

		uint32_t type() const
		{
			return type_;
		}

		boost::shared_ptr<RenderVariable> const & var() const
		{
			return var_;
		}

		uint32_t ArraySize() const
		{
			return array_size_;
		}

		std::string const & Name() const
		{
			return name_;
		}
		std::string const & Semantic() const
		{
			return semantic_;
		}

		void Dirty(bool dirty)
		{
			var_->Dirty(dirty);
		}
		bool IsDirty() const
		{
			return var_->IsDirty();
		}

		virtual RenderEffectParameter& operator=(bool const & value);
		virtual RenderEffectParameter& operator=(int const & value);
		virtual RenderEffectParameter& operator=(float const & value);
		virtual RenderEffectParameter& operator=(float2 const & value);
		virtual RenderEffectParameter& operator=(float3 const & value);
		virtual RenderEffectParameter& operator=(float4 const & value);
		virtual RenderEffectParameter& operator=(float4x4 const & value);
		virtual RenderEffectParameter& operator=(TexturePtr const & value);
		virtual RenderEffectParameter& operator=(std::vector<bool> const & value);
		virtual RenderEffectParameter& operator=(std::vector<int> const & value);
		virtual RenderEffectParameter& operator=(std::vector<float> const & value);
		virtual RenderEffectParameter& operator=(std::vector<float4> const & value);
		virtual RenderEffectParameter& operator=(std::vector<float4x4> const & value);

		virtual void Value(bool& val) const;
		virtual void Value(int& val) const;
		virtual void Value(float& val) const;
		virtual void Value(float2& val) const;
		virtual void Value(float3& val) const;
		virtual void Value(float4& val) const;
		virtual void Value(float4x4& val) const;
		virtual void Value(SamplerPtr& val) const;
		virtual void Value(std::vector<bool>& val) const;
		virtual void Value(std::vector<int>& val) const;
		virtual void Value(std::vector<float>& val) const;
		virtual void Value(std::vector<float4>& val) const;
		virtual void Value(std::vector<float4x4>& val) const;

	protected:
		RenderEffect& effect_;
		std::string name_;
		std::string semantic_;

		uint32_t type_;
		boost::shared_ptr<RenderVariable> var_;
		uint32_t array_size_;

		std::vector<boost::shared_ptr<RenderEffectAnnotation> > annotations_;
	};
}

#endif		// _RENDEREFFECT_HPP
