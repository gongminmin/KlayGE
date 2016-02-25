#ifndef _MTL_EDITOR_CORE_COMMANDS_HPP
#define _MTL_EDITOR_CORE_COMMANDS_HPP

#pragma once

namespace KlayGE
{
	class MtlEditorCore;

	enum MtlEditorCommandCode
	{
		ECC_SetCurrFrame,
		ECC_SelectMesh,
		ECC_SetAmbientMaterial,
		ECC_SetDiffuseMaterial,
		ECC_SetSpecularMaterial,
		ECC_SetShininessMaterial,
		ECC_SetEmitMaterial,
		ECC_SetOpacityMaterial,
		ECC_SetDiffuseTexture,
		ECC_SetSpecularTexture,
		ECC_SetShininessTexture,
		ECC_SetNormalTexture,
		ECC_SetHeightTexture,
		ECC_SetEmitTexture,
		ECC_SetOpacityTexture,

		ECC_NumCommands
	};

	extern char const * editor_command_name[ECC_NumCommands];

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommand
	{
	public:
		explicit MtlEditorCommand(MtlEditorCore* core);
		~MtlEditorCommand()
		{
		}

		virtual char const * Name() const = 0;
		virtual MtlEditorCommandCode Code() const = 0;

		virtual void Execute() = 0;
		virtual void Revoke() = 0;

	protected:
		MtlEditorCore* core_;
	};

	typedef std::shared_ptr<MtlEditorCommand> MtlEditorCommandPtr;

	template <MtlEditorCommandCode code>
	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandConcrete : public MtlEditorCommand
	{
	public:
		explicit MtlEditorCommandConcrete(MtlEditorCore* core)
			: MtlEditorCommand(core)
		{
		}

		virtual char const * Name() const override
		{
			return editor_command_name[code];
		}
		virtual MtlEditorCommandCode Code() const override
		{
			return code;
		}
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetCurrFrame : public MtlEditorCommandConcrete<ECC_SetCurrFrame>
	{
	public:
		MtlEditorCommandSetCurrFrame(MtlEditorCore* core, float frame);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		float frame_;
		float old_frame_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSelectMesh : public MtlEditorCommandConcrete<ECC_SelectMesh>
	{
	public:
		MtlEditorCommandSelectMesh(MtlEditorCore* core, uint32_t mesh_id);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mesh_id_;
		uint32_t old_mesh_id_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetAmbientMaterial : public MtlEditorCommandConcrete<ECC_SetAmbientMaterial>
	{
	public:
		MtlEditorCommandSetAmbientMaterial(MtlEditorCore* core, uint32_t mtl_id, float* value);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		float3 ambient_;
		float3 old_ambient_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetDiffuseMaterial : public MtlEditorCommandConcrete<ECC_SetDiffuseMaterial>
	{
	public:
		MtlEditorCommandSetDiffuseMaterial(MtlEditorCore* core, uint32_t mtl_id, float* value);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		float3 diffuse_;
		float3 old_diffuse_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetSpecularMaterial : public MtlEditorCommandConcrete<ECC_SetSpecularMaterial>
	{
	public:
		MtlEditorCommandSetSpecularMaterial(MtlEditorCore* core, uint32_t mtl_id, float* value);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		float3 specular_;
		float3 old_specular_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetShininessMaterial : public MtlEditorCommandConcrete<ECC_SetShininessMaterial>
	{
	public:
		MtlEditorCommandSetShininessMaterial(MtlEditorCore* core, uint32_t mtl_id, float value);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		float shininess_;
		float old_shininess_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetEmitMaterial : public MtlEditorCommandConcrete<ECC_SetEmitMaterial>
	{
	public:
		MtlEditorCommandSetEmitMaterial(MtlEditorCore* core, uint32_t mtl_id, float* value);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		float3 emit_;
		float3 old_emit_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetOpacityMaterial : public MtlEditorCommandConcrete<ECC_SetOpacityMaterial>
	{
	public:
		MtlEditorCommandSetOpacityMaterial(MtlEditorCore* core, uint32_t mtl_id, float value);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		float opacity_;
		float old_opacity_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetDiffuseTexture : public MtlEditorCommandConcrete<ECC_SetDiffuseTexture>
	{
	public:
		MtlEditorCommandSetDiffuseTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		std::string name_;
		std::string old_name_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetSpecularTexture : public MtlEditorCommandConcrete<ECC_SetSpecularTexture>
	{
	public:
		MtlEditorCommandSetSpecularTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		std::string name_;
		std::string old_name_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetShininessTexture : public MtlEditorCommandConcrete<ECC_SetShininessTexture>
	{
	public:
		MtlEditorCommandSetShininessTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		std::string name_;
		std::string old_name_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetNormalTexture : public MtlEditorCommandConcrete<ECC_SetNormalTexture>
	{
	public:
		MtlEditorCommandSetNormalTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		std::string name_;
		std::string old_name_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetHeightTexture : public MtlEditorCommandConcrete<ECC_SetHeightTexture>
	{
	public:
		MtlEditorCommandSetHeightTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		std::string name_;
		std::string old_name_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetEmitTexture : public MtlEditorCommandConcrete<ECC_SetEmitTexture>
	{
	public:
		MtlEditorCommandSetEmitTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		std::string name_;
		std::string old_name_;
	};

	class KLAYGE_MTL_EDITOR_CORE_API MtlEditorCommandSetOpacityTexture : public MtlEditorCommandConcrete<ECC_SetOpacityTexture>
	{
	public:
		MtlEditorCommandSetOpacityTexture(MtlEditorCore* core, uint32_t mtl_id, char const * name);

		virtual void Execute() override;
		virtual void Revoke() override;

	private:
		uint32_t mtl_id_;
		std::string name_;
		std::string old_name_;
	};
}

#endif		// _MTL_EDITOR_CORE_COMMANDS_HPP

