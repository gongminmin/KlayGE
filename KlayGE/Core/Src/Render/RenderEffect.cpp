// RenderEffect.cpp
// KlayGE 渲染效果类 实现文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/Config.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlaygE/RenderFactory.hpp>

#include <KlayGE/PackedFile/PackedFile.hpp>
#include <KlayGE/DiskFile/DiskFile.hpp>

#include <KlayGE/RenderEffect.hpp>

namespace KlayGE
{
	class NullRenderEffect : public RenderEffect
	{
	public:
		void Desc(UINT& parameters, UINT& techniques, UINT& functions)
		{
			parameters = 0;
			techniques = 0;
			functions = 0;
		}

		RenderEffectPtr Clone() const
			{ return RenderEffect::NullObject(); }

		RenderEffectParameterPtr Parameter(UINT index)
			{ return RenderEffectParameter::NullObject(); }
		RenderEffectParameterPtr ParameterByName(const std::string& name)
			{ return RenderEffectParameter::NullObject(); }
		RenderEffectParameterPtr ParameterBySemantic(const std::string& semantic)
			{ return RenderEffectParameter::NullObject(); }

		void SetTechnique(const std::string& techName)
			{ }
		void SetTechnique(UINT tech)
			{ }

		UINT Begin(UINT flags = 0)
			{ return 1; }
		void Pass(UINT passNum)
			{ }
		void End()
			{ }
	};

	RenderEffectPtr RenderEffect::NullObject()
	{
		static RenderEffectPtr obj(new NullRenderEffect);
		return obj;
	}


	class NullRenderEffectParameter : public RenderEffectParameter
	{
	public:
		void SetFloat(float value)
			{ }
		float GetFloat() const
			{ return 0; }
		void SetVector(const Vector4& value)
			{ }
		Vector4 GetVector() const
			{ return Vector4::Zero(); }
		void SetMatrix(const Matrix4& value)
			{ }
		Matrix4 GetMatrix() const
			{ return Matrix4::Identity(); }
		void SetMatrixArray(const Matrix4* matrices, size_t count)
			{ }
		void GetMatrixArray(Matrix4* matrices, size_t count)
			{ }
		void SetInt(int value)
			{ }
		int GetInt() const
			{ return 0; }
		void SetBool(bool value)
			{ }
		bool GetBool() const
			{ return false; }
		void SetString(const std::string& value)
			{ }
		std::string GetString() const
		{
			static std::string str;
			return str;
		}

		void SetTexture(const TexturePtr& tex)
			{ }
	};

	RenderEffectParameterPtr RenderEffectParameter::NullObject()
	{
		static RenderEffectParameterPtr obj(new NullRenderEffectParameter);
		return obj;
	}


	RenderEffectPtr LoadRenderEffect(const std::string& effectName)
	{
		VFilePtr file(ResLocator::Instance().Locate(effectName)->Load());

		std::vector<char> data(file->Length());
		file->Read(&data[0], data.size());

		return Context::Instance().RenderFactoryInstance().MakeRenderEffect(std::string(&data[0], data.size()));
	}
}
