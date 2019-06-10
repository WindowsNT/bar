#pragma once


void List(bool ext = false)
{
	using namespace std;
	unsigned long nf = 0;
	if (files.size() < 1)
	{
		Help(0, "l");
		return;
	}

	CONSOLECOLOR cc;
	MAPPER m(files[0].c_str(), PAGE_READONLY);
	const char* map = (const char*)m.Map();
	if (!map)
	{
		cc.Color(FOREGROUND_RED | FOREGROUND_INTENSITY);
		wcout << L"Cannot open " << files[0].c_str() << endl;
		return;
	}

	auto fs = m.fs();
	cc.Color(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	if (!ext)
		wcout << L"Reading " << files[0].c_str() << endl;

	list<TAGDATA> tags;
	vector<TAGDATA> vtags;
	if (!TagCollect(map, fs, tags,vtags))
	{
		cc.Color(FOREGROUND_RED | FOREGROUND_INTENSITY);
		wcout << L"Not a BAR file" << endl;
		return;
	}

	cc.r();
	for (auto& t : tags)
	{
		if (EndProcess)
			break;
		TAG* tt = (TAG*)t.d.data();
		if (tt->code == (unsigned int)TAGCODE::HEADER)
		{
			HEADER* h = (HEADER*)tt;
			if (!ext)
			{
				wprintf(L"BAR file created with BAR %u.%u\r\n", h->major, h->minor);
				wprintf(L"ID                   Size      CmpSize       Sig       CRC         Alg    Name\r\n");
				wprintf(L"------------------------------------------------------------------------------------------------------------------------\r\n");
			}
		}
		if (tt->code == (unsigned int)TAGCODE::ITEMMETA)
		{
			cc.Color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			ITEMMETA* h = (ITEMMETA*)tt;
			char* pp = (char*)tt;
			vector<char> fn;
			fn.resize(h->tag.size - sizeof(ITEMMETA));
			memcpy(fn.data(), pp + sizeof(ITEMMETA),fn.size());
			ystring fnn = fn.data();


			// Include ?
			bool id = IncludeItem(fnn.a_str(), false);
			if (!id)
				continue;


			int dt = 0;
			unsigned long long sigsize = 0;
			wstring fd = L"FILE";
			for (auto& tt : t.data)
			{
				if (tt.type == (unsigned int)DATATYPE::DATA)
				{
					dt = tt.ctype;
					if (tt.etype == 1)
						fd = L"FIL*";
				}
				if (tt.type == (unsigned int)DATATYPE::DIFF)
				{
					fd = L"DIFF";
				}
				if (tt.type == (unsigned int)DATATYPE::SIG)
				{
					sigsize = tt.s;
				}
			}


			if (ext)
			{
				auto extx = PathFindExtension(fnn.c_str());

				wprintf(L"%s\t%u\t%llu\t%llu\t%s\t%s\r\n",fd.c_str(),h->fid,h->uncsize,h->cmpsize,fnn.c_str(),extx);
			}
			else
			{
				if (h->unccrc)
					wprintf(L"%s %7u    %s    %s    %s    %08X    %02u     %s\r\n", fd.c_str(), h->fid, DispSize(h->uncsize).c_str(), DispSize(h->cmpsize).c_str(), DispSize(sigsize).c_str(), h->unccrc, dt, fnn.c_str());
				else
					wprintf(L"%s %7u    %s    %s    %s                %02u     %s\r\n", fd.c_str(), h->fid, DispSize(h->uncsize).c_str(), DispSize(h->cmpsize).c_str(), DispSize(sigsize).c_str(), dt, fnn.c_str());
			}
			nf++;
		}
	}

	wprintf(L"Total files %u", nf);
	
}