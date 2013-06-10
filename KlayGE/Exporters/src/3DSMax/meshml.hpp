// meshml.hpp
// KlayGE meshml导出类 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://www.klayge.org
//
// 2.5.0
// 初次建立 (2005.5.1)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _MESHML_HPP
#define _MESHML_HPP

#include <string>
#include <vector>

namespace KlayGE
{
	class meshml_export : public SceneExport
	{
	public:
		meshml_export();
		~meshml_export();

		// SceneExport methods
		int ExtCount();
		TCHAR const * Ext(int n);     
		TCHAR const * LongDesc();
		TCHAR const * ShortDesc();
		TCHAR const * AuthorName(); 
		TCHAR const * CopyrightMessage(); 
		TCHAR const * OtherMessage1(); 
		TCHAR const * OtherMessage2(); 
		unsigned int Version();
		void ShowAbout(HWND wnd);
		int DoExport(TCHAR const * name, ExpInterface* exp_interface, Interface* max_interface, BOOL suppress_prompts, DWORD options);
		BOOL SupportsOptions(int ext, DWORD options);

	private:
		static INT_PTR WINAPI export_wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

		void enum_node(INode* node);

	private:
		std::string file_name_;
		Interface* max_interface_;
		std::vector<INode*> export_nodes_;

		bool in_chs_;
	};

	class meshml_class_desc : public ClassDesc
	{
	public:
		int IsPublic()
		{
			return TRUE;
		}

		void* Create(BOOL /*loading*/)
		{
			return new meshml_export;
		}

		TCHAR const * ClassName()
		{
			return _T("MeshML Export");
		}
		SClass_ID SuperClassID()
		{
			return SCENE_EXPORT_CLASS_ID;
		}
		Class_ID ClassID()
		{
			return Class_ID(0x509c6516, 0x128d15a7);
		}
		TCHAR const * Category()
		{
			return _T("");
		}
	};
}

#endif		// _MESHML_HPP
