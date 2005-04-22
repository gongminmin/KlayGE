#ifndef _MESHML_HPP
#define _MESHML_HPP

#include <string>

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

	void refresh_node_list(HWND wnd);
	void enum_node(HWND wnd, INode* node);

private:
	std::basic_string<TCHAR> file_name_;
	HWND dlg_wnd_;

	Interface* max_interface_;
};

class MeshML_ClassDesc : public ClassDesc
{
public:
	int IsPublic()
	{
		return TRUE;
	}

	void* Create(BOOL loading)
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

#endif		// _MESHML_HPP
