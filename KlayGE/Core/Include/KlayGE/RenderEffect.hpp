// RenderEffect.hpp
// KlayGE 渲染效果脚本类 头文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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

#include <KlayGE/PreDeclare.hpp>
#include <vector>
#include <map>
#include <string>

#pragma warning(disable: 4100 4512)
#include <boost/utility.hpp>
#include <boost/any.hpp>

#include <KlayGE/Math.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class RenderEffect
	{
	private:
		typedef std::map<std::string, std::pair<RenderEffectParameterPtr, bool> > params_type;
		typedef std::vector<RenderTechniquePtr> techniques_type;
		
	public:
		RenderEffect();
		virtual ~RenderEffect()
			{ }

		static RenderEffectPtr NullObject();

		RenderEffectParameterPtr ParameterByName(std::string const & name);
		RenderEffectParameterPtr ParameterBySemantic(std::string const & semantic);

		bool ValidateTechnique(std::string const & name);
		void ActiveTechnique(std::string const & name);
		RenderTechniquePtr ActiveTechnique() const;

		uint32_t Begin(uint32_t flags = 0);
		void End();

		void DirtyParam(std::string const & name);

	private:
		virtual std::string DoNameBySemantic(std::string const & semantic) = 0;
		virtual RenderEffectParameterPtr DoParameterByName(std::string const & name) = 0;

		virtual uint32_t DoBegin(uint32_t flags) = 0;
		virtual void DoEnd() = 0;

		virtual void DoActiveTechnique() = 0;

	protected:
		techniques_type::iterator TechniqueByName(std::string const & name);

	protected:
		params_type params_;

		techniques_type techniques_;
		int32_t active_tech_;
	};

	class RenderTechnique
	{
	public:
		RenderTechnique(RenderEffect& effect, std::string const & name)
			: effect_(effect), name_(name)
		{
		}
		virtual ~RenderTechnique()
		{
		}

		std::string const & Name() const
		{
			return name_;
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

		virtual bool Validate() = 0;

	protected:
		std::string name_;

		RenderEffect& effect_;

		typedef std::vector<RenderPassPtr> passes_type;
		passes_type passes_;
	};

	class RenderPass
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
		uint32_t index_;

		RenderEffect& effect_;
	};

	class RenderEffectParameter : boost::noncopyable
	{
	public:
		RenderEffectParameter(RenderEffect& effect, std::string const & name);
		virtual ~RenderEffectParameter();

		static RenderEffectParameterPtr NullObject();

		std::string const & Name() const;

		virtual RenderEffectParameter& operator=(float const & value);
		virtual RenderEffectParameter& operator=(Vector3 const & value);
		virtual RenderEffectParameter& operator=(Vector4 const & value);
		virtual RenderEffectParameter& operator=(Matrix4 const & value);
		virtual RenderEffectParameter& operator=(int const & value);
		virtual RenderEffectParameter& operator=(SamplerPtr const & value);
		virtual RenderEffectParameter& operator=(std::vector<float> const & value);
		virtual RenderEffectParameter& operator=(std::vector<Vector4> const & value);
		virtual RenderEffectParameter& operator=(std::vector<Matrix4> const & value);
		virtual RenderEffectParameter& operator=(std::vector<int> const & value);

		virtual void Value(float& val) const;
		virtual void Value(Vector3& val) const;
		virtual void Value(Vector4& val) const;
		virtual void Value(Matrix4& val) const;
		virtual void Value(int& val) const;
		virtual void Value(SamplerPtr& val) const;
		virtual void Value(std::vector<float>& val) const;
		virtual void Value(std::vector<Vector4>& val) const;
		virtual void Value(std::vector<Matrix4>& val) const;
		virtual void Value(std::vector<int>& val) const;

		virtual void Flush() = 0;

	protected:
		virtual void DoFlush(float const & value);
		virtual void DoFlush(Vector3 const & value);
		virtual void DoFlush(Vector4 const & value);
		virtual void DoFlush(Matrix4 const & value);
		virtual void DoFlush(int const & value);
		virtual void DoFlush(SamplerPtr const & value);
		virtual void DoFlush(std::vector<float> const & value);
		virtual void DoFlush(std::vector<Vector4> const & value);
		virtual void DoFlush(std::vector<Matrix4> const & value);
		virtual void DoFlush(std::vector<int> const & value);

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
