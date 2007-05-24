// IPassword.hpp
// KlayGE 打包系统密码访问接口 头文件 来自7zip
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.5.24)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _IPASSWORD_HPP
#define _IPASSWORD_HPP

#include "BaseDefines.hpp"

// MIDL_INTERFACE("23170F69-40C1-278A-0000-000500xx0000")
DEFINE_GUID(IID_ICryptoGetTextPassword,
	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x05, 0x00, 0x10, 0x00, 0x00);
struct ICryptoGetTextPassword : public IUnknown
{
	STDMETHOD(CryptoGetTextPassword)(BSTR* password) PURE;
};

#endif		// _IPASSWORD_HPP

