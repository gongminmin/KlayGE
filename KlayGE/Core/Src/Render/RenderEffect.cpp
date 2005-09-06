// RenderEffect.cpp
// KlayGE 渲染效果类 实现文件
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
// 2.2.0
// 统一使用istream作为资源标示符 (2004.10.26)
//
// 2.1.2
// 增加了Parameter (2004.5.26)
//
// 2.0.3
// 初次建立 (2003.3.2)
// 修改了SetTexture的参数 (2004.3.6)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <KlayGE/RenderEffect.hpp>

namespace KlayGE
{
	class NullRenderEffect : public RenderEffect
	{
	private:
		std::string DoNameBySemantic(std::string const & /*semantic*/)
			{ return ""; }
		RenderEffectParameterPtr DoParameterByName(std::string const & /*name*/)
			{ return RenderEffectParameter::NullObject(); }

		uint32_t DoBegin(uint32_t /*flags*/)
			{ return 0; }
		void DoEnd()
			{ }

		void DoActiveTechnique()
		{
		}
	};

	RenderEffect::RenderEffect()
		: active_tech_(0)
	{
	}

	RenderEffectPtr RenderEffect::NullObject()
	{
		static RenderEffectPtr obj(new NullRenderEffect);
		return obj;
	}

	RenderEffectParameterPtr RenderEffect::ParameterByName(std::string const & name)
	{
		params_type::iterator iter = params_.find(name);
		if (iter != params_.end())
		{
			return iter->second.first;
		}
		else
		{
			RenderEffectParameterPtr ret = this->DoParameterByName(name);
			params_.insert(std::make_pair(name, std::make_pair(ret, true)));
			return ret;
		}
	}

	RenderEffectParameterPtr RenderEffect::ParameterBySemantic(std::string const & semantic)
	{
		return this->ParameterByName(this->DoNameBySemantic(semantic));
	}

	RenderEffect::techniques_type::iterator RenderEffect::TechniqueByName(std::string const & name)
	{
		for (techniques_type::iterator iter = techniques_.begin(); iter != techniques_.end(); ++ iter)
		{
			if (name == (*iter)->Name())
			{
				return iter;
			}
		}
		return techniques_.end();
	}

	bool RenderEffect::ValidateTechnique(std::string const & name)
	{
		techniques_type::iterator iter = this->TechniqueByName(name);
		return (*iter)->Validate();
	}

	void RenderEffect::ActiveTechnique(std::string const & name)
	{
		techniques_type::iterator iter = this->TechniqueByName(name);
		active_tech_ = (iter == techniques_.end()) ? 0 : static_cast<int32_t>(iter - techniques_.begin());

		this->DoActiveTechnique();
	}

	RenderTechniquePtr RenderEffect::ActiveTechnique() const
	{
		return techniques_[active_tech_];
	}

	void RenderEffect::DirtyParam(std::string const& name)
	{
		BOOST_ASSERT(params_.find(name) != params_.end());

		params_[name].second = true;
	}

	uint32_t RenderEffect::Begin(uint32_t flags)
	{
		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			if (iter->second.second)
			{
				iter->second.first->Flush();
				iter->second.second = false;
			}
		}

		return this->DoBegin(flags);
	}

	void RenderEffect::End()
	{
		return this->DoEnd();
	}


	class NullRenderEffectParameter : public RenderEffectParameter
	{
	public:
		NullRenderEffectParameter()
			: RenderEffectParameter(*RenderEffect::NullObject(), "")
		{
		}

		RenderEffectParameter& operator=(float const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(Vector4 const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(Matrix4 const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(int const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(SamplerPtr const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<float> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<Vector4> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<Matrix4> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<int> const & /*value*/)
		{
			return *this;
		}
		
		void Value(float& val) const
		{
			val = 0;
		}
		void Value(Vector4& val) const
		{
			val = Vector4::Zero();
		}
		void Value(Matrix4& val) const
		{
			val = Matrix4::Identity();
		}
		void Value(int& val) const
		{
			val = 0;
		}
		void Value(SamplerPtr& val) const
		{
			val = SamplerPtr();
		}
		void Value(std::vector<float>& val) const
		{
			val.clear();
		}
		void Value(std::vector<Vector4>& val) const
		{
			val.clear();
		}
		void Value(std::vector<Matrix4>& val) const
		{
			val.clear();
		}
		void Value(std::vector<int>& val) const
		{
			val.clear();
		}

		void Flush()
		{
		}

	private:
		NullRenderEffectParameter(NullRenderEffectParameter const & rhs);
		NullRenderEffectParameter& operator=(NullRenderEffectParameter const & rhs);
	};


	RenderEffectParameter::RenderEffectParameter(RenderEffect& effect, std::string const & name)
		: effect_(effect), name_(name)
	{
	}

	RenderEffectParameter::~RenderEffectParameter()
	{
	}

	RenderEffectParameterPtr RenderEffectParameter::NullObject()
	{
		static RenderEffectParameterPtr obj(new NullRenderEffectParameter);
		return obj;
	}

	std::string const & RenderEffectParameter::Name() const
	{
		return name_;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float const & value)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(Vector4 const & value)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(Matrix4 const & value)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(int const & value)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(SamplerPtr const & value)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<float> const & value)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<Vector4> const & value)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<Matrix4> const & value)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<int> const & value)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	void RenderEffectParameter::Value(float& val) const
	{
		BOOST_ASSERT(false);
		val = 0;
	}

	void RenderEffectParameter::Value(Vector4& val) const
	{
		BOOST_ASSERT(false);
		val = Vector4::Zero();
	}

	void RenderEffectParameter::Value(Matrix4& val) const
	{
		BOOST_ASSERT(false);
		val = Matrix4::Identity();
	}

	void RenderEffectParameter::Value(int& val) const
	{
		BOOST_ASSERT(false);
		val = 0;
	}

	void RenderEffectParameter::Value(SamplerPtr& val) const
	{
		BOOST_ASSERT(false);
		val = SamplerPtr();
	}

	void RenderEffectParameter::Value(std::vector<float>& val) const
	{
		BOOST_ASSERT(false);
		val.clear();
	}

	void RenderEffectParameter::Value(std::vector<Vector4>& val) const
	{
		BOOST_ASSERT(false);
		val.clear();
	}

	void RenderEffectParameter::Value(std::vector<Matrix4>& val) const
	{
		BOOST_ASSERT(false);
		val.clear();
	}

	void RenderEffectParameter::Value(std::vector<int>& val) const
	{
		BOOST_ASSERT(false);
		val.clear();
	}

	void RenderEffectParameter::DoFlush(float const & value)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(Vector4 const & value)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(Matrix4 const & value)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(int const & value)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(SamplerPtr const & value)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(std::vector<float> const & value)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(std::vector<Vector4> const & value)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(std::vector<Matrix4> const & value)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(std::vector<int> const & value)
	{
		BOOST_ASSERT(false);
	}
}
