#pragma once


void Sign()
{
	using namespace std;
	if (files.size() < 1)
	{
		Help(0, "s");
		return;
	}

	sw.sigs = 1;
	ccx = make_shared<CONSOLECOLOR>();
	TEMPFILE t;

	try
	{
		tpoollib::tpool<> p;
		if (sw.threads > 0)
			p.Create(1, sw.threads);

		// Write header
		HEADER t1;
		auto hrw = t.Write((char*)& t1, sizeof(t1));
		FailWrite(hrw);

		list<TAGDATA> tags;
		vector<TAGDATA> vtags;
		shared_ptr<MAPPER> me = make_shared<MAPPER>(files[0].c_str(), PAGE_READONLY);
		const char* omap = (const char*)me->Map();
		if (omap)
			TagCollect((const char*)omap, me->fs(), tags,vtags);

		if (tags.empty())
		{
			ccx->Color(FOREGROUND_RED | FOREGROUND_INTENSITY);
			wcout << L"Not a BAR file" << endl;
			throw;
		}

		// Create the entries
		for (auto& tx : tags)
		{
			if (EndProcess)
				break;
			TAG* tt = (TAG*)tx.d.data();
			if (tt->code != (unsigned int)TAGCODE::ITEMMETA)
				continue;

			ITEMMETA* m = (ITEMMETA*)tt;
			char* fn = (char*)(m) + sizeof(ITEMMETA);
			ystring fz = fn;

			auto hrw = t.Write((const char*)m, sizeof(ITEMMETA));
			FailWrite(hrw);

			// And the file name
			auto a = fz.a_str();
			hrw = t.Write(a, strlen(a) + 1);
			FailWrite(hrw);
		}

		// Create the signature
		for (auto& tx : tags)
		{
			if (EndProcess)
				break;
			TAG* tt = (TAG*)tx.d.data();
			if (tt->code != (unsigned int)TAGCODE::ITEMMETA)
				continue;

			DIFFLIB::DIFF diff;
			DIFFLIB::MemoryDiffWriter d;
			ITEMMETA* m = (ITEMMETA*)tt;
			for (auto& tt : tx.data)
			{
				if (tt.type == (unsigned int)DATATYPE::DATA)
				{

					if (sw.pwd.length())
					{

					}


					// Decompress if compressed
					vector<char> r;
					if (tt.ctype == (unsigned int)DATACOMP::UNC)
					{
						r.resize(tt.s);
						memcpy(r.data(), omap + tt.ofs, tt.s);
					}
					else
					{
						r.resize(0);
						if (tt.s)
						{
							COMPRESSIONAPI c(tt.ctype);
							c.DecompressX(omap + tt.ofs, tt.s, r);
						}
					}

					DIFFLIB::MemoryRdcFileReader mr(r.data(),r.size());
					auto hr = diff.GenerateSignature(&mr, d);
					if (FAILED(hr))
					{
						char* fn = (char*)(m)+sizeof(ITEMMETA);
						ystring fz = fn;
						ccx->Color(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
						Warnings->emplace_back(ystring().Format(L"Generating signature for %s failed", fz.c_str()));
						continue;
					}

					if (sw.pwd.length())
					{
						std::vector<char> d2;
						if (!ee.Encrypt(d.p(), d2))
							throw;
						d.p() = d2;
					}

					arlock.writelock([&](int& fid)
						{
							// The data
							ITEMDATA da;
							da.fid = m->fid;
							da.dtt = (unsigned int)DATATYPE::SIG;
							da.cmpt = (unsigned int)DATACOMP::UNC;
							da.tag.size += d.sz();
							da.follows = d.sz();
							auto hrw = t.Write((const char*)& da, sizeof(da));
							FailWrite(hrw);

							// And the data
							hrw = t.Write((const char*)d.p().data(), d.sz());
							FailWrite(hrw);
						});
				}
			}
		}

		wprintf(L"\r\n");

		// Copy the data
		for (auto& tx : tags)
		{
			if (EndProcess)
				break;
			TAG* tt = (TAG*)tx.d.data();
			if (tt->code != (unsigned int)TAGCODE::ITEMMETA)
				continue;

			//ITEMMETA* m = (ITEMMETA*)tt;
			for (auto& tt : tx.data)
			{
				if (tt.type == (unsigned int)DATATYPE::DATA)
				{
					// And the data
					hrw = t.Write((const char*)&tt.copy, sizeof(tt.copy));
					FailWrite(hrw);
					hrw = t.Write((const char*)omap + tt.ofs,tt.s);
					FailWrite(hrw);
				}
			}
		}
		wprintf(L"\r\n");
		if (omap)
			me = nullptr;
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
	}
	catch (...)
	{

	}
	t.Delete();
	if (ccx)
		ccx->r();
	ccx = nullptr;

}