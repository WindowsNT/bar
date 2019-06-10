#pragma once


HRESULT Merge()
{
	using namespace std;
	if (files.size() < 2)
	{
		Help(0, "m");
		return E_INVALIDARG;
	}

	TEMPFILE t;
	CONSOLECOLOR cc;
	try
	{

		// Write header
		HEADER t1;
		auto hrw = t.Write((char*)& t1, sizeof(t1));
		FailWrite(hrw);

		vector<tuple<MAPPER,const char*,list<TAGDATA>,vector<TAGDATA>>> f;
		for (size_t i = 1; i < files.size(); i++)
		{
			auto& fi = files[i];
			MAPPER m(fi.c_str(),PAGE_READONLY);
			const char* mx = (const char*)m.Map();
			if (!mx)
				throw;
			list<TAGDATA> tags;
			vector<TAGDATA> vtags;
			wprintf(L"Reading archive %s...\r\n", fi.c_str());
			if (!TagCollect(mx, m.fs(), tags, vtags))
				throw;

			f.push_back(make_tuple<>(forward<MAPPER>(m),mx,forward<list<TAGDATA>>(tags),forward<vector<TAGDATA>>(vtags)));
		}


		// Create the entries
		unsigned int fid = 1;
		for (auto& ff : f)
		{
			auto& tags = std::get<2>(ff);
			unsigned long long ie = 0;
			for (auto& tx : tags)
			{
				wprintf(L"\rWriting entries %llu/%llu...", ie + 1, (unsigned long long)tags.size());
				ie++;
				if (EndProcess)
					break;
				TAG* tt = (TAG*)tx.d.data();
				if (tt->code != (unsigned int)TAGCODE::ITEMMETA)
					continue;

				ITEMMETA* m = (ITEMMETA*)tt;
				m->fid = fid++;
				char* fn = (char*)(m)+sizeof(ITEMMETA);
				ystring fz = fn;

				auto hrw = t.Write((const char*)m, sizeof(ITEMMETA));
				FailWrite(hrw);

				// And the file name
				auto a = fz.a_str();
				hrw = t.Write(a, strlen(a) + 1);
				FailWrite(hrw);
			}
		}
		wprintf(L"\r\n");

		for (auto& ff : f)
		{
			auto& tags = std::get<2>(ff);
			const char* omap = std::get<1>(ff);
			unsigned long long ie = 0;
			for (auto& tx : tags)
			{
				if (EndProcess)
					break;
				wprintf(L"\rWriting data %llu/%llu...", ie + 1, (unsigned long long)tags.size());
				ie++;
				TAG* tt = (TAG*)tx.d.data();
				if (tt->code != (unsigned int)TAGCODE::ITEMMETA)
					continue;

				for (auto& tt : tx.data)
				{
					// And the data
					hrw = t.Write((const char*)& tt.copy, sizeof(tt.copy));
					FailWrite(hrw);
					hrw = t.Write((const char*)omap + tt.ofs, tt.s);
					FailWrite(hrw);
				}
			}
		}
		wprintf(L"\r\n");


		t.Close();
		if (sw.T == 0 && EndProcess == 0)
		{
			BOOL R = MoveFileEx(t.fn.c_str(), files[0].c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
			if (!R)
			{
				t.Delete();
				FailWrite(E_FAIL);
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}


}