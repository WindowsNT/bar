#pragma once

class dir
{
public:

	class root
	{
	public:
		std::map<std::wstring,root> sub;
	};
	root r;

	void push(const wchar_t* p)
	{
		std::vector<wchar_t> pp(wcslen(p) + 2);
		memcpy(pp.data(),p, wcslen(p) * 2);
		wchar_t* px = pp.data();
		root* rr = &r;
		for (;;)
		{
			auto p2 = wcschr(px, '\\');
			if (!p2)
			{
				root r2;
				rr->sub[px] = r2;
				break;
			}
			*p2 = 0;
			root r2;
			if (rr->sub.find(px) == rr->sub.end())
				rr->sub[px] = r2;
			rr = &rr->sub[px];
			px = p2 + 1;
		}
	}
};

void ListInsertColumn(HWND hL, int i, UINT mask, int fmt, int cx, TCHAR* txt, int txtm, int sub, int iI, int iO)
{
	LV_COLUMN lc = { mask,fmt,cx,txt,txtm,sub,iI,iO };
	ListView_InsertColumn(hL, i, &lc);
}

void AutoSizeLVColumn(HWND hL, int j)
{
	SendMessage(hL, LVM_SETCOLUMNWIDTH, j, LVSCW_AUTOSIZE);
	int a1 = ListView_GetColumnWidth(hL, j);
	SendMessage(hL, LVM_SETCOLUMNWIDTH, j, LVSCW_AUTOSIZE_USEHEADER);
	int a2 = ListView_GetColumnWidth(hL, j);
	if (a1 > a2)
		SendMessage(hL, LVM_SETCOLUMNWIDTH, j, LVSCW_AUTOSIZE);
}


LRESULT CALLBACK Main_DP(HWND hh, UINT mm, WPARAM ww, LPARAM ll)
{
	using namespace std;
	HWND hL = GetDlgItem(hh, 901);
	switch (mm)
	{
	case WM_CREATE:
	{
		hL = CreateWindowEx(0, L"SysListView32", L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS, 0, 0, 1, 1, hh, (HMENU)901, hAppInstance, 0);
		int p = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER;
		SendMessage(hL, LVM_FIRST + 54, p, p);
		ListInsertColumn(hh, 0, LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, 0, 30, _T("#"), 0, 0, 0, 0);
		ListInsertColumn(hh, 1, LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, 0, 30, _T("Name"), 0, 0, 0, 0);
		
		SendMessage(hh, WM_SIZE, 0, 0);
		SendMessage(hh, WM_USER + 1, 0, 0);
		break;
	}
	case WM_SIZE:
	{
		RECT rc;
		GetClientRect(hh, &rc);
		SetWindowPos(hL, 0, 0, 0, rc.right, rc.bottom, SWP_SHOWWINDOW);
		return 0;
	}

	case WM_COMMAND:
	{
		int LW = LOWORD(ww);
		UNREFERENCED_PARAMETER(LW);


		return 0;
	}

	case WM_USER + 1:
	{
		MAPPER m(files[0].c_str(), PAGE_READONLY);
		const char* map = (const char*)m.Map();
		if (!map)
		{
			SendMessage(hh, WM_CLOSE, 0, 0);
			return 0;
		}

		auto fs = m.fs();

		list<TAGDATA> tags;
		vector<TAGDATA> vtags;
		if (!TagCollect(map, fs, tags, vtags))
		{
			SendMessage(hh, WM_CLOSE, 0, 0);
			return 0;
		}

		dir d;
		for (auto& t : tags)
		{
			TAG* tt = (TAG*)t.d.data();
			if (tt->code == (unsigned int)TAGCODE::HEADER)
			{
			}
			if (tt->code == (unsigned int)TAGCODE::ITEMMETA)
			{
				ITEMMETA* h = (ITEMMETA*)tt;
				char* pp = (char*)tt;
				vector<char> fn;
				fn.resize(h->tag.size - sizeof(ITEMMETA));
				memcpy(fn.data(), pp + sizeof(ITEMMETA), fn.size());
				ystring fnn = fn.data();

				d.push(fnn.c_str());

			}
		}

		return 0;
	}

	case WM_CLOSE:
	{
		DestroyWindow(hh);
		return 0;
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	}
	return DefWindowProc(hh, mm, ww, ll);
}



void WV()
{
	// View archive
	if (files.empty())
		return;

	FreeConsole();

	WNDCLASSEX wClass = { 0 };
	wClass.cbSize = sizeof(wClass);

	wClass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_PARENTDC;
	wClass.lpfnWndProc = (WNDPROC)Main_DP;
	wClass.hInstance = GetModuleHandle(0);
	wClass.hIcon = hIcon1;
	wClass.hCursor = LoadCursor(0, IDC_ARROW);
	wClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wClass.lpszClassName = _T("CLASS");
	wClass.hIconSm = hIcon1;
	RegisterClassEx(&wClass);

	auto MainWindow = CreateWindowEx(0,
		_T("CLASS"),
		L"Backup ARchiver",
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, GetModuleHandle(0), 0);

	ShowWindow(MainWindow, SW_SHOW);


	MSG msg;

	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}