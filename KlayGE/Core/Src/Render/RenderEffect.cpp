// RenderEffect.cpp
// KlayGE 渲染效果类 实现文件
// Ver 3.2.0
// 版权所有(C) 龚敏敏, 2003-2005
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
	};

	RenderEffectPtr RenderEffect::NullObject()
	{
		static RenderEffectPtr obj(new NullRenderEffect);
		return obj;
	}

	RenderEffectParameterPtr RenderEffect::ParameterByName(std::string const & name)
	{
		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			if (name == (*iter)->Name())
			{
				return *iter;
			}
		}
		return RenderEffectParameter::NullObject();
	}

	RenderEffectParameterPtr RenderEffect::ParameterBySemantic(std::string const & semantic)
	{
		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			if (semantic == (*iter)->Semantic())
			{
				return *iter;
			}
		}
		return RenderEffectParameter::NullObject();
	}

	RenderTechniquePtr RenderEffect::TechniqueByName(std::string const & name)
	{
		for (techniques_type::iterator iter = techniques_.begin(); iter != techniques_.end(); ++ iter)
		{
			if (name == (*iter)->Name())
			{
				return *iter;
			}
		}
		return RenderTechnique::NullObject();
	}

	void RenderEffect::FlushParams()
	{
		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			if ((*iter)->IsDirty())
			{
				(*iter)->Flush();
			}
		}
	}

	
	class NullRenderTechnique : public RenderTechnique
	{
	public:
		NullRenderTechnique()
			: RenderTechnique(*RenderEffect::NullObject(), "")
		{
		}

		bool Validate()
		{
			return true;
		}

	private:
		uint32_t DoBegin(uint32_t /*flags*/)
		{
			return 0;
		}
		void DoEnd()
		{
		}
	};

	RenderTechniquePtr RenderTechnique::NullObject()
	{
		static RenderTechniquePtr obj(new NullRenderTechnique);
		return obj;
	}

	uint32_t RenderTechnique::Begin(uint32_t flags)
	{
		effect_.FlushParams();
		return this->DoBegin(flags);
	}

	void RenderTechnique::End()
	{
		return this->DoEnd();
	}


	class NullRenderEffectParameter : public RenderEffectParameter
	{
	public:
		NullRenderEffectParameter()
			: RenderEffectParameter(*RenderEffect::NullObject(), "", "")
		{
		}

		RenderEffectParameter& operator=(bool const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(int const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(float const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(float2 const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(float3 const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(float4 const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(float4x4 const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(SamplerPtr const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<bool> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<int> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<float> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<float4> const & /*value*/)
		{
			return *this;
		}
		RenderEffectParameter& operator=(std::vector<float4x4> const & /*value*/)
		{
			return *this;
		}

		void Value(bool& val) const
		{
			val = false;
		}
		void Value(int& val) const
		{
			val = 0;
		}
		void Value(float& val) const
		{
			val = 0;
		}
		void Value(float2& val) const
		{
			val = float2::Zero();
		}
		void Value(float3& val) const
		{
			val = float3::Zero();
		}
		void Value(float4& val) const
		{
			val = float4::Zero();
		}
		void Value(float4x4& val) const
		{
			val = float4x4::Identity();
		}
		void Value(SamplerPtr& val) const
		{
			val = SamplerPtr();
		}
		void Value(std::vector<bool>& val) const
		{
			val.clear();
		}
		void Value(std::vector<int>& val) const
		{
			val.clear();
		}
		void Value(std::vector<float>& val) const
		{
			val.clear();
		}
		void Value(std::vector<float4>& val) const
		{
			val.clear();
		}
		void Value(std::vector<float4x4>& val) const
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


	RenderEffectParameter::RenderEffectParameter(RenderEffect& effect,
						std::string const & name, std::string const & semantic)
		: effect_(effect), name_(name), semantic_(semantic),
			dirty_(false)
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
	
	RenderEffectParameter& RenderEffectParameter::operator=(bool const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(int const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float2 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float3 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float4 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float4x4 const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(SamplerPtr const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<bool> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<int> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<float> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<float4> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(std::vector<float4x4> const & /*value*/)
	{
		BOOST_ASSERT(false);
		return *this;
	}

	void RenderEffectParameter::Value(bool& val) const
	{
		BOOST_ASSERT(false);
		val = false;
	}

	void RenderEffectParameter::Value(int& val) const
	{
		BOOST_ASSERT(false);
		val = 0;
	}

	void RenderEffectParameter::Value(float& val) const
	{
		BOOST_ASSERT(false);
		val = 0;
	}

	void RenderEffectParameter::Value(float2& val) const
	{
		BOOST_ASSERT(false);
		val = float2::Zero();
	}

	void RenderEffectParameter::Value(float3& val) const
	{
		BOOST_ASSERT(false);
		val = float3::Zero();
	}

	void RenderEffectParameter::Value(float4& val) const
	{
		BOOST_ASSERT(false);
		val = float4::Zero();
	}

	void RenderEffectParameter::Value(float4x4& val) const
	{
		BOOST_ASSERT(false);
		val = float4x4::Identity();
	}

	void RenderEffectParameter::Value(SamplerPtr& val) const
	{
		BOOST_ASSERT(false);
		val = SamplerPtr();
	}

	void RenderEffectParameter::Value(std::vector<bool>& val) const
	{
		BOOST_ASSERT(false);
		val.clear();
	}

	void RenderEffectParameter::Value(std::vector<int>& val) const
	{
		BOOST_ASSERT(false);
		val.clear();
	}

	void RenderEffectParameter::Value(std::vector<float>& val) const
	{
		BOOST_ASSERT(false);
		val.clear();
	}

	void RenderEffectParameter::Value(std::vector<float4>& val) const
	{
		BOOST_ASSERT(false);
		val.clear();
	}

	void RenderEffectParameter::Value(std::vector<float4x4>& val) const
	{
		BOOST_ASSERT(false);
		val.clear();
	}

	void RenderEffectParameter::DoFlush(bool const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(int const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(float const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(float2 const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(float3 const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(float4 const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(float4x4 const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(SamplerPtr const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(std::vector<bool> const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(std::vector<int> const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(std::vector<float> const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(std::vector<float4> const & /*value*/)
	{
		BOOST_ASSERT(false);
	}

	void RenderEffectParameter::DoFlush(std::vector<float4x4> const & /*value*/)
	{
		BOOST_ASSERT(false);
	}
}
