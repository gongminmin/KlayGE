// RenderEffect.cpp
// KlayGE 渲染效果类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/ResLoader.hpp>

#include <KlayGE/RenderEffect.hpp>

namespace KlayGE
{
	class NullRenderEffect : public RenderEffect
	{
	private:
		uint32_t HashCode() const
		{
			return 0;
		}

		void Desc(uint32_t& parameters, uint32_t& techniques, uint32_t& functions)
		{
			parameters = 0;
			techniques = 0;
			functions = 0;
		}

		std::string DoNameBySemantic(std::string const & /*semantic*/)
			{ return ""; }
		RenderEffectParameterPtr DoParameterByName(std::string const & /*name*/)
			{ return RenderEffectParameter::NullObject(); }

		bool SetTechnique(std::string const & /*techName*/)
		{
			return false;
		}
		bool SetTechnique(uint32_t /*tech*/)
		{
			return false;
		}

		uint32_t Begin(uint32_t /*flags*/)
			{ return 0; }
		void End()
			{ }
		void BeginPass(uint32_t /*passNum*/)
			{ }
		void EndPass()
			{ }
	};

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
			params_[name] = std::make_pair(ret, true);
			return ret;
		}
	}

	RenderEffectParameterPtr RenderEffect::ParameterBySemantic(std::string const & semantic)
	{
		return this->ParameterByName(this->DoNameBySemantic(semantic));
	}

	void RenderEffect::DirtyParam(std::string const& name)
	{
		assert(params_.find(name) != params_.end());

		params_[name].second = true;
	}

	void RenderEffect::FlushParams()
	{
		for (params_type::iterator iter = params_.begin(); iter != params_.end(); ++ iter)
		{
			if (iter->second.second)
			{
				iter->second.first->DoFlush();
				iter->second.second = false;
			}
		}
	}


	class NullRenderEffectParameter : public RenderEffectParameter
	{
	public:
		NullRenderEffectParameter()
			: RenderEffectParameter(*RenderEffect::NullObject(), "")
		{
		}

	private:
		bool DoTestType(RenderEffectParameterType /*type*/)
		{
			return true;
		}

		void DoFloat(float /*value*/)
		{
		}
		void DoVector4(Vector4 const & /*value*/)
		{
		}
		void DoMatrix4(Matrix4 const & /*value*/)
		{
		}
		void DoInt(int /*value*/)
		{
		}
		void DoTexture(TexturePtr const & /*value*/)
		{
		}

		void DoSetFloatArray(float const * /*value*/, size_t /*count*/)
		{
		}
		void DoSetVector4Array(Vector4 const * /*value*/, size_t /*count*/)
		{
		}
		void DoSetMatrix4Array(Matrix4 const * /*value*/, size_t /*count*/)
		{
		}
		void DoSetIntArray(int const * /*value*/, size_t /*count*/)
		{
		}

	private:
		NullRenderEffectParameter(NullRenderEffectParameter const & rhs);
		NullRenderEffectParameter& operator=(NullRenderEffectParameter const & rhs);
	};

	RenderEffectParameterPtr RenderEffectParameter::NullObject()
	{
		static RenderEffectParameterPtr obj(new NullRenderEffectParameter);
		return obj;
	}

	RenderEffectParameter::RenderEffectParameter(RenderEffect& effect, std::string const & name)
		: effect_(effect), name_(name), type_(REPT_Unknown)
	{
	}

	RenderEffectParameter& RenderEffectParameter::operator=(float value)
	{
		assert(this->DoTestType(REPT_float));

		bool dirty = false;
		if (REPT_Unknown == type_)
		{
			type_ = REPT_float;
			dirty = true;
		}
		else
		{
			assert(REPT_float == type_);
			
			dirty = (this->ToFloat() != value);
		}

		if (dirty)
		{
			val_ = value;
			effect_.DirtyParam(name_);
		}

		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(Vector4 const & value)
	{
		assert(this->DoTestType(REPT_Vector4));

		bool dirty = false;
		if (REPT_Unknown == type_)
		{
			type_ = REPT_Vector4;
			dirty = true;
		}
		else
		{
			assert(REPT_Vector4 == type_);
			
			dirty = (this->ToVector4() != value);
		}

		if (dirty)
		{
			val_ = value;
			effect_.DirtyParam(name_);
		}

		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(Matrix4 const & value)
	{
		assert(this->DoTestType(REPT_Matrix4));

		bool dirty = false;
		if (REPT_Unknown == type_)
		{
			type_ = REPT_Matrix4;
			dirty = true;
		}
		else
		{
			assert(REPT_Matrix4 == type_);
			
			dirty = (this->ToMatrix4() != value);
		}

		if (dirty)
		{
			val_ = value;
			effect_.DirtyParam(name_);
		}

		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(int value)
	{
		assert(this->DoTestType(REPT_int));

		bool dirty = false;
		if (REPT_Unknown == type_)
		{
			type_ = REPT_int;
			dirty = true;
		}
		else
		{
			assert(REPT_int == type_);
			
			dirty = (this->ToInt() != value);
		}

		if (dirty)
		{
			val_ = value;
			effect_.DirtyParam(name_);
		}

		return *this;
	}

	RenderEffectParameter& RenderEffectParameter::operator=(TexturePtr const & value)
	{
		assert(this->DoTestType(REPT_Texture));

		bool dirty = false;
		if (REPT_Unknown == type_)
		{
			type_ = REPT_Texture;
			dirty = true;
		}
		else
		{
			assert(REPT_Texture == type_);
			
			dirty = (this->ToTexture() != value);
		}

		if (dirty)
		{
			val_ = value;
			effect_.DirtyParam(name_);
		}

		return *this;
	}

	float RenderEffectParameter::ToFloat() const
	{
		assert(REPT_float == type_);
		return boost::get<float>(val_);
	}

	Vector4 const & RenderEffectParameter::ToVector4() const
	{
		assert(REPT_Vector4 == type_);
		return boost::get<Vector4>(val_);
	}

	Matrix4 const & RenderEffectParameter::ToMatrix4() const
	{
		assert(REPT_Matrix4 == type_);
		return boost::get<Matrix4>(val_);
	}

	int RenderEffectParameter::ToInt() const
	{
		assert(REPT_int == type_);
		return boost::get<int>(val_);
	}

	TexturePtr const & RenderEffectParameter::ToTexture() const
	{
		assert(REPT_Texture == type_);
		return boost::get<TexturePtr>(val_);
	}

	void RenderEffectParameter::SetFloatArray(float const * value, size_t count)
	{
		assert(this->DoTestType(REPT_float_array));

		bool dirty = false;
		if (REPT_Unknown == type_)
		{
			type_ = REPT_float_array;
			dirty = true;
		}
		else
		{
			assert(REPT_float_array == type_);

			std::vector<float>& v = boost::get<std::vector<float> >(val_);
			dirty = !std::equal(v.begin(), v.end(), value);
		}

		if (dirty)
		{
			std::vector<float>& v = boost::get<std::vector<float> >(val_);
			v.assign(value, value + count);
			effect_.DirtyParam(name_);
		}
	}

	void RenderEffectParameter::GetFloatArray(float* value, size_t count)
	{
		assert(REPT_float_array == type_);

		std::vector<float>& v = boost::get<std::vector<float> >(val_);
		std::copy(v.begin(), v.begin() + count, value);
	}

	void RenderEffectParameter::SetVector4Array(Vector4 const * value, size_t count)
	{
		assert(this->DoTestType(REPT_Vector4_array));

		bool dirty = false;
		if (REPT_Unknown == type_)
		{
			type_ = REPT_Vector4_array;
			dirty = true;
		}
		else
		{
			assert(REPT_Vector4_array == type_);

			std::vector<Vector4>& v = boost::get<std::vector<Vector4> >(val_);
			dirty = !std::equal(v.begin(), v.end(), value);
		}

		if (dirty)
		{
			std::vector<Vector4>& v = boost::get<std::vector<Vector4> >(val_);
			v.assign(value, value + count);
			effect_.DirtyParam(name_);
		}
	}

	void RenderEffectParameter::GetVector4Array(Vector4* value, size_t count)
	{
		assert(REPT_Vector4_array == type_);

		std::vector<Vector4>& v = boost::get<std::vector<Vector4> >(val_);
		std::copy(v.begin(), v.begin() + count, value);
	}

	void RenderEffectParameter::SetMatrix4Array(Matrix4 const * value, size_t count)
	{
		assert(this->DoTestType(REPT_Matrix4_array));

		bool dirty = false;
		if (REPT_Unknown == type_)
		{
			type_ = REPT_Matrix4_array;
			dirty = true;
		}
		else
		{
			assert(REPT_Matrix4_array == type_);

			std::vector<Matrix4>& v = boost::get<std::vector<Matrix4> >(val_);
			dirty = !std::equal(v.begin(), v.end(), value);
		}

		if (dirty)
		{
			std::vector<Matrix4>& v = boost::get<std::vector<Matrix4> >(val_);
			v.assign(value, value + count);
			effect_.DirtyParam(name_);
		}
	}

	void RenderEffectParameter::GetMatrix4Array(Matrix4* value, size_t count)
	{
		assert(REPT_Matrix4_array == type_);

		std::vector<Matrix4>& v = boost::get<std::vector<Matrix4> >(val_);
		std::copy(v.begin(), v.begin() + count, value);
	}

	void RenderEffectParameter::SetIntArray(int const * value, size_t count)
	{
		assert(this->DoTestType(REPT_int_array));

		bool dirty = false;
		if (REPT_Unknown == type_)
		{
			type_ = REPT_int_array;
			dirty = true;
		}
		else
		{
			assert(REPT_int_array == type_);

			std::vector<int>& v = boost::get<std::vector<int> >(val_);
			dirty = !std::equal(v.begin(), v.end(), value);
		}

		if (dirty)
		{
			std::vector<int>& v = boost::get<std::vector<int> >(val_);
			v.assign(value, value + count);
			effect_.DirtyParam(name_);
		}
	}

	void RenderEffectParameter::GetIntArray(int* value, size_t count)
	{
		assert(REPT_int_array == type_);

		std::vector<int>& v = boost::get<std::vector<int> >(val_);
		std::copy(v.begin(), v.begin() + count, value);
	}

	void RenderEffectParameter::DoFlush()
	{
		switch (type_)
		{
		case REPT_float:
			this->DoFloat(boost::get<float>(val_));
			break;

		case REPT_Vector4:
			this->DoVector4(boost::get<Vector4>(val_));
			break;

		case REPT_Matrix4:
			this->DoMatrix4(boost::get<Matrix4>(val_));
			break;

		case REPT_int:
			this->DoInt(boost::get<int>(val_));
			break;

		case REPT_Texture:
			this->DoTexture(boost::get<TexturePtr>(val_));
			break;

		case REPT_float_array:
			{
				std::vector<float>& v = boost::get<std::vector<float> >(val_);
				this->DoSetFloatArray(&v[0], v.size());
			}
			break;

		case REPT_Vector4_array:
			{
				std::vector<Vector4>& v = boost::get<std::vector<Vector4> >(val_);
				this->DoSetVector4Array(&v[0], v.size());
			}
			break;

		case REPT_Matrix4_array:
			{
				std::vector<Matrix4>& v = boost::get<std::vector<Matrix4> >(val_);
				this->DoSetMatrix4Array(&v[0], v.size());
			}
			break;

		case REPT_int_array:
			{
				std::vector<int>& v = boost::get<std::vector<int> >(val_);
				this->DoSetIntArray(&v[0], v.size());
			}
			break;

		default:
			assert(false);
			break;
		}
	}


	RenderEffectPtr LoadRenderEffect(std::string const & effectName)
	{
		ResIdentifierPtr file(ResLoader::Instance().Load(effectName));

		file->seekg(0, std::ios_base::end);
		std::vector<char> data(file->tellg());
		file->seekg(0);
		file->read(&data[0], static_cast<std::streamsize>(data.size()));

		return Context::Instance().RenderFactoryInstance().MakeRenderEffect(std::string(data.begin(), data.end()));
	}
}
