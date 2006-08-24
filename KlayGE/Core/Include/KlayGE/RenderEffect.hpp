// RenderEffect.hpp
// KlayGE 渲染效果脚本类 头文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
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

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4100 4512)
#endif
#include <boost/utility.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/any.hpp>

#include <KlayGE/Math.hpp>

namespace KlayGE
{
	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class RenderEffect
	{
	protected:
		typedef std::map<std::string, std::pair<RenderEffectParameterPtr, bool> > params_type;
		typedef std::vector<RenderTechniquePtr> techniques_type;

	public:
		virtual ~RenderEffect()
			{ }

		static RenderEffectPtr NullObject();

		RenderEffectParameterPtr ParameterByName(std::string const & name);
		RenderEffectParameterPtr ParameterBySemantic(std::string const & semantic);

		bool ValidateTechnique(std::string const & name);

		uint32_t NumTechniques() const
		{
			return static_cast<uint32_t>(techniques_.size());
		}
		RenderTechniquePtr Technique(std::string const & name);
		RenderTechniquePtr Technique(uint32_t n) const
		{
			BOOST_ASSERT(n < techniques_.size());
			return techniques_[n];
		}

		void DirtyParam(std::string const & name);
		void FlushParams();

	private:
		virtual std::string DoNameBySemantic(std::string const & semantic) = 0;
		virtual RenderEffectParameterPtr DoParameterByName(std::string const & name) = 0;

	protected:
		params_type params_;

		techniques_type techniques_;
	};

	class RenderTechnique : boost::noncopyable
	{
	public:
		RenderTechnique(RenderEffect& effect, std::string const & name)
			: effect_(effect), name_(name)
		{
		}
		virtual ~RenderTechnique()
		{
		}

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

		uint32_t Begin(uint32_t flags = 0);
		void End();

		virtual bool Validate() = 0;

	private:
		virtual uint32_t DoBegin(uint32_t flags) = 0;
		virtual void DoEnd() = 0;

	protected:
		RenderEffect& effect_;
		std::string name_;

		typedef std::vector<RenderPassPtr> passes_type;
		passes_type passes_;
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

		uint32_t Index() const
		{
			return index_;
		}

		virtual void Begin() = 0;
		virtual void End() = 0;

	protected:
		RenderEffect& effect_;
		uint32_t index_;
	};

	class RenderEffectParameter : boost::noncopyable
	{
	public:
		RenderEffectParameter(RenderEffect& effect, std::string const & name);
		virtual ~RenderEffectParameter();

		static RenderEffectParameterPtr NullObject();

		std::string const & Name() const;

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

		virtual void Flush() = 0;

	protected:
		virtual void DoFlush(bool const & value);
		virtual void DoFlush(int const & value);
		virtual void DoFlush(float const & value);
		virtual void DoFlush(float2 const & value);
		virtual void DoFlush(float3 const & value);
		virtual void DoFlush(float4 const & value);
		virtual void DoFlush(float4x4 const & value);
		virtual void DoFlush(SamplerPtr const & value);
		virtual void DoFlush(std::vector<bool> const & value);
		virtual void DoFlush(std::vector<int> const & value);
		virtual void DoFlush(std::vector<float> const & value);
		virtual void DoFlush(std::vector<float4> const & value);
		virtual void DoFlush(std::vector<float4x4> const & value);

	protected:
		RenderEffect& effect_;
		std::string name_;
	};

	template <typename T>
	class RenderEffectParameterConcrete : public RenderEffectParameter
	{
	public:
		RenderEffectParameterConcrete(RenderEffect& effect, std::string const & name)
			: RenderEffectParameter(effect, name), val_()
		{
		}

		RenderEffectParameter& operator=(T const & value)
		{
			bool dirty = false;

			if (value != val_)
			{
				dirty = true;
				val_ = value;
				effect_.DirtyParam(name_);
			}

			return *this;
		}

		void Value(T& val) const
		{
			val = val_;
		}

		void Flush()
		{
			this->DoFlush(val_);
		}

	protected:
		T val_;

	private:
		RenderEffectParameterConcrete(RenderEffectParameterConcrete const & rhs);
		RenderEffectParameterConcrete& operator=(RenderEffectParameterConcrete const & rhs);
	};
}

#endif		// _RENDEREFFECT_HPP
