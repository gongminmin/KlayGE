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
		RenderEffectParameterPtr ParameterByName(std::string const & name)
			{ return RenderEffectParameter::NullObject(); }
		RenderEffectParameterPtr ParameterBySemantic(std::string const & semantic)
			{ return RenderEffectParameter::NullObject(); }

		void SetTechnique(std::string const & techName)
			{ }
		void SetTechnique(UINT tech)
			{ }

		UINT Begin(UINT flags = 0)
			{ return 1; }
		void BeginPass(UINT passNum)
			{ }
		void EndPass()
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
		RenderEffectParameter const & operator=(float value)
			{ return *this; }
		RenderEffectParameter const & operator=(Vector4 const & value)
			{ return *this; }
		RenderEffectParameter const & operator=(Matrix4 const & value)
			{ return *this; }
		RenderEffectParameter const & operator=(int value)
			{ return *this; }
		RenderEffectParameter const & operator=(TexturePtr const & tex)
			{ return *this; }

		operator float() const
			{ return 0; }
		operator Vector4() const
			{ return Vector4::Zero(); }
		operator Matrix4() const
			{ return Matrix4::Identity(); }
		operator int() const
			{ return 0; }

		void SetFloatArray(float const * matrices, size_t count)
			{ }
		void GetFloatArray(float* matrices, size_t count)
			{ }
		void SetVectorArray(Vector4 const * matrices, size_t count)
			{ }
		void GetVectorArray(Vector4* matrices, size_t count)
			{ }
		void SetMatrixArray(Matrix4 const * matrices, size_t count)
			{ }
		void GetMatrixArray(Matrix4* matrices, size_t count)
			{ }
		void SetIntArray(int const * matrices, size_t count)
			{ }
		void GetIntArray(int* matrices, size_t count)
			{ }
	};

	RenderEffectParameterPtr RenderEffectParameter::NullObject()
	{
		static RenderEffectParameterPtr obj(new NullRenderEffectParameter);
		return obj;
	}


	RenderEffectPtr LoadRenderEffect(std::string const & effectName)
	{
		VFilePtr file(ResLocator::Instance().Locate(effectName)->Load());

		std::vector<char> data(file->Length());
		file->Read(&data[0], data.size());

		return Context::Instance().RenderFactoryInstance().MakeRenderEffect(std::string(&data[0], data.size()));
	}
}
