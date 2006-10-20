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
#include <boost/any.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/Math.hpp>

namespace KlayGE
{
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

	class states_define
	{
	public:
		static states_define& instance();

		uint32_t state_code(std::string const & name) const;
		std::string const & state_name(uint32_t code) const;

	private:
		states_define();

	private:
		std::vector<std::pair<std::string, std::string> > states_;
	};

	class RenderVariable
	{
	public:
		void set_value(boost::any const & p)
		{
			value_ = p;
		}
		boost::any const & get_value() const
		{
			return value_;
		}

	protected:
		boost::any value_;
	};

	struct shader_desc
	{
		std::string profile;
		std::string func_name;
	};

	class RenderAnnotation
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

		void Begin(uint32_t flags = 0);
		void End();

		bool Validate();

	protected:
		virtual RenderPassPtr MakeRenderPass(uint32_t index) = 0;

		virtual void DoBegin(uint32_t flags) = 0;
		virtual void DoEnd() = 0;

	protected:
		RenderEffect& effect_;
		std::string name_;

		typedef std::vector<RenderPassPtr> passes_type;
		passes_type passes_;

		std::vector<boost::shared_ptr<RenderAnnotation> > annotations_;
		float weight_;
	};

	class RenderState
	{
	public:
		void Load(ResIdentifierPtr const & source);

		uint32_t type() const
		{
			return type_;
		}

		uint32_t state() const
		{
			return state_;
		}

		boost::shared_ptr<RenderVariable> const & var() const
		{
			return var_;
		}

	private:
		uint32_t type_;
		uint32_t state_;

		boost::shared_ptr<RenderVariable> var_;
	};

	class RenderPass : boost::noncopyable
	{
	public:
		RenderPass(RenderEffect& effect, uint32_t index)
			: effect_(effect), index_(index)
		{
		}
		virtual ~RenderPass()
		{
		}

		static RenderPassPtr NullObject();

		void Load(ResIdentifierPtr const & source);

		std::string const & name() const
		{
			return name_;
		}

		uint32_t Index() const
		{
			return index_;
		}

		uint32_t NumStates() const;
		RenderState const & State(uint32_t state_id) const;

		virtual void Begin() = 0;
		virtual void End() = 0;

		bool Validate() const
		{
			return is_validate_;
		}

	private:
		virtual void DoRead() = 0;

	protected:
		RenderEffect& effect_;
		uint32_t index_;

		std::string name_;
		std::vector<boost::shared_ptr<RenderAnnotation> > annotations_;
		std::vector<boost::shared_ptr<RenderState> > render_states_;

		bool is_validate_;
	};

	class RenderEffectParameter : boost::noncopyable
	{
	public:
		RenderEffectParameter(RenderEffect& effect,
			std::string const & name, std::string const & semantic);
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
			dirty_ = dirty;
		}
		bool IsDirty() const
		{
			return dirty_;
		}

		virtual RenderEffectParameter& operator=(bool const & value);
		virtual RenderEffectParameter& operator=(int const & value);
		virtual RenderEffectParameter& operator=(float const & value);
		virtual RenderEffectParameter& operator=(float2 const & value);
		virtual RenderEffectParameter& operator=(float3 const & value);
		virtual RenderEffectParameter& operator=(float4 const & value);
		virtual RenderEffectParameter& operator=(float4x4 const & value);
		virtual RenderEffectParameter& operator=(SamplerPtr const & value);
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

		bool dirty_;

		uint32_t type_;
		boost::shared_ptr<RenderVariable> var_;
		uint32_t array_size_;

		std::vector<boost::shared_ptr<RenderAnnotation> > annotations_;
	};

	template <typename T>
	class RenderEffectParameterConcrete : public RenderEffectParameter
	{
	public:
		RenderEffectParameterConcrete(RenderEffect& effect,
					std::string const & name, std::string const & semantic)
			: RenderEffectParameter(effect, name, semantic), val_()
		{
		}

		RenderEffectParameter& operator=(T const & value)
		{
			if (value != val_)
			{
				val_ = value;
				this->Dirty();
			}

			return *this;
		}

		void Value(T& val) const
		{
			val = val_;
		}

	protected:
		T val_;

	private:
		RenderEffectParameterConcrete(RenderEffectParameterConcrete const & rhs);
		RenderEffectParameterConcrete& operator=(RenderEffectParameterConcrete const & rhs);
	};
}

#endif		// _RENDEREFFECT_HPP
