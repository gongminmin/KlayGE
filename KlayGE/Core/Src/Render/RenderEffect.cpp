#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
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

		void SetValue(const String& name, const void* data, UINT bytes)
			{ }
		void* GetValue(const String& name, UINT bytes) const
			{ return NULL; }

		void SetFloat(const String& name, float value)
			{ }
		float GetFloat(const String& name) const
			{ return 0; }
		void SetVector(const String& name, const Vector4& value)
			{ }
		Vector4 GetVector(const String& name) const
			{ return Vector4::Zero(); }
		void SetMatrix(const String& name, const Matrix4& value)
			{ }
		Matrix4 GetMatrix(const String& name) const
			{ return Matrix4::Identity(); }
		void SetInt(const String& name, int value)
			{ }
		int GetInt(const String& name) const
			{ return 0; }
		void SetBool(const String& name, bool value)
			{ }
		bool GetBool(const String& name) const
			{ return false; }
		void SetString(const String& name, const String& value)
			{ }
		String GetString(const String& name) const
		{
			static String str;
			return str;
		}

		void SetTexture(const String& name, const Texture& tex)
			{ }
		void SetVertexShader(const String& name, U32 vsHandle)
			{ }
		void SetPixelShader(const String& name, U32 psHandle)
			{ }

		void Technique(const String& technique)
			{ }
		void Technique(UINT technique)
			{ }
		void Validate()
			{ }

		UINT Begin(UINT flags = 0)
			{ return 1; }
		void Pass(UINT passNum)
			{ }
		void End()
			{ }
	};

	RenderEffectPtr NullRenderEffectInstance()
	{
		static RenderEffectPtr obj(new NullRenderEffect);
		return obj;
	}
}
