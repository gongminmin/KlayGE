// RenderEffect.hpp
// KlayGE 渲染效果类 头文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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

		void SetValue(const std::string& name, const void* data, UINT bytes)
			{ }
		void* GetValue(const std::string& name, UINT bytes) const
			{ return NULL; }

		void SetFloat(const std::string& name, float value)
			{ }
		float GetFloat(const std::string& name) const
			{ return 0; }
		void SetVector(const std::string& name, const Vector4& value)
			{ }
		Vector4 GetVector(const std::string& name) const
			{ return Vector4::Zero(); }
		void SetMatrix(const std::string& name, const Matrix4& value)
			{ }
		Matrix4 GetMatrix(const std::string& name) const
			{ return Matrix4::Identity(); }
		void SetMatrixArray(const std::string& name, const Matrix4* matrices, size_t count)
			{ }
		void GetMatrixArray(const std::string& name, Matrix4* matrices, size_t count)
			{ }
		void SetInt(const std::string& name, int value)
			{ }
		int GetInt(const std::string& name) const
			{ return 0; }
		void SetBool(const std::string& name, bool value)
			{ }
		bool GetBool(const std::string& name) const
			{ return false; }
		void SetString(const std::string& name, const std::string& value)
			{ }
		std::string GetString(const std::string& name) const
		{
			static std::string str;
			return str;
		}

		void SetTexture(const std::string& name, const TexturePtr& tex)
			{ }

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

	RenderEffectPtr LoadRenderEffect(const std::string& effectName)
	{
		VFilePtr file(ResLocator::Instance().Locate(effectName)->Load());

		std::vector<char> data(file->Length());
		file->Read(&data[0], data.size());

		return Context::Instance().RenderFactoryInstance().MakeRenderEffect(std::string(&data[0], data.size()));
	}
}
