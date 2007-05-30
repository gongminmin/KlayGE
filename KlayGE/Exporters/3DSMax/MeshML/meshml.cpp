// meshml.cpp
// KlayGE meshml导出类 实现文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.5.0
// 初次建立 (2005.5.1)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <max.h>
#include <Interval.h>

#include <vector>
#include <sstream>

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
		return TEXT("Copyright 2005");
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
		::MessageBox(hWnd, TEXT("Minmin Gong, Copyright 2005"), TEXT("About"), MB_OK);
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

	int meshml_export::DoExport(TCHAR const * name, ExpInterface* exp_interface, Interface* max_interface,
							BOOL suppress_prompts, DWORD options)
	{
		file_name_ = name;

		max_interface_ = max_interface;
		HWND max_wnd = max_interface->GetMAXHWnd();
		if (::DialogBoxParam(dll_instance, MAKEINTRESOURCE(IDD_MESHML_EXPORT), max_wnd,
			export_wnd_proc, reinterpret_cast<LPARAM>(this)))
		{
			MessageBox(max_wnd, TEXT("导出成功！"), TEXT("MeshML Export"), MB_OK);
		}

		return 1;
	}

	void meshml_export::refresh_node_list(HWND dlg_wnd)
	{
		::SendMessage(::GetDlgItem(dlg_wnd, IDC_NODE_LIST), LB_RESETCONTENT, NULL, NULL);
		INode* root_node = max_interface_->GetRootNode();
		int const num_children = root_node->NumberOfChildren();
		for (int i = 0; i < num_children; ++ i)
		{
			this->enum_node(dlg_wnd, root_node->GetChildNode(i));
		}
	}

	void meshml_export::enum_node(HWND dlg_wnd, INode* node)
	{
		if (!node->IsNodeHidden()
			|| (BST_CHECKED == SendMessage(GetDlgItem(dlg_wnd, IDC_HIDDEN), BM_GETCHECK, NULL, NULL)))
		{
			HWND node_list_wnd = ::GetDlgItem(dlg_wnd, IDC_NODE_LIST);
			int i = ::SendMessage(node_list_wnd, LB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(node->GetName()));
			::SendMessage(node_list_wnd, LB_SETITEMDATA, i, reinterpret_cast<LPARAM>(node));
		}

		int num_children = node->NumberOfChildren();
		for (int i = 0; i < num_children; ++ i)
		{
			this->enum_node(dlg_wnd, node->GetChildNode(i));
		}
	}

	INT_PTR WINAPI meshml_export::export_wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (WM_INITDIALOG == msg)
		{
			meshml_export* instance = reinterpret_cast<meshml_export*>(lparam);
			assert(instance != NULL);

			::SetWindowLongPtr(wnd, GWL_USERDATA, reinterpret_cast<LONG_PTR>(instance));

			instance->dlg_wnd_ = wnd;

			::SendMessage(::GetDlgItem(wnd, IDC_HIDDEN), BM_SETCHECK, BST_UNCHECKED, NULL);

			instance->refresh_node_list(wnd);
		}
		else
		{
			meshml_export* instance = reinterpret_cast<meshml_export*>(::GetWindowLongPtr(wnd, GWL_USERDATA));

			switch (msg)
			{
			case WM_COMMAND:
				switch (LOWORD(wparam))
				{
				case IDC_SEL_ALL:
					::SendMessage(::GetDlgItem(wnd, IDC_NODE_LIST), LB_SETSEL, TRUE, -1);
					break;

				case IDC_HIDDEN:
					assert(instance != NULL);
					instance->refresh_node_list(wnd);
					break;

				case IDOK:
					{
						assert(instance != NULL);

						HWND node_list_wnd = ::GetDlgItem(wnd, IDC_NODE_LIST);
						int num_sel = ::SendMessage(node_list_wnd, LB_GETSELCOUNT, NULL, NULL);
						if (num_sel > 0)
						{
							int const joint_per_ver = ::SendMessage(::GetDlgItem(wnd, IDC_SPIN_JOINT_PER_VER), UDM_GETPOS32, NULL, NULL);
							Interval const se_ticks = instance->max_interface_->GetAnimRange();
							TimeValue const cur_time = instance->max_interface_->GetTime();
							meshml_extractor extractor(instance->max_interface_, joint_per_ver, cur_time,
								se_ticks.Start() / GetTicksPerFrame(), se_ticks.End() / GetTicksPerFrame());

							std::vector<int> sel_items(num_sel);
							::SendMessage(node_list_wnd, LB_GETSELITEMS, num_sel, reinterpret_cast<LPARAM>(&sel_items[0]));

							std::vector<INode*> nodes;
							for (size_t i = 0; i < sel_items.size(); ++ i)
							{
								nodes.push_back(reinterpret_cast<INode*>(::SendMessage(node_list_wnd,
									LB_GETITEMDATA, sel_items[i], NULL)));
							}

							extractor.export_objects(nodes);
							extractor.write_xml(instance->file_name_);
						}
						else
						{
							::MessageBox(wnd, TEXT("Please select something."), TEXT("MeshML Export"), MB_OK);
						}

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
