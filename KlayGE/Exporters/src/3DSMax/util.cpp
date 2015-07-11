// util.cpp
// KlayGE ʵ�ú��� ʵ���ļ�
// Ver 2.5.0
// ��Ȩ����(C) ������, 2005
// Homepage: http://www.klayge.org
//
// 2.5.0
// ���ν��� (2005.5.1)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable: 4100) // Many unreferended parameters.
#pragma warning(disable: 4238) // Rvalue used as lvalue.
#pragma warning(disable: 4239) // Default argument with type conversion.
#pragma warning(disable: 4244) // Many conversion from int to WORD.
#pragma warning(disable: 4245) // Signed/unsigned conversion.
#pragma warning(disable: 4458) // Declaration of '...' hides class member.
#pragma warning(disable: 4459) // Declaration of '...' hides global declaration.
#pragma warning(disable: 4512) // BitArray::NumberSetProxy and DelayedNodeMat don't have assignment operator.
#include <max.h>
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable: 4100) // Many unreferended parameters.
#if VERSION_3DSMAX >= 7 << 16
#include <CS/bipexp.h>
#else
#include <bipexp.h>
#endif
#pragma warning(pop)

#ifdef PI
#undef PI
#endif
#include <KFL/KFL.hpp>

#include "util.hpp"

namespace KlayGE
{
	std::string tstr_to_str(std::basic_string<TCHAR> const & tstr)
	{
		std::string ret;
		Convert(ret, tstr);
		return ret;
	}

	bool is_mesh(INode* node)
	{
		if (NULL == node)
		{
			return false;
		}

		ObjectState os = node->EvalWorldState(0);
		if (NULL == os.obj)
		{
			return false;
		}

		if (os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID)
		{
			if (os.obj->ClassID() != Class_ID(TARGET_CLASS_ID, 0))
			{
				return true;
			}
		}
		
		return false;
	}

	bool is_bone(INode* node)
	{
		if (NULL == node)
		{
			return false;
		}

		ObjectState os = node->EvalWorldState(0);
		if (NULL == os.obj)
		{
			return false;
		}

		if (os.obj->SuperClassID() == HELPER_CLASS_ID)
		{
			return true;
		}

		if (os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID)
		{
			if (os.obj->ClassID() == BONE_OBJ_CLASSID)
			{
				return true;
			}
		}

		Control* ctl = node->GetTMController();
		if ((ctl->ClassID() == BIPSLAVE_CONTROL_CLASS_ID)
			|| (ctl->ClassID() == BIPBODY_CONTROL_CLASS_ID))
		{
			return true;
		}

		return false;
	}
}
