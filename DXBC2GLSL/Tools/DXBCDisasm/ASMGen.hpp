#ifndef _DXBC2GLSL_ASMGEN_HPP
#define _DXBC2GLSL_ASMGEN_HPP

#pragma once

#include <DXBC2GLSL/Shader.hpp>

class ASMGen
{
public:
	explicit ASMGen(boost::shared_ptr<ShaderProgram> const & program);

	void ToASM(std::ostream& out);

private:
	void Disasm(std::ostream& out, ShaderOperand const & op, ShaderImmType imm_type);
	void Disasm(std::ostream& out, ShaderInstruction const & op);
	void Disasm(std::ostream& out, ShaderDecl const & op);
	void Disasm(std::ostream& out, std::vector<DXBCSignatureParamDesc> const & signature, uint32_t fourcc);
	void Disasm(std::ostream& out, std::vector<DXBCConstantBuffer> const & cb);
	void Disasm(std::ostream& out, std::vector<DXBCInputBindDesc> const & bindings);
	// Disasm cb default value
	void Disasm(std::ostream& out, DXBCShaderVariable const & var);
	void Disasm(std::ostream& out, DXBCShaderVariable const & var, uint32_t offset);
	void Disasm(std::ostream& out, const char* value, ShaderVariableType type);

private:
	boost::shared_ptr<ShaderProgram> program_;
};

#endif		// _DXBC2GLSL_ASMGEN_HPP
