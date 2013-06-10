// meshml.cpp
// KlayGE meshml导出类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2005-2007
// Homepage: http://www.klayge.org
//
// 3.6.0
// 简化了对话框 (2007.6.2)
//
// 2.5.0
// 初次建立 (2005.5.1)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable: 4100 4238 4239 4244 4245 4512)
#include <max.h>
#include <Interval.h>
#pragma warning(pop)

#include <vector>
#include <sstream>

#ifdef PI
#undef PI
#endif
#include <KFL/KFL.hpp>

#include "util.hpp"
#include "export_main.hpp"
#include "mesh_extractor.hpp"
#include "meshml.hpp"

namespace KlayGE
{
	meshml_export::meshml_export()
	{
	}

	meshml_export::~meshml_export()
	{
	}

	int meshml_export::ExtCount()
	{
		return 1;
	}

	TCHAR const * meshml_export::Ext(int n)
	{
		switch (n)
		{
		case 0:
			return TEXT("meshml");

		default:
			return NULL;
		}
	}

	TCHAR const * meshml_export::LongDesc()
	{
		return TEXT("Mesh XML File");
	}

	TCHAR const * meshml_export::ShortDesc()
	{
		return TEXT("MeshML File");
	}

	TCHAR const * meshml_export::AuthorName()
	{
		return TEXT("Minmin Gong");
	}

	TCHAR const * meshml_export::CopyrightMessage()
	{
		return TEXT("Copyright 2005-2013");
	}

	TCHAR const * meshml_export::OtherMessage1()
	{
		return TEXT("");
	}

	TCHAR const * meshml_export::OtherMessage2()
	{
		return TEXT("");
	}

	unsigned int meshml_export::Version()
	{
		return 010;
	}

	void meshml_export::ShowAbout(HWND hWnd)
	{
		::MessageBox(hWnd, TEXT("Minmin Gong, Copyright 2005-2013"), TEXT("About"), MB_OK);
	}

	BOOL meshml_export::SupportsOptions(int ext, DWORD options)
	{
		if ((options & SCENE_EXPORT_SELECTED) && (0 == ext))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	int meshml_export::DoExport(TCHAR const * name, ExpInterface* /*exp_interface*/, Interface* max_interface,
							BOOL /*suppress_prompts*/, DWORD options)
	{
		file_name_ = tstr_to_str(name);

		max_interface_ = max_interface;

		export_nodes_.clear();
		if (options & SCENE_EXPORT_SELECTED)
		{
			int count = max_interface_->GetSelNodeCount();
			for (int i = 0; i < count; ++ i)
			{
				this->enum_node(max_interface_->GetSelNode(i));
			}
		}
		else
		{
			this->enum_node(this->max_interface_->GetRootNode());
		}

		DWORD dlg_id;
		WORD lang_id = MaxSDK::Util::GetLanguageID();
		if ((LANG_CHINESE == PRIMARYLANGID(lang_id)) && (SUBLANG_CHINESE_SIMPLIFIED == SUBLANGID(lang_id)))
		{
			dlg_id = IDD_MESHML_EXPORT_CHS;
			in_chs_ = true;
		}
		else
		{
			dlg_id = IDD_MESHML_EXPORT_EN;
			in_chs_ = false;
		}

		HWND max_wnd = max_interface->GetMAXHWnd();
		if (::DialogBoxParam(dll_instance, MAKEINTRESOURCE(dlg_id), max_wnd,
			export_wnd_proc, reinterpret_cast<LPARAM>(this)))
		{
			if (in_chs_)
			{
				::MessageBox(max_wnd, TEXT("导出成功!"), TEXT("MeshML导出插件"), MB_OK);
			}
			else
			{
				::MessageBox(max_wnd, TEXT("Export Successful!"), TEXT("MeshML Export"), MB_OK);
			}
		}

		return 1;
	}

	void meshml_export::enum_node(INode* node)
	{
		if (!node->IsNodeHidden())
		{
			export_nodes_.push_back(node);
		}

		int num_children = node->NumberOfChildren();
		for (int i = 0; i < num_children; ++ i)
		{
			this->enum_node(node->GetChildNode(i));
		}
	}

	INT_PTR WINAPI meshml_export::export_wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (WM_INITDIALOG == msg)
		{
			meshml_export* instance = reinterpret_cast<meshml_export*>(lparam);
			assert(instance != NULL);

			::SetWindowLongPtr(wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(instance));

			::SendMessage(::GetDlgItem(wnd, IDC_TANGENT_COMBO), CB_ADDSTRING, 0, 
				instance->in_chs_ ? reinterpret_cast<LPARAM>(TEXT("正切四元数")) : reinterpret_cast<LPARAM>(TEXT("Tangent quaternion")));
			::SendMessage(::GetDlgItem(wnd, IDC_TANGENT_COMBO), CB_ADDSTRING, 0,
				instance->in_chs_ ? reinterpret_cast<LPARAM>(TEXT("法线")) : reinterpret_cast<LPARAM>(TEXT("Normal")));
			::SendMessage(::GetDlgItem(wnd, IDC_TANGENT_COMBO), CB_ADDSTRING, 0,
				instance->in_chs_ ? reinterpret_cast<LPARAM>(TEXT("无")) : reinterpret_cast<LPARAM>(TEXT("None")));
			::SendMessage(::GetDlgItem(wnd, IDC_TANGENT_COMBO), CB_SETCURSEL, 0, 0);
			::SendMessage(::GetDlgItem(wnd, IDC_TEXCOORD_CHECK), BM_SETCHECK, BST_CHECKED, NULL);
			::SendMessage(::GetDlgItem(wnd, IDC_COMBINE_MESHES_CHECK), BM_SETCHECK, BST_UNCHECKED, NULL);
		}
		else
		{
			meshml_export* instance = reinterpret_cast<meshml_export*>(::GetWindowLongPtr(wnd, GWLP_USERDATA));

			switch (msg)
			{
			case WM_COMMAND:
				switch (LOWORD(wparam))
				{
				case IDOK:
					{
						assert(instance != NULL);

						int const joint_per_ver = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_SPIN_JOINT_PER_VER), UDM_GETPOS32, NULL, NULL));
						bool const combine_meshes = (BST_CHECKED == ::SendMessage(::GetDlgItem(wnd, IDC_COMBINE_MESHES_CHECK), BM_GETCHECK, NULL, NULL));
						Interval const se_ticks = instance->max_interface_->GetAnimRange();
						meshml_extractor extractor(instance->max_interface_->GetRootNode(), joint_per_ver,
							instance->max_interface_->GetTime(),
							se_ticks.Start() / GetTicksPerFrame(), se_ticks.End() / GetTicksPerFrame(),
							combine_meshes);

						export_vertex_attrs eva;
						eva.tangent_quat = (0 == ::SendMessage(::GetDlgItem(wnd, IDC_TANGENT_COMBO), CB_GETCURSEL, NULL, NULL));
						eva.normal = (1 == ::SendMessage(::GetDlgItem(wnd, IDC_TANGENT_COMBO), CB_GETCURSEL, NULL, NULL));
						eva.tex = (BST_CHECKED == ::SendMessage(::GetDlgItem(wnd, IDC_TEXCOORD_CHECK), BM_GETCHECK, NULL, NULL));

						extractor.export_objects(instance->export_nodes_);
						extractor.write_xml(instance->file_name_, eva);

						::EndDialog(wnd, 1);
					}
					break;

				case IDCANCEL:
					::EndDialog(wnd, 0);
					break;
				}
				break;

			case WM_CLOSE:
				::EndDialog(wnd, 0);
				break;

			default:
				return FALSE;
			}
		}

		return TRUE;
	}
}
