/**
 * @file GLSLGen.cpp
 * @author Shenghua Lin, Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef _DXBC2GLSL_GLSLGEN_HPP
#define _DXBC2GLSL_GLSLGEN_HPP

#pragma once

#include <DXBC2GLSL/Shader.hpp>
#include <map>

enum GLSLVersion
{
	GSV_110 = 0,		// GL 2.0
	GSV_120,			// GL 2.1
	GSV_130,			// GL 3.0
	GSV_140,			// GL 3.1
	GSV_150,			// GL 3.2
	GSV_330,			// GL 3.3
	GSV_400,			// GL 4.0
	GSV_410,			// GL 4.1
	GSV_420,			// GL 4.2
	GSV_430,			// GL 4.3
	GSV_440,			// GL 4.4
	GSV_450,			// GL 4.5
	GSV_460,			// GL 4.6

	GSV_100_ES,			// GL ES 2.0
	GSV_300_ES,			// GL ES 3.0
	GSV_310_ES,			// GL ES 3.1
	GSV_320_ES,			// GL ES 3.2

	GSV_NumVersions
};

enum GLSLRules : uint32_t
{
	GSR_UniformBlockBinding = 1UL << 0,		// Set means allow uniform block layout bindings e.g.layout(binding=N) uniform {};.
	GSR_GlobalUniformsInUBO = 1UL << 1,		// Set means collect global uniforms in uniform block named $Globals.
	GSR_UseUBO = 1UL << 2,					// Set means generating uniform blocks.
	GSR_ExplicitPSOutputLayout = 1UL << 3,
	GSR_ExplicitInputLayout = 1UL << 4,
	GSR_UIntType = 1UL << 5,
	GSR_GenericTexture = 1UL << 6,
	GSR_PSInterpolation = 1UL << 7,
	GSR_InOutPrefix = 1UL << 8,
	GSR_Int64Type = 1UL << 9,
	GSR_TextureGrad = 1UL << 10,
	GSR_BitwiseOp = 1UL << 11,
	GSR_MultiStreamGS = 1UL << 12,
	GSR_CoreGS = 1UL << 13,
	GSR_Precision = 1UL << 14,
	GSR_VersionDecl = 1UL << 15,
	GSR_MatrixType = 1UL << 16,
	GSR_ArrayConstructors = 1UL << 17,
	GSR_DrawBuffers = 1UL << 18,
	GSR_EXTShaderTextureLod = 1UL << 19,
	GSR_EXTDrawBuffers = 1UL << 20,
	GSR_OESStandardDerivatives = 1UL << 21,
	GSR_EXTFragDepth = 1UL << 22,
	GSR_EXTTessellationShader = 1UL << 23,
	GSR_PrecisionOnSampler = 1UL << 24,
	GSR_ExplicitMultiSample = 1UL << 25,
	GSR_EXTVertexShaderLayer = 1UL << 26,
};

struct RegisterDesc
{
	uint32_t index;
	ShaderRegisterComponentType type;
	ShaderInterpolationMode interpolation;
	bool is_depth;
};

struct HSForkPhase
{
	uint32_t fork_instance_count;
	std::vector<std::shared_ptr<ShaderDecl>> dcls;
	std::vector<std::shared_ptr<ShaderInstruction>> insns;//instructions
	
	HSForkPhase()
		: fork_instance_count(0)
	{
	}
};

struct HSJoinPhase
{
	uint32_t join_instance_count;
	std::vector<std::shared_ptr<ShaderDecl>> dcls;
	std::vector<std::shared_ptr<ShaderInstruction>> insns;//instructions
	
	HSJoinPhase()
		: join_instance_count(0)
	{
	}
};

struct HSControlPointPhase
{
	std::vector<std::shared_ptr<ShaderDecl>> dcls;
	std::vector<std::shared_ptr<ShaderInstruction>> insns;//instructions
};

class GLSLGen final
{
public:
	static uint32_t DefaultRules(GLSLVersion version);

	void FeedDXBC(std::shared_ptr<ShaderProgram> const & program,
		bool has_gs, bool has_ps, ShaderTessellatorPartitioning ds_partitioning, ShaderTessellatorOutputPrimitive ds_output_primitive,
		GLSLVersion version, uint32_t glsl_rules);
	void ToGLSL(std::ostream& out);
	void ToHSControlPointPhase(std::ostream& out);
	void ToHSForkPhases(std::ostream& out);
	void ToHSJoinPhases(std::ostream& out);

private:
	void ToStructs(std::ostream& out);
	void ToType(std::ostream& out, DXBCShaderTypeDesc const& type_desc) const;
	void ToDeclarations(std::ostream& out);
	void ToDclInterShaderInputRecords(std::ostream& out);
	void ToDclInterShaderOutputRecords(std::ostream& out);
	void ToDclInterShaderPatchConstantRecords(std::ostream& out);
	void ToDeclInterShaderInputRegisters(std::ostream& out) const;
	void ToCopyToInterShaderInputRegisters(std::ostream& out) const;
	void ToDeclInterShaderOutputRegisters(std::ostream& out) const;
	void ToCopyToInterShaderOutputRecords(std::ostream& out) const;
	void ToDclInterShaderPatchConstantRegisters(std::ostream& out);
	void ToCopyToInterShaderPatchConstantRecords(std::ostream& out) const;
	void ToCopyToInterShaderPatchConstantRegisters(std::ostream& out) const;
	void ToDefaultHSControlPointPhase(std::ostream& out)const;
	void ToDeclaration(std::ostream& out, ShaderDecl const & dcl);
	void ToInstruction(std::ostream& out, ShaderInstruction const & insn) const;
	void ToOperands(std::ostream& out, ShaderOperand const & op, uint32_t imm_as_type,
		bool mask = true, bool dcl_array = false, bool no_swizzle = false, bool no_idx = false, bool no_cast = false,
		ShaderInputType const & sit = SIT_UNDEFINED) const;
	ShaderImmType OperandAsCBufferType(
		uint32_t imm_as_type, uint32_t offset, uint32_t var_start_offset, DXBCShaderTypeDesc const& var_type_desc) const;
	ShaderImmType OperandAsType(ShaderOperand const & op, uint32_t imm_as_type) const;
	int ToSingleComponentSelector(std::ostream& out, ShaderOperand const & op, int i, bool dot = true) const;
	void ToOperandName(std::ostream& out, ShaderOperand const& op, DXBCShaderTypeDesc const& type_desc, const char* var_name,
		uint32_t var_start_offset, std::vector<DXBCShaderVariable> const& cb_vars, uint32_t offset, bool contain_multi_var,
		bool dynamic_indexed, uint32_t register_index, uint32_t num_selectors, bool no_swizzle, bool& need_comps) const;
	void ToOperandName(std::ostream& out, ShaderOperand const & op, ShaderImmType as_type,
		bool* need_idx, bool* need_comps, bool no_swizzle = false, bool no_idx = false,
		ShaderInputType const & sit = SIT_UNDEFINED) const;
	void ToComponentSelectors(std::ostream& out, ShaderOperand const & op, bool dot = true, uint32_t offset = 0) const;
	void ToTemps(std::ostream& out, ShaderDecl const & dcl);
	void ToImmConstBuffer(std::ostream& out, ShaderDecl const & dcl);
	void ToDefaultValue(std::ostream& out, DXBCShaderVariable const & var);
	void ToDefaultValue(std::ostream& out, DXBCShaderVariable const & var, uint32_t offset);
	void ToDefaultValue(std::ostream& out, char const * value, ShaderVariableType type);
	uint32_t ComponentSelectorFromMask(uint32_t mask, uint32_t comps) const;
	uint32_t ComponentSelectorFromSwizzle(uint8_t const swizzle[4], uint32_t comps) const;
	uint32_t ComponentSelectorFromScalar(uint8_t scalar) const;
	uint32_t ComponentSelectorFromCount(uint32_t count) const;
	void ToComponentSelector(std::ostream& out, uint32_t comps, uint32_t offset = 0) const;
	bool IsImmediateNumber(ShaderOperand const & op) const;
	// param i:the component selector to get
	// return:the idx of selector:0 1 2 3 stand for x y z w
	// example: for .xw i=0 return 0 i=1 return 3
	int GetComponentSelector(ShaderOperand const & op, int i) const;
	int GetOperandComponentNum(ShaderOperand const & op) const;
	uint32_t GetMaxComponentSelector(ShaderOperand const & op) const;
	uint32_t GetMinComponentSelector(ShaderOperand const & op) const;
	ShaderRegisterComponentType GetOutputParamType(ShaderOperand const & op) const;
	DXBCSignatureParamDesc const & GetOutputParamDesc(ShaderOperand const & op, uint32_t index = 0) const;
	DXBCSignatureParamDesc const & GetInputParamDesc(ShaderOperand const & op, uint32_t index = 0) const;
	DXBCInputBindDesc const & GetResourceDesc(ShaderInputType type, uint32_t bind_point) const;
	DXBCConstantBuffer const & GetConstantBuffer(ShaderCBufferType type, char const * name) const;
	uint32_t GetNumPatchConstantSignatureRegisters(std::vector<DXBCSignatureParamDesc> const & params_patch)const;
	void FindDclIndexRange();
	void FindSamplers();
	void LinkCFInsns();
	void FindLabels();
	void FindEndOfProgram();
	void FindTempDcls();
	void FindHSControlPointPhase();
	void FindHSForkPhases();
	void FindHSJoinPhases();
	ShaderImmType FindTextureReturnType(ShaderOperand const & op) const;

private:
	std::shared_ptr<ShaderProgram> program_;

	ShaderType shader_type_;
	bool has_gs_;
	bool has_ps_;
	ShaderTessellatorPartitioning ds_partitioning_;
	ShaderTessellatorOutputPrimitive ds_output_primitive_;
	std::vector<DclIndexRangeInfo> idx_range_info_;
	std::vector<TextureSamplerInfo> textures_;
	std::vector<ShaderDecl> temp_dcls_;
	std::map<int64_t, bool> cb_index_mode_;
	std::vector<HSControlPointPhase> hs_control_point_phase_;
	std::vector<HSForkPhase> hs_fork_phases_;
	std::vector<HSJoinPhase> hs_join_phases_;
	bool enter_hs_fork_phase_;
	bool enter_final_hs_fork_phase_;
	bool enter_hs_join_phase_;
	bool enter_final_hs_join_phase_;

	// for ifs, the insn number of the else or endif if there is no else
	// for elses, the insn number of the endif
	// for endifs, the insn number of the if
	// for loops, the insn number of the endloop
	// for endloops, the insn number of the loop
	// for all others, -1
	std::vector<uint32_t> cf_insn_linked_;
	std::vector<LabelInfo> label_to_insn_num_;
	bool labels_found_;
	// the id of ret instruction which is not nested in any flow control statements
	uint32_t end_of_program_;

	GLSLVersion glsl_version_;
	uint32_t glsl_rules_;

	mutable std::vector<uint8_t> temp_as_type_;
};

#endif		// _DXBC2GLSL_GLSLGEN_HPP
