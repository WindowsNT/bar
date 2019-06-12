#include "stdafx.h"
#include "files.h"
#include "rgf\\rgf1.hpp"

HINSTANCE hAppInstance = 0;
HICON hIcon1 = 0;
bool EndProcess = false;
unsigned int Dups = 0;
tlock<std::list<ystring>> Warnings;

#include "func.hpp"

#if __has_include("ds.hpp")
#define USE_ADES
#endif

std::string Cmd()
{
	using namespace std;
	using namespace TCLAP;
	TCLAP::CmdLine cmd("", ' ', "1.0", false);
	try
	{
		// Command
		vector <string> c1 = { "a","d","ds","e","ee","l","ll","m","s","t","tt","u","uu","w","z" };
		ValuesConstraint<string> con1(c1);
		UnlabeledValueArg<string> co1("command", "Specifies the command", true, "", &con1, false, 0);
		cmd.add(co1);

		// Switches
		SwitchArg r("r", "recurse", "Recurse Subdirectories", false); cmd.add(r);
		SwitchArg o("o", "overwrite", "Overwrite archive", false); cmd.add(o);
		SwitchArg s("s", "sigs", "Generate signatures", false); cmd.add(s);
		SwitchArg t("t", "test", "Test run", false); cmd.add(t);
		SwitchArg y("y", "yes", "Assume yes to queries", false); cmd.add(y);
		SwitchArg dups("", "dup", "Duplicate files", false); cmd.add(dups);

		// Multiple Arguments
		MultiArg<string> i("i", "include", "Include files", false, "string", 0); cmd.add(i);
		MultiArg<string> x("x", "exclude", "Exclude files", false, "string", 0); cmd.add(x);
		MultiArg<string> xr("", "xr", "Exclude files with regexp", false, "string", 0); cmd.add(xr);
		MultiArg<string> ir("", "ir", "Include files with regexp", false, "string", 0);	cmd.add(ir);
		MultiArg<string> incr("", "incr", "Include files incremental", false, "string", 0);	cmd.add(incr);
		MultiArg<string> ds("", "ds", "Digital Signature", false, "string", 0);	cmd.add(ds);

		// Pause
		ValueArg<string> pause("", "pause", "Pause after ending 0 (default) or 1 ", false, "0", "int", nullptr);	cmd.add(pause);

		// Threads
		char deft[100] = { 0 };
		sprintf_s(deft, 100, "%u", std::thread::hardware_concurrency()*2);
		if (IsDebuggerPresent()) sprintf_s(deft, 100, "0");
		ValueArg<string> threads("", "threads", "threads", false, deft, "int", nullptr);	cmd.add(threads);

		ValueArg<string> rgf("", "rgf", "rgf", false, "", "string", nullptr);	cmd.add(rgf);
		ValueArg<string> keep("", "keep", "keep", false, "", "int", nullptr);	cmd.add(keep);

		// Password
		ValueArg<string> pwd("p", "password", "password", false, "", "string", nullptr);	cmd.add(pwd);

		// Files
		UnlabeledMultiArg<string> multi("files", "file names", false, "string", true);
		cmd.add(multi);

		// Parse
		cmd.setExceptionHandling(false);
		vector<string> v;
		for (int ii = 0; ii < __argc; ii++)
			v.push_back(ystring(__wargv[ii]).a_str());
		cmd.parse(v);

		// Get Items
		command = co1.getValue();
		//		sw.SignatureFile = signaturefile.getValue().c_str();
		sw.R = r.getValue();
		sw.ds = ds.getValue();
		sw.O = o.getValue();
		sw.Y = y.getValue();
		sw.T = t.getValue();
		//		sw.CompareMode = atoi(comparemode.getValue().c_str());
		sw.pwd = pwd.getValue();
		sw.rgf = rgf.getValue();
		sw.keep = keep.getValue();
		sw.dup = dups.getValue();
		sw.threads = atoi(threads.getValue().c_str());
		sw.pause = atoi(pause.getValue().c_str());
		sw.sigs = s.getValue();
		for (auto& a : incr.getValue()) sw.incr.push_back(ystring(a));
		for (auto& a : i.getValue()) sw.I.push_back(ystring(a).a_str());
		for (auto& a : x.getValue()) sw.X.push_back(ystring(a).a_str());
		for (auto& a : xr.getValue()) sw.XR.push_back(ystring(a));
		for (auto& a : ir.getValue()) sw.IR.push_back(ystring(a));
		for (auto& a : multi.getValue()) files.push_back(ystring(a.c_str()));
	}
	catch (TCLAP::ArgException & e)
	{
		return e.what();
	}
	return "";
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	EndProcess = 1;
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
		printf("Ctrl-C event\n\n");
		Beep(750, 300);
		return(TRUE);

		// CTRL-CLOSE: confirm that the user wants to exit. 
	case CTRL_CLOSE_EVENT:
		Beep(600, 200);
		printf("Ctrl-Close event\n\n");
		return(TRUE);

		// Pass other signals to the next handler. 
	case CTRL_BREAK_EVENT:
		Beep(900, 200);
		printf("Ctrl-Break event\n\n");
		return TRUE;

	case CTRL_LOGOFF_EVENT:
		Beep(1000, 200);
		printf("Ctrl-Logoff event\n\n");
		return TRUE;

	case CTRL_SHUTDOWN_EVENT:
		Beep(750, 500);
		printf("Ctrl-Shutdown event\n\n");
		return TRUE;

	default:
		return TRUE;
	}
}

void Help(const char* err = 0,const char* cmd = 0)
{

	if (cmd == 0)
	{
		std::cout << R"(
Backup Archiver commands:
     a  - Create new archive or update archive
     d  - Delete files
     ds - Add digital signature
     e  - Extract archive
     ee - Extract from an archive folder (main archive and differential updates)
     l  - List archive contents
     ll - List archive contents (csv)
     m  - Merge archives
     s  - Generate signature for archive
     t  - Test archive
     tt - Test from an archive folder (main archive and differential updates)
     u  - Differentially update archive
     uu - Differentially update archive (backup to folder)
     z  - Separate archive to signatures only

Backup Archiver Switches:

	-i <mask>	- Include files
	-p <pwd>	- Set password
	-r		- Recurse subdirectories
	-o		- Overwrite archive if it exists before adding files
	-s		- Generate signatures
	-t		- Test run
	-y		- Yes to all questions
	-x		- Exclude files
	--ir		- Include files with regexp
	--xr		- Exclude files with regexp
	--rgf <f>	- Specify file for OneDrive/GoogleDrive/DropBox
	--keep <dir>	- Keep an archive uploaded with uu to a local folder

	 
)";
		return;
	}

	if (strcmp(cmd,"a") == 0)
	{

	}
}


#ifdef USE_ADES
#include "ds.hpp"
#endif
#include "archive2.hpp"
#include "sign.hpp"
#include "list.hpp"
#include "delete.hpp"
#include "separate.hpp"
#include "update.hpp"
#include "extract.hpp"
#include "merge.hpp"
#include "window.hpp"

// a r:\test.bar -r -x *\.git\* -x *.tlog --crcs 1 -s f:\tp2\pr
// e r:\test.bar r:\bb
// u r:\test.bar -r -x *\.git\* -x *.tlog --crcs 1 f:\tp2\pr

int wmain()
{

	WSADATA wData;
	WSAStartup(MAKEWORD(2, 2), &wData);
	INITCOMMONCONTROLSEX icex = { 0 };
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_DATE_CLASSES | ICC_WIN95_CLASSES;
	icex.dwSize = sizeof(icex);
	InitCommonControlsEx(&icex);
	InitCommonControls();
	RGF::AXLIBRARY::AXRegister();


	// Browser Emulation
	std::vector<wchar_t> fn(1000);
	GetModuleFileName(0, fn.data(), 1000);
	PathStripPath(fn.data());
	RGF::RKEY k(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
	k[fn.data()] = 11001UL;

	using namespace std;
	CoInitializeEx(0, COINIT_MULTITHREADED);
	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
	hAppInstance = GetModuleHandle(0);
	hIcon1 = LoadIcon(hAppInstance, _T("ICON_1"));
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
	PrepareDoMatchTable();

	if (__argc == 2 && GetFileAttributes(__wargv[1]) != 0xFFFFFFFF)
	{
		command = "w";
		files.push_back(__wargv[1]);
	}
	else
	{
		auto ss = Cmd();
		if (ss.length())
			Help(ss.c_str());
	}

	if (command == L"w")
		WV();

	if (command != L"ll")
		std::cout << "Backup ARchiver " << BAR_MAJ_VERSION << "." << BAR_MIN_VERSION << ", Copyright (C) Chourdakis Michael" << std::endl;

	if (command == L"a")
		Archive2();

	if (command == L"s")
		Sign();

	if (command == L"e")
		Extract();

	if (command == L"m")
		Merge();

	if (command == L"ee")
		Extract(0,1);

	if (command == L"l")
		List();

	if (command == L"ll")
		List(1);

	if (command == L"t")
		Extract(1);

	if (command == L"tt")
		Extract(1, 1);

	if (command == L"u")
		Update();

	if (command == L"uu")
		Update(1);

	if (command == L"d")
		Delete();

	if (command == L"z")
		Separate();

#ifdef USE_ADES
	if (command == L"ds")
		ds();
#endif

	Warnings.readlock([](const list<ystring>& w)
		{
			for (auto& rr : w)
				wcout << rr << endl;
			if (!w.empty())
				wprintf(L"Number of warnings: %zu\r\n", w.size());
		});

	if (Dups)
		wprintf(L"Number of duplicates: %u\r\n", Dups);

	k[fn.data()].Delete();
	return 0;
}