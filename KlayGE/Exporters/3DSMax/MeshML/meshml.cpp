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
		return TEXT("Copyright 2005-2011");
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
		::MessageBox(hWnd, TEXT("Minmin Gong, Copyright 2005-2011"), TEXT("About"), MB_OK);
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

		HWND max_wnd = max_interface->GetMAXHWnd();
		if (::DialogBoxParam(dll_instance, MAKEINTRESOURCE(IDD_MESHML_EXPORT), max_wnd,
			export_wnd_proc, reinterpret_cast<LPARAM>(this)))
		{
			MessageBox(max_wnd, TEXT("导出成功！"), TEXT("MeshML Export"), MB_OK);
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

			int const normal_index = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Normal"))));
			int const tangent_index = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Tangent"))));
			int const texcoord_index = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Texture coordinate"))));

			int const binormal_index = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_IGNORED_VERTEX_ATTRS), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Binormal"))));

			::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_SETITEMDATA, normal_index, static_cast<LPARAM>(VEU_Normal));
			::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_SETITEMDATA, tangent_index, static_cast<LPARAM>(VEU_Tangent));
			::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_SETITEMDATA, texcoord_index, static_cast<LPARAM>(VEU_TextureCoord));

			::SendMessage(::GetDlgItem(wnd, IDC_LIST_IGNORED_VERTEX_ATTRS), LB_SETITEMDATA, binormal_index, static_cast<LPARAM>(VEU_Binormal));

			::SendMessage(::GetDlgItem(wnd, IDC_COMBINE_MESHES), BM_SETCHECK, BST_CHECKED, NULL);
		}
		else
		{
			meshml_export* instance = reinterpret_cast<meshml_export*>(::GetWindowLongPtr(wnd, GWLP_USERDATA));

			switch (msg)
			{
			case WM_COMMAND:
				switch (LOWORD(wparam))
				{
				case IDC_BUTTON_ADD:
					{
						int const count = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_IGNORED_VERTEX_ATTRS), LB_GETSELCOUNT, 0, 0));
						if (count > 0)
						{
							std::vector<int> sel_items(count);
							::SendMessage(::GetDlgItem(wnd, IDC_LIST_IGNORED_VERTEX_ATTRS), LB_GETSELITEMS, count, reinterpret_cast<LPARAM>(&sel_items[0]));

							for (size_t i = 0; i < sel_items.size(); ++ i)
							{
								int len = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_IGNORED_VERTEX_ATTRS), LB_GETTEXTLEN, sel_items[i], NULL));
								std::vector<TCHAR> text(len + 1);
								::SendMessage(::GetDlgItem(wnd, IDC_LIST_IGNORED_VERTEX_ATTRS), LB_GETTEXT, sel_items[i], reinterpret_cast<LPARAM>(&text[0]));
								int data = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_IGNORED_VERTEX_ATTRS), LB_GETITEMDATA, sel_items[i], NULL));

								int const ind = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(&text[0])));
								::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_SETITEMDATA, ind, static_cast<LPARAM>(data));

								::SendMessage(::GetDlgItem(wnd, IDC_LIST_IGNORED_VERTEX_ATTRS), LB_DELETESTRING, sel_items[i], 0);
							}
						}
					}
					break;

				case IDC_BUTTON_DEL:
					{
						int const count = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_GETSELCOUNT, 0, 0));
						if (count > 0)
						{
							std::vector<int> sel_items(count);
							::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_GETSELITEMS, count, reinterpret_cast<LPARAM>(&sel_items[0]));

							for (size_t i = 0; i < sel_items.size(); ++ i)
							{
								int len = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_GETTEXTLEN, sel_items[i], NULL));
								std::vector<TCHAR> text(len + 1);
								::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_GETTEXT, sel_items[i], reinterpret_cast<LPARAM>(&text[0]));
								int data = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_GETITEMDATA, sel_items[i], NULL));

								int const ind = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_IGNORED_VERTEX_ATTRS), LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(&text[0])));
								::SendMessage(::GetDlgItem(wnd, IDC_LIST_IGNORED_VERTEX_ATTRS), LB_SETITEMDATA, ind, static_cast<LPARAM>(data));

								::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_DELETESTRING, sel_items[i], 0);
							}
						}
					}
					break;

				case IDOK:
					{
						assert(instance != NULL);

						int const joint_per_ver = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_SPIN_JOINT_PER_VER), UDM_GETPOS32, NULL, NULL));
						bool const combine_meshes = (BST_CHECKED == ::SendMessage(::GetDlgItem(wnd, IDC_COMBINE_MESHES), BM_GETCHECK, NULL, NULL));
						Interval const se_ticks = instance->max_interface_->GetAnimRange();
						meshml_extractor extractor(instance->max_interface_->GetRootNode(), joint_per_ver,
							instance->max_interface_->GetTime(),
							se_ticks.Start() / GetTicksPerFrame(), se_ticks.End() / GetTicksPerFrame(),
							combine_meshes);

						export_vertex_attrs eva;
						eva.normal = false;
						eva.tangent = false;
						eva.binormal = false;
						eva.tex = false;
						for (int index = 0;; ++ index)
						{
							int data = static_cast<int>(::SendMessage(::GetDlgItem(wnd, IDC_LIST_EXPORT_VERTEX_ATTRS), LB_GETITEMDATA, index, NULL));
							if (data == LB_ERR)
							{
								break;
							}

							switch (data)
							{
							case VEU_Normal:
								eva.normal = true;
								break;

							case VEU_Tangent:
								eva.tangent = true;
								break;

							case VEU_Binormal:
								eva.binormal = true;
								break;

							case VEU_TextureCoord:
								eva.tex = true;
								break;
							}
						}

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
