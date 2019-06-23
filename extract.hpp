#pragma once


RWMUTEX mCR;
HRESULT CompareRemove(list<FOUNDIT>* CompareList,ITEMMETA* m,TAGDATA& t,std::vector<char>& r,const char* mapz = 0,size_t szz = 0)
{
	RWMUTEXLOCKWRITE wr(&mCR);
	if (CompareList && m->unccrc)
	{
		auto& li = *CompareList;
		// Find to list

		std::list<FOUNDIT>::iterator i = li.begin();
		while (i != li.end())
		{
			if (i->rel != t.fil)
			{
				i++;
				continue;
			}

			MAPPER mm(i->full.c_str());
			auto m3 = mm.Map();
			if (!m3)
			{
				i++;
				return S_OK;
			}

			CRC c3;
			auto cc3 = c3.findcrc32((char*)m3, mm.fs());
			if (m->unccrc != cc3)
			{
				if (!i->DataForCompare.empty())
				{
					// We must test if diff will result in m3
					DIFFLIB::DIFF diff;

					shared_ptr<DIFFLIB::MemoryRdcFileReader> m1;
					vector<char> rxx;
					m1 = make_shared<DIFFLIB::MemoryRdcFileReader>(mapz, szz);
					shared_ptr<DIFFLIB::MemoryRdcFileReader> r1;
					r1 = make_shared<DIFFLIB::MemoryRdcFileReader>(i->DataForCompare.data(), i->DataForCompare.size());

					DIFFLIB::MemoryDiffWriter wr;
					auto res = diff.Reconstruct(r1.get(), m1.get(), 0, wr);
					if (SUCCEEDED(res))
					{
						i->DataForCompare = wr.p();
					}
				}
				else
				{
					i->DataForCompare = r;
				}
				i++;
				return E_FAIL;
			}
			else
			{
				if (!i->DataForCompare.empty())
				{
					// We must test if diff will result in m3
					DIFFLIB::DIFF diff;

					shared_ptr<DIFFLIB::MemoryRdcFileReader> m1;
					vector<char> rxx;
					m1 = make_shared<DIFFLIB::MemoryRdcFileReader>(mapz,szz);
					shared_ptr<DIFFLIB::MemoryRdcFileReader> r1;
					r1 = make_shared<DIFFLIB::MemoryRdcFileReader>(i->DataForCompare.data(),i->DataForCompare.size());

					DIFFLIB::MemoryDiffWriter wr;
					auto res = diff.Reconstruct(r1.get(), m1.get(), 0, wr);
					if (SUCCEEDED(res))
					{
						CRC c4;
						auto cc4 = c4.findcrc32((char*)wr.p().data(), wr.sz());
						if (cc3 == cc4)
						{
							li.erase(i++);
							return S_OK;
						}
					}
					i++;
					return E_FAIL;
				}
				li.erase(i++);
				return S_OK;
			}
		}
	}
	return E_NOINTERFACE;

}

tlock<std::map<unsigned int, std::tuple<TAGDATA*,ITEMMETA*>>> DupCache;
bool ExtractData(const char* mapz,TAGDATA* tt,bool Testing,std::vector<TAGDATA>& vtags,ystring *uu = 0,list<FOUNDIT>* CompareList = 0,bool TestingFolder = false)
{
	using namespace std;
	if (sw.threads > 0)
	{
		CoInitializeEx(0, COINIT_MULTITHREADED);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	}
	if (EndProcess)
		return false;

	bool RR = true;
	TAGDATA& t = *tt;
	TAG* ttf = (TAG*)t.d.data();
	ITEMMETA* m = (ITEMMETA*)ttf;

	if (!uu)
		DupCache->operator[](m->fid) = make_tuple<TAGDATA*,ITEMMETA*>(forward<TAGDATA*>(tt),forward<ITEMMETA*>(m));

	using namespace std;
	char* fd = ((char*)m) + sizeof(ITEMMETA);
	ystring fi = fd;
	if (uu)
		fi = *uu;
	// Extract this item?

/*	wchar_t o[1009];
	swprintf_s(o, 1000, L"About %S\r\n", fd);
	OutputDebugString(o);
	*/

	DIFFLIB::MemoryDiffWriter NewSig;
	vector<char> OldSig;
	vector<char> r;


	auto DupProcess = [&](const char* r1,size_t sz, vector<char>& r2) -> bool
	{
		if (sz != sizeof(unsigned int))
			return false;
		unsigned int rfid = *(unsigned int*)r1;

		// Find this tag
		auto tg = DupCache->operator[](rfid);
		if (get<0>(tg) == 0)
		{
			// Search manually
			for (auto& t : vtags)
			{
				TAG* tt = (TAG*)t.d.data();
				if (tt->code != (unsigned int)TAGCODE::ITEMMETA)
					continue;
				ITEMMETA* m = (ITEMMETA*)tt;
				if (m->fid == rfid)
				{
					tg = make_tuple<TAGDATA*, ITEMMETA*>(forward<TAGDATA*>(&t), forward<ITEMMETA*>(m));
					break;
				}
			}
		}

		if (get<0>(tg) == 0)
		{
			wprintf(L"Extract could not find dup data for %s.\r\n",fi.c_str());
			return false;
		}

		return ExtractData(mapz, get<0>(tg), Testing, vtags,&fi,CompareList,TestingFolder);
	};

	try
	{
		for (auto& d : t.data)
		{
			if (d.type == (unsigned int)DATATYPE::SIG)
			{
				if (Testing)
				{
					if (sw.pwd.length())
					{
						if (!ee.Decrypt(mapz + d.ofs, d.s, OldSig))
							throw;
					}
					else
					{
						OldSig.resize(d.s);
						memcpy(OldSig.data(), mapz + d.ofs, d.s);
					}
				}
			}

			if (d.type == (unsigned int)DATATYPE::DIFF)
			{
				if (Testing)
				{
					// Existing comparison
					vector<char>vdd;
					auto hr = CompareRemove(CompareList, m, t,vdd,mapz + d.ofs,d.s);
					if (hr == E_NOINTERFACE)
					{

					}
					continue;
				}

				DIFFLIB::DIFF diff;

				// The file, on disk
				wstring trg = files[1];
				trg += L"\\";
				trg += fi;
				shared_handle hX = CreateFile(trg.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
				if (hX == INVALID_HANDLE_VALUE)
				{
					Warnings->emplace_back(ystring().Format(L"Diff for non existing file found: %s", trg.c_str()));
				}
				
				if (hX != INVALID_HANDLE_VALUE)
				{

					shared_ptr<DIFFLIB::MemoryRdcFileReader> m1;
					vector<char> rxx;
					if (sw.pwd.length())
					{
						if (!ee.Decrypt(mapz + d.ofs, d.s, rxx))
							throw;
						m1 = make_shared<DIFFLIB::MemoryRdcFileReader>(rxx.data(), rxx.size());
					}
					else
						m1 = make_shared<DIFFLIB::MemoryRdcFileReader>(mapz + d.ofs, d.s);

					DIFFLIB::FileRdcFileReader r1(hX);
					DIFFLIB::MemoryDiffWriter wr;
					auto res =diff.Reconstruct(&r1, m1.get(), 0, wr);
					if (FAILED(res))
					{
						Warnings->emplace_back(ystring().Format(L"Reconstructing %s failed", trg.c_str()));
						throw;
					}
					r = wr.p();
					hX.Close();

					if (sw.T == 0)
					{
						shared_handle hX = CreateFile(trg.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
						if (hX == 0 || hX == INVALID_HANDLE_VALUE)
						{
							Warnings->emplace_back(ystring().Format(L"Writing %s failed", trg.c_str()));
							continue;
						}

						DWORD A = 0;
						WriteFile(hX, r.data(), (DWORD)r.size(), &A, 0);
						SetFileTime(hX, (FILETIME*)& m->t1, (FILETIME*)& m->t2, (FILETIME*)& m->t3);
					}

				}

			}

			if (d.type == (unsigned int)DATATYPE::DATA)
			{
				if (sw.pwd.length())
				{
					if (d.ctype == (unsigned int)DATACOMP::UNC)
					{
						if (!ee.Decrypt(mapz + d.ofs, d.s, r))
							throw;
					}
					else
					{
						vector<char> r2;
						if (!ee.Decrypt(mapz + d.ofs, d.s, r2))
							throw;

						if (d.ctype == COMPRESS_ALGORITHM_DUP)
						{
							if (!DupProcess(r2.data(),r2.size(), r))
								throw;
						}
						else
						{
							COMPRESSIONAPI c(d.ctype);
							auto hrx = c.DecompressX(r2.data(), r2.size(), r);
							if (FAILED(hrx))
								RR = false;
						}
					}
				}
				else
				{
					if (d.ctype == (unsigned int)DATACOMP::UNC)
					{
						r.resize(d.s);
						memcpy(r.data(), mapz + d.ofs, d.s);
					}
					else
					{
						r.resize(0);
						if (d.s)
						{
							if (d.ctype == COMPRESS_ALGORITHM_DUP)
							{
								if (!DupProcess(mapz + d.ofs,d.s, r))
									throw;
							}
							else
							{

								COMPRESSIONAPI c(d.ctype);
								auto hrx = c.DecompressX(mapz + d.ofs, d.s, r);
								if (FAILED(hrx))
									RR = false;
							}
						}
					}
				}

				if (Testing)
				{
					if (d.etype == 1 && sw.pwd.empty())
						RR = false;

					// CRC checks
					if (m->unccrc != 0)
					{
						CRC c;
						auto c2 = c.findcrc32(r.data(), r.size());
						if (m->unccrc != c2 && c2 != -1 && m->unccrc != -1)
						{
							RR = false;
						}
					}

					if (CompareList && !m->unccrc)
						RR = true;

					// Existing comparison
					CompareRemove(CompareList, m, t,r);

					// Signature
					DIFFLIB::DIFF diff;
					DIFFLIB::MemoryRdcFileReader mr((const char*)r.data(), r.size());
					diff.GenerateSignature(&mr, NewSig);
					continue;
				}

				if (d.ctype == COMPRESS_ALGORITHM_DUP)
					continue; // we extracted it via dup

				// Save it
				wstring trg = files[1];
				trg += L"\\";
				trg += fi;

				if (sw.T == 0 && RR)
				{
					shared_handle hX = CreateFile(trg.c_str(), GENERIC_WRITE, 0, 0, CREATE_NEW, 0, 0);
					if (hX == 0 || hX == INVALID_HANDLE_VALUE)
					{
						int Answer = Ask(ystring().Format(L"File %s exists. Overwrite?", trg.c_str()));
						if (Answer == 'Y')
							hX = CreateFile(trg.c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
						if (hX == 0 || hX == INVALID_HANDLE_VALUE)
							Warnings->emplace_back(ystring().Format(L"Writing %s failed", trg.c_str()));
					}

					if (hX == 0 || hX == INVALID_HANDLE_VALUE)
						continue;

					DWORD A = 0;
					WriteFile(hX, r.data(), (DWORD)r.size(), &A, 0);
					SetFileTime(hX, (FILETIME*)& m->t1, (FILETIME*)& m->t2, (FILETIME*)& m->t3);
				}
			}
		}

		if (Testing)
		{
			if (OldSig.size() != 0 && OldSig != NewSig.p() && !r.empty())
			{
				RR = false;
			}

		}

		return RR;
	}
	catch (...)
	{
		return false;
	}
}

HRESULT OneDownload(RGF::GOD::ONEDRIVE& one, ystring fid,TEMPFILE& fil)
{
	RGF::GOD::RESTAPI::DATA d;
	auto hr = one.Download(fid.a_str(), 0, &d,0,(unsigned long long)-1, [](unsigned long long f, unsigned long long t, void* l) -> HRESULT
		{
			if (EndProcess)
				return E_FAIL;

			f *= 100;
			f = (int)(f / t);
			printf("\rDownloading from OneDrive...%u%%...", (unsigned long)f);


			return S_OK;
		}
	,0);
	if (FAILED(hr))
		return hr;
	return fil.Write(d.data(), d.size());
}
list<FOUNDIT> CompareList;

void Extract(bool Testing = false, bool Folder = false, bool Compare = false)
{
	using namespace std;
	ccx = make_shared<CONSOLECOLOR>();
	if (sw.pwd.length())
	{
		if (!ee.Prepare(sw.pwd.a_str()))
		{
			throw 0;
		}
	}

	try
	{
		ThreadPool p2(sw.threads ? sw.threads : 4);
		std::vector<std::future<int>> p2results;
		if (files.size() < 2 && !Testing)
		{
			Help(0, "e");
			return;
		}
		if (files.size() < 2 && Compare)
		{
			Help(0, "c");
			return;
		}
		if (files.size() < 1 && Testing)
		{
			Help(0, "t");
			return;
		}

		if (Compare && CompareList.empty())
		{
			// Search for files
			tpoollib::tpool<> p;
			if (sw.threads > 0 && UseTP2 == false)
				p.Create(1, sw.threads);
			BuildFileList(CompareList, p);
		}

		ystring ffid;
		RGF::GOD::ONEDRIVE one;
		RGF::GOD::GOOGLEDRIVE goo;
		int Remote = 0;

		if (Folder)
		{
			// This is a folder with data, we enumerate it and extract all full and incrementals for it
			vector<FOUNDIT> fld;
			if (wcsstr(files[0].c_str(), L"onedrive:") != 0)
			{
				// Load rgf
				XML3::XML rr(sw.rgf.c_str());
				RGF::RGBF s;
				s.onedrive.tokens.resize(3);
				s.onedrive.id = rr.GetRootElement()["tokens"]["onedrive"].vv("u").GetValue();
				s.onedrive.secret = rr.GetRootElement()["tokens"]["onedrive"].vv("p").GetValue();
				s.onedrive.tokens[0] = rr.GetRootElement()["tokens"]["onedrive"].vv("t1").GetValue();
				s.onedrive.tokens[1] = rr.GetRootElement()["tokens"]["onedrive"].vv("t2").GetValue();

				const wchar_t* rdir = files[0].c_str() + 9;

				one.SetClient(s.onedrive.id.c_str(), s.onedrive.secret.c_str());
				int auth = one.Auth(s.onedrive.tokens);
				if (auth == 0)
					throw;

				if (s.onedrive.tokens.size() == 3)
				{
					rr.GetRootElement()["tokens"]["onedrive"].vv("t1").SetValue(s.onedrive.tokens[0].c_str());
					rr.GetRootElement()["tokens"]["onedrive"].vv("t2").SetValue(s.onedrive.tokens[1].c_str());
				}
				rr.Save();

				auto root = one.GetRootFolderID();
				ffid = one.IDFromPath(ystring(rdir).a_str());
				auto dr = one.dir(ffid.a_str(), true);
				if (dr.empty())
					throw;

				// Parse this list
				std::vector<std::tuple<std::string, std::string, std::string>> AllItems;
				RGF::GOD::EnumNames(one, dr, &AllItems, 2, 0);

				for (auto& a : AllItems)
				{
					string mim = get<2>(a);
					if (mim == "application/vnd.google-apps.folder")
						continue;

					FOUNDIT it;
					it.f = get<0>(a);
					it.full = get<1>(a);
					it.rel = get<1>(a);
					fld.push_back(it);
				}

				Remote = 2;

			}
			else
			{
				CheckFiles(files[0].c_str(), files[0].c_str(), 0, (LPARAM)& fld,
					[&](const TCHAR* a1, const TCHAR* a2, WIN32_FIND_DATA* e, LPARAM, int dep) -> HRESULT
					{
						if (dep > 0)
							return S_OK;
						if (EndProcess)
							return E_FAIL;
						ystring fi = a2;
						fi += L"\\";
						fi += e->cFileName;

						if (e->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							return S_FALSE;


						FOUNDIT it;
						it.f = e->cFileName;
						it.full = fi;
						it.rel = fi.c_str() + wcslen(a1) + 1;
						it.w = *e;
						fld.push_back(it);
						return S_OK;
					}
				, 0);
			}

			map<unsigned long long, FULLANDINCR> fai;
			FullAndIncr(fld, fai);

			if (fai.empty())
				return;

			if (Remote == 2)
			{
				// Download full somewhere, extract it
				for (auto& ff : fai)
				{
					TEMPFILE f0;
					wcout << L"Downloading full backup...\r\n";
					if (FAILED(OneDownload(one,ff.second.fid,f0)))
						throw;
					f0.Close();
					files[0] = f0.fn;

					Extract(Testing, false);
					f0.Delete();
					for (auto& fx : ff.second.incrs)
					{
						TEMPFILE f1;
						wcout << L"Downloading incremental backup...\r\n";
						if (FAILED(OneDownload(one, fx.fid, f1)))
							throw;
						f1.Close();
						files[0] = f1.fn;
						Extract(Testing, false);
						f1.Delete();
					}
				}

			}
			else
			{
				for (auto& ff : fai)
				{
					files[0] = ff.second.full;
					Extract(Testing, false,Compare);
					for (auto& fx : ff.second.incrs)
					{
						files[0] = fx.full;
						Extract(Testing, false,Compare);
					}
				}
			}

			return;
		}



		MAPPER m(files[0].c_str(), PAGE_READONLY);
		const char* map = (const char*)m.Map();
		if (!map)
		{
			wcout << L"Cannot open " << files[0].c_str() << endl;
			return;
		}

		auto fs = m.fs();
		wcout << L"Reading " << files[0].c_str() << endl;

		list<TAGDATA> tags;
		vector<TAGDATA> vtags;
		if (!TagCollect(map, fs, tags, vtags))
		{
			wcout << L"Not a BAR file" << endl;
			return;
		}

		// Extract
		// Reconstruct Folders
		if (!Testing)
			wcout << L"Creating directory structure...\r\n";
		for (auto& t : tags)
		{
			if (sw.T)
				break;
			if (Testing)
				break;

			TAG* tt = (TAG*)t.d.data();
			if (EndProcess)
				break;

			ITEMMETA* m = (ITEMMETA*)tt;
			char* fd = ((char*)m) + sizeof(ITEMMETA);
			ystring fi = fd;


			// Save it
			wstring trg = files[1];
			trg += L"\\";
			trg += fi;

			vector<wchar_t> tx(trg.length() + 1);
			wcscpy_s(tx.data(), tx.size(), trg.c_str());

			PathCchRemoveFileSpec(tx.data(), trg.length());
			wchar_t* atx = tx.data();
			SHCreateDirectory(0, atx);
		}

		auto itemss = tags.size();
		size_t iidx = 0;
		for (auto& t : tags)
		{
			TAG* tt = (TAG*)t.d.data();
			if (EndProcess)
				break;

			// Include ?
			ITEMMETA* m = (ITEMMETA*)tt;
			char* fd = ((char*)m) + sizeof(ITEMMETA);
			ystring fi = fd;
			bool id = IncludeItem(fi.a_str(), false);
			if (!id)
				continue;

			if (sw.threads == 0)
			{
				iidx++;
				if ((iidx % 10) == 0 || iidx == itemss)
				{
					if (Testing)
						wprintf(L"\rTesting %zu of %zu ...", iidx, itemss);
					else
						wprintf(L"\rExtracting %zu of %zu ...", iidx, itemss);
				}
			}

			if (tt->code == (unsigned int)TAGCODE::ITEMMETA)
			{
				if (sw.threads == 0)
				{
					bool RX = ExtractData(map, &t, Testing,vtags,0,Compare ? &CompareList : 0,Folder);
					if (!RX && !EndProcess)
						Warnings->emplace_back(ystring().Format(L"%s failed", fi.c_str()));
				}
				else
				{
					p2results.emplace_back(p2.enqueue([&]() -> int {
						bool RX = ExtractData(map, &t, Testing,vtags, 0, Compare ? &CompareList : 0, Folder);
						if (!RX && !EndProcess)
							Warnings->emplace_back(ystring().Format(L"%s failed", fi.c_str()));
						return 1;
						}));
				}
			}
		}
		if (sw.threads > 0)
		{
			for (auto& e : p2results)
			{
				iidx++;
				if (!EndProcess)
				{
					if ((iidx % 10) == 0 || iidx == itemss)
					{
						if (Testing)
							wprintf(L"\rTesting %zu of %zu ...", iidx, itemss);
						else
							wprintf(L"\rExtracting %zu of %zu ...", iidx, itemss);
					}
				}
				e.get();
			}
			p2results.clear();
		}
		wprintf(L"\r\n");
	}
	catch (...)
	{

	}
	if (ccx)
		ccx->r();
	ccx = nullptr;
}

