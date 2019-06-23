#pragma once


struct INCR
{
	ystring f;
	std::list<TAGDATA> tags;
	std::vector<TAGDATA> vtags;
	std::shared_ptr<MAPPER> m;
	const char* map = 0;
	unsigned long maxfid = 0;
};
std::vector<INCR> incrs;


void CALLBACK CreateAndWriteDiff(PTP_CALLBACK_INSTANCE, PVOID vv, PTP_WORK w)
{
	using namespace std;
	if (EndProcess)
		return;
	V vvv;
	FOUNDIT* f = (FOUNDIT*)vv;
	if (w)
	{
		CoInitializeEx(0, COINIT_MULTITHREADED);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	}

	if (f->incridx >= incrs.size())
	{
		FailWrite(E_FAIL);
		return;
	}
	if (!f->OpenMapper())
	{
		Warnings->emplace_back(ystring().Format(L"Cannot open file %s", f->full.c_str()));
		return;
	}

	auto& inc = incrs[f->incridx];

	shared_ptr<MAPPER> me4 = make_shared<MAPPER>(inc.f.c_str(), PAGE_READONLY);
	const char* omap4 = (const char*)me4->Map();


	DIFFLIB::DIFF d;
	for (auto& tt : f->LinkForDiff->data)
	{
		if (tt.type == (unsigned int)DATATYPE::SIG)
		{
			vector<char> ss1;
			ss1.resize(tt.s);
			memcpy(ss1.data(), omap4 + tt.ofs, tt.s);
			if (sw.pwd.length())
			{
				vector<char> ss2;
				if (!ee.Decrypt(ss1.data(), ss1.size(), ss2))
					throw;
				ss1 = ss2;
			}
			else
			{
			}

			DIFFLIB::MemoryRdcFileReader sig1(ss1.data(), ss1.size());
			void* map = f->mapper->Map();
			DIFFLIB::MemoryRdcFileReader r2((const char*)map, f->mapper->fs());
			DIFFLIB::MemoryDiffWriter sig2;
			d.GenerateSignature(&r2, sig2);
			auto re2 = sig2.GetReader();

			DIFFLIB::MemoryDiffWriter wr;
			auto hr = d.GenerateDiff(&sig1, re2, &r2, wr);
			if (FAILED(hr))
			{
				ccx->Color(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
				Warnings->emplace_back(ystring().Format(L"Generating signature for %s failed", f->full.c_str()));
				return;
			}

			if (sw.pwd.length())
			{
				std::vector<char> d2;
				if (!ee.Encrypt(wr.p(), d2))
					throw;
				wr.p() = d2;
			}

			arlock.writelock([&](int& fid)
				{
					ITEMDATA da;
					da.fid = f->linkfid;
					da.dtt = (unsigned int)DATATYPE::DIFF;
					da.cmpt = (unsigned int)0;
					da.tag.size += wr.p().size();
					da.follows = wr.p().size();
					auto hrw = f->t->Write((const char*)& da, sizeof(da));
					FailWrite(hrw);

					// And the data
					hrw = f->t->Write((const char*)wr.p().data(), wr.p().size());
					FailWrite(hrw);

					// Get fp
					auto fpc = f->t->fp(0);

					// And update meta
					f->t->fp(f->tempfp, true);
					if (true)
					{
						// 
						CRC c;
						f->tempmeta.cmpcrc = c.findcrc32((const char*)wr.p().data(), wr.p().size());
					}

					f->tempmeta.fid = f->linkfid;
					f->tempmeta.cmpsize = wr.p().size();
				//	f->tempmeta.uncsize = wr.p().size();
					hrw = f->t->Write((const char*)& f->tempmeta, sizeof(f->tempmeta));
					FailWrite(hrw);

					// Set fp
					f->t->fp(fpc, true);

					// Free the mapper
					f->CloseMapper();

				});


		}
	}
};


volatile LONG64 NeedUpdate = 0;
void DiffSearch(FOUNDIT* pit)
{
	FOUNDIT& it = *pit;
	if (EndProcess)
		return;
	using namespace std;
	// Find this item in any of the incremental files
	it.diffsearch = E_NOINTERFACE;
	for (size_t iidx = 0; iidx < incrs.size(); iidx++)
	{
		auto& ii = incrs[iidx];
		ITEMMETAANDFILE imf;
		auto hr2 = FindFileInTags(ii.tags,ii.vtags, it.rel, imf, &it);
		if (hr2 != E_NOINTERFACE)
		{
			it.diffsearch = hr2;
			it.linkfid = imf.fid;
			if (hr2 == S_FALSE)
			{
				for (auto& ttt : imf.td.data)
				{
					if (ttt.type == (unsigned int)DATATYPE::SIG)
					{
						it.incridx = iidx;
						it.LinkForDiff = make_shared<TAGDATA>(imf.td);
					}
				}
			}
			if (it.diffsearch == S_FALSE && it.LinkForDiff == 0)
				it.diffsearch = E_NOINTERFACE;
		}
	}
	if (it.diffsearch != S_OK)
		InterlockedIncrement64(&NeedUpdate);

}


void OneUpload(RGF::GOD::ONEDRIVE& one, ystring fn,std::string fid,ystring nn)
{
	MAPPER m(fn.c_str(), PAGE_READONLY);
	auto mp = m.Map();
	if (!mp)
		throw;
	std::string resd, retd;
	auto hr = one.Upload2(0, 0, (const char*)mp, m.fs(), fid.c_str(), nn.a_str(), resd, retd, 
		[](unsigned long long f, unsigned long long t, void* l) -> HRESULT
		{
			if (EndProcess)
				return E_FAIL;

			f *= 100;
			f = (int)(f / t);
			printf("\rUploading to OneDrive...%u%%...", (unsigned long)f);


			return S_OK;
		}
		, 0);
	if (FAILED(hr))
		throw;

	printf("\r\nDone\r\n");

}


void GooUpload(RGF::GOD::GOOGLEDRIVE& goo, ystring fn, std::string fid, ystring nn)
{
	MAPPER m(fn.c_str(), PAGE_READONLY);
	auto mp = m.Map();
	if (!mp)
		throw;
	std::string resd, retd;
	auto hr = goo.UploadOnce(0, (const char*)mp, m.fs(), fid.c_str(), nn.a_str(), resd,
		[](unsigned long long f, unsigned long long t, void* l) -> HRESULT
		{
			if (EndProcess)
				return E_FAIL;

			f *= 100;
			f = (int)(f / t);
			printf("\rUploading to GoogleDrive...%u%%...", (unsigned long)f);


			return S_OK;
		}
	, 0);
	if (FAILED(hr))
		throw;

	printf("\r\nDone\r\n");

}

struct FULLANDINCR
{
	ystring full;
	ystring fid;
	SYSTEMTIME sT;
	std::vector<FULLANDINCR> incrs;

	bool operator <(const FULLANDINCR& fx)
	{
		FILETIME f1, f2;
		SystemTimeToFileTime(&sT, &f1);
		SystemTimeToFileTime(&fx.sT, &f2);
		unsigned long long l1 = 0, l2 = 0;
		memcpy(&l1, &f1, sizeof(l1));
		memcpy(&l2, &f2, sizeof(l2));
		if (l1 < l2)
			return true;
		return false;
	}
};


void FullAndIncr(std::vector<FOUNDIT>& fld, std::map<unsigned long long, FULLANDINCR>& fai)
{
	using namespace std;
	for (auto& f : fld)
	{
		SYSTEMTIME s1;
		SYSTEMTIME s2;
		auto fx = [](string f, SYSTEMTIME& filet, SYSTEMTIME& file2) -> int
		{
			if (f.length() < 19)
				return 0;
			const char* fn = f.c_str();

			char a2[10] = { 0 };
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 5, 4);	filet.wYear = (WORD)atoi(a2);
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 9, 2);	filet.wMonth = (WORD)atoi(a2);
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 11, 2);	filet.wDay = (WORD)atoi(a2);
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 13, 2);	filet.wHour = (WORD)atoi(a2);
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 15, 2);	filet.wMinute = (WORD)atoi(a2);
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 17, 2);	filet.wSecond = (WORD)atoi(a2);

			if (filet.wYear <= 2000)
				return 0;

			if (f.length() < 34)
				return 1;

			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 20, 4);	file2.wYear = (WORD)atoi(a2);
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 24, 2);	file2.wMonth = (WORD)atoi(a2);
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 26, 2);	file2.wDay = (WORD)atoi(a2);
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 28, 2);	file2.wHour = (WORD)atoi(a2);
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 30, 2);	file2.wMinute = (WORD)atoi(a2);
			memset(a2, 0, 10);	memcpy_s(a2, 10, fn + 32, 2);	file2.wSecond = (WORD)atoi(a2);

			if (file2.wYear <= 2000)
				return 0;

			return 2;
		};
		wstring a = f.full.c_str();
		vector<wchar_t> aa(wcslen(a.c_str()) + 1);
		memcpy(aa.data(), a.c_str(), wcslen(a.c_str()) * 2);
		const wchar_t* a2 = wcsrchr(aa.data(), '\\');
		if (a2)
			a2++;
		else
			a2 = aa.data();

		auto r1 = fx(ystring(a2).a_str(), s1, s2);
		if (r1 == 0)
			continue;
		FILETIME ff1;
		SystemTimeToFileTime(&s1, &ff1);
		unsigned long long l1 = 0;
		memcpy(&l1, &ff1, sizeof(FILETIME));

		if (_wcsnicmp(a2, L"FULL_", 5) == 0)
		{
			fai[l1].full = f.full;
			fai[l1].fid = f.f;
			fai[l1].sT = s1;
		}
		if (_wcsnicmp(a2, L"INCR_", 5) == 0)
		{
			FULLANDINCR fff;
			fff.full = f.full;
			fff.sT = s2;
			fff.fid = f.f;
			fai[l1].incrs.push_back(fff);
		}

		std::sort(fai[l1].incrs.begin(), fai[l1].incrs.end());
	}

}

void Update(bool Folder = false)
{
	using namespace std;

	if (sw.pwd.length())
	{
		if (!ee.Prepare(sw.pwd.a_str()))
		{
			throw 0;
		}
	}

	if (Folder)
	{
		sw.incr.clear();
		if (files.size() < 2)
		{
			Help(0, "uu");
			return;
		}
	}
	else
	{
		if (files.size() < 2 || sw.incr.empty())
		{
			Help(0, "u");
			return;
		}
	}

	// files[0] = archive to create
	// files[...] = directories
	// incrs = files

	// If Folder mode, incrs are found from the folder 
	SYSTEMTIME sL = { 0 };
	int Remote = 0;
	RGF::GOD::ONEDRIVE one;
	RGF::GOD::GOOGLEDRIVE goo;
	ystring ffid;

	if (Folder)
	{
		// Search folder
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
			RGF::GOD::EnumNames(one, dr, &AllItems, 2,0);

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
		if (wcsstr(files[0].c_str(), L"gdrive:") != 0)
		{
			// Load rgf
			XML3::XML rr(sw.rgf.c_str());
			RGF::RGBF s;
			s.google.tokens.resize(3);
			s.google.id = rr.GetRootElement()["tokens"]["google"].vv("u").GetValue();
			s.google.secret = rr.GetRootElement()["tokens"]["google"].vv("p").GetValue();
			s.google.tokens[0] = rr.GetRootElement()["tokens"]["google"].vv("t1").GetValue();
			s.google.tokens[1] = rr.GetRootElement()["tokens"]["google"].vv("t2").GetValue();

			const wchar_t* rdir = files[0].c_str() + 7;

			goo.SetClient(s.google.id.c_str(), s.google.secret.c_str());
			int auth = goo.Auth(s.google.tokens);
			if (auth == 0)
				throw;

			if (s.google.tokens.size() == 3)
			{
				rr.GetRootElement()["tokens"]["google"].vv("t1").SetValue(s.google.tokens[0].c_str());
				rr.GetRootElement()["tokens"]["google"].vv("t2").SetValue(s.google.tokens[1].c_str());
			}
			rr.Save();

			auto root = goo.GetRootFolderID();
			ffid = goo.IDFromPath(ystring(rdir).a_str());
			auto dr = goo.dir(ffid.a_str(), true);
			if (dr.empty())
				throw;

			// Parse this list
			std::vector<std::tuple<std::string, std::string, std::string>> AllItems;
			RGF::GOD::EnumNames(goo, dr, &AllItems, 1,0);

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

			Remote = 1;

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
		FullAndIncr(fld,fai);

		if (fai.empty())
		{
			// Do an A instead
			if (Remote == 2)
			{
				TEMPFILE tx;
				tx.Close();
				tx.Delete();
				auto fis = files;
				files[0] = tx.fn;
				Archive2();

				// Upload
				files = fis;
				ystring fn;
				SYSTEMTIME sT;
				GetLocalTime(&sT);
				fn.Format(L"FULL_%04u%02u%02u%02u%02u%02u.bar", sT.wYear, sT.wMonth, sT.wDay, sT.wHour, sT.wMinute, sT.wSecond);
				OneUpload(one,tx.fn.c_str(),ffid.a_str(),fn);
				if (sw.keep.length())
				{
					ystring t = sw.keep;
					t += L"\\";
					t += fn;
					if (!MoveFile(tx.fn.c_str(), t.c_str()))
						wcout << L"File cannot be kept to " << sw.keep << endl;
				}
				else
					tx.Delete();
			}
			else
			if (Remote == 1)
			{
				TEMPFILE tx;
				tx.Close();
				tx.Delete();
				auto fis = files;
				files[0] = tx.fn;
				Archive2();

				// Upload
				files = fis;
				ystring fn;
				SYSTEMTIME sT;
				GetLocalTime(&sT);
				fn.Format(L"FULL_%04u%02u%02u%02u%02u%02u.bar", sT.wYear, sT.wMonth, sT.wDay, sT.wHour, sT.wMinute, sT.wSecond);
				GooUpload(goo, tx.fn.c_str(), ffid.a_str(), fn);
				if (sw.keep.length())
				{
					ystring t = sw.keep;
					t += L"\\";
					t += fn;
					if (!MoveFile(tx.fn.c_str(), t.c_str()))
						wcout << L"File cannot be kept to " << sw.keep << endl;
				}
				else
					tx.Delete();

			}
			else
			{
				ystring fn;
				SYSTEMTIME sT;
				GetLocalTime(&sT);
				fn.Format(L"\\FULL_%04u%02u%02u%02u%02u%02u.bar", sT.wYear, sT.wMonth, sT.wDay, sT.wHour, sT.wMinute, sT.wSecond);
				files[0] += fn;
				Archive2();
			}
			return;
		}

		// Will do on first fai
		for (auto& faii : fai)
		{
			wprintf(L"Getting full backup %s...\r\n", faii.second.full.c_str());
			sL = faii.second.sT;
			auto fx = files;

			files.clear();
			if (Remote)
				files.push_back(faii.second.fid);
			else
				files.push_back(faii.second.full);

			TEMPFILE t;
			t.Close();
			t.Delete();

			files.push_back(t.fn.c_str());
			HRESULT hrx = 0;
			if (Remote == 2) hrx = Separate(&one);
			if (Remote == 1) hrx = Separate(&goo);
			if (Remote == 0) hrx = Separate(0);
			if (FAILED(hrx))
				return;
			
			sw.incr.push_back(t.fn.c_str());
			for (auto& more : faii.second.incrs)
			{
				wprintf(L"Getting incremental backup %s...\r\n", more.full.c_str());
				files.clear();
				if (Remote)
					files.push_back(more.fid);
				else
					files.push_back(more.full.c_str());
				TEMPFILE t;
				t.Close();
				t.Delete();

				files.push_back(t.fn.c_str());
				HRESULT hrx = 0;
				if (Remote == 2) hrx = Separate(&one);
				if (Remote == 1) hrx = Separate(&goo);
				if (Remote == 0) hrx = Separate(0);
				if (FAILED(hrx))
					return;

				sw.incr.push_back(t.fn.c_str());
			}

			files = fx;
			break;
		}
	}

	ccx = make_shared<CONSOLECOLOR>();
	TEMPFILE t;

	try
	{
		tpoollib::tpool<> p;
		if (sw.threads > 0 && UseTP2 == false)
			p.Create(1, sw.threads);
		ThreadPool p2(sw.threads ? sw.threads : 4);
		if (sw.threads)
			wprintf(L"Using %u threads.\r\n", sw.threads);
		std::vector<std::future<int>> p2results;

		// Write header
		HEADER t1;
		auto hrw = t.Write((char*)& t1, sizeof(t1));
		FailWrite(hrw);

		// Search for files
		list<FOUNDIT> items;
		BuildFileList(items, p);

		/*
			When item is not found in the other archives, put it in full
			When it's found, either don't put it (if same) or put a diff
		*/

		for (auto& a : sw.incr)
		{
			wprintf(L"Reading incremental archive ...\r\n");
			INCR ii;
			ii.f = a;
			ii.m = make_shared<MAPPER>(a.c_str(), PAGE_READONLY);
			ii.map = (const char*)ii.m->Map();
			if (ii.map)
				ii.maxfid = TagCollect((const char*)ii.map, ii.m->fs(), ii.tags,ii.vtags);
			incrs.push_back(ii);
		}

		auto itemss = items.size();
		size_t iidx = 0;
		for (auto& it : items)
		{
			if (sw.threads == 0)
			{
				iidx++;
				if ((iidx % 10) == 0 || iidx == itemss)
				{
					wprintf(L"\rLooking up %zu of %zu [%llu needed updates so far]...", iidx, itemss,NeedUpdate);
				}
			}

			if (sw.threads == 0)
				DiffSearch(&it);
			else
			{
				p2results.emplace_back(p2.enqueue([&]() -> int {
					DiffSearch(&it);
					return 1;
					}));

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
						wprintf(L"\rLooking up %zu of %zu [%llu needed updates so far]...", iidx, itemss, NeedUpdate);
				}
				e.get();
			}
			p2results.clear();
		}
		wprintf(L"\r\n");

		// Create file entries
		// Update fid
		unsigned long fid = 0;
		for (auto& ii : incrs)
		{
			if (ii.maxfid >= fid)
				fid = ii.maxfid + 1;
		}

		for (auto& fi : items)
		{
			if (EndProcess)
				break;
			fi.t = &t;
			if (fi.diffsearch == S_OK)
				continue; // Skip this, we have it completely

			if (!fi.OpenMapper())
			{
				Warnings->emplace_back(ystring().Format(L"Cannot open %s", fi.full.c_str()));
				continue;
			}

			ITEMMETA me;
			if (fi.diffsearch == E_NOINTERFACE)
			{
				fid++;
				fi.fid = fid;
			}
			else
			{
				fi.fid = fi.linkfid;
			}

			me.attr = fi.w.dwFileAttributes;
			ULARGE_INTEGER li;
			li.HighPart = fi.w.nFileSizeHigh;
			li.LowPart = fi.w.nFileSizeLow;
			me.uncsize = li.QuadPart;
			me.cmpsize = 0;// d.size();

			GetFileTime(fi.mapper->hF, (LPFILETIME)& me.t1, (LPFILETIME)& me.t2, (LPFILETIME)& me.t3);
			if (true)
			{
				CRC c;
				void* map = fi.mapper->Map();
				me.unccrc = c.findcrc32((const char*)map, fi.mapper->fs());
				fi.CloseMapper();
			}

			me.tag.size = sizeof(ITEMMETA) + strlen(fi.rel.a_str()) + 1;
		
			me.fid = fi.fid;
			fi.tempfp = t.fp(0);
			fi.tempmeta = me;
			auto hrw = t.Write((const char*)& me, sizeof(me));
			FailWrite(hrw);

			// And the file name
			auto a = fi.rel.a_str();
			hrw = t.Write(a, strlen(a) + 1);
			FailWrite(hrw);
		}

		// Sigs
		for (auto& fi : items)
		{
			if (EndProcess)
				break;
			fi.t = &t;
			if (fi.diffsearch == S_OK)
				continue; // Skip this, we have it completely

			if (sw.threads == 0)
				CreateAndWriteSignature(0, (PVOID)& fi, 0);
			else
			{
				if (UseTP2)
				{
					p2results.emplace_back(p2.enqueue([&]() -> int {
						CreateAndWriteSignature(0, (PVOID)& fi, (PTP_WORK)1);
						return 1;
						}));
				}
				else
				{
					auto workit = p.CreateItem<PTP_WORK, PTP_WORK_CALLBACK>(CreateAndWriteSignature, (PVOID)& fi);
					p.RunItem(workit);
				}
			}
		}
		if (sw.threads > 0)
		{
			if (UseTP2)
			{
				for (auto& e : p2results)
					e.get();
				p2results.clear();
			}
			else
				p.Join();
		}


		// Data
		for (auto& fi : items)
		{
			if (EndProcess)
				break;
			fi.t = &t;
			if (fi.diffsearch == S_OK)
				continue; // Skip this, we have it completely

			if (fi.diffsearch == E_NOINTERFACE)
			{
				// Put it completely
				if (sw.threads == 0)
					CreateAndWriteData(0, (PVOID)& fi, 0);
				else
				{
					if (UseTP2)
					{
						p2results.emplace_back(p2.enqueue([&]() -> int {
							CreateAndWriteData(0, (PVOID)& fi, (PTP_WORK)1);
							return 1;
							}));
					}
					else
					{
						auto workit = p.CreateItem<PTP_WORK, PTP_WORK_CALLBACK>(CreateAndWriteData, (PVOID)& fi);
						p.RunItem(workit);
					}
				}
			}
			else
			{
				// Store as diff
				if (sw.threads == 0)
					CreateAndWriteDiff(0, (PVOID)& fi, 0);
				else
				{
					if (UseTP2)
					{
						p2results.emplace_back(p2.enqueue([&]() -> int {
							CreateAndWriteDiff(0, (PVOID)& fi, (PTP_WORK)1);
							return 1;
							}));
					}
					else
					{
						auto workit = p.CreateItem<PTP_WORK, PTP_WORK_CALLBACK>(CreateAndWriteDiff, (PVOID)& fi);
						p.RunItem(workit);
					}
				}
			}
		}
		if (sw.threads > 0)
		{
			if (UseTP2)
			{
				size_t iidx = 0;
				for (auto& e : p2results)
				{
					iidx++;
					if ((iidx % 10) == 0 || iidx == p2results.size())
						wprintf(L"\rCompleting item %zu/%zu  ...", iidx, p2results.size());

					e.get();
				}
				p2results.clear();
			}
			else
				p.Join();
		}
		wprintf(L"\r\n");


		t.Close();
		incrs.clear();
		if (sw.T == 0 && EndProcess == 0 && NeedUpdate > 0)
		{

			if (Folder)
			{
				// Create an incremental file there
				ystring fi;
				SYSTEMTIME sT;
				GetLocalTime(&sT);

				if (Remote == 2)
				{
					fi.Format(L"INCR_%04u%02u%02u%02u%02u%02u_%04u%02u%02u%02u%02u%02u.bar",
						sL.wYear, sL.wMonth, sL.wDay, sL.wHour, sL.wMinute, sL.wSecond,
						sT.wYear, sT.wMonth, sT.wDay, sT.wHour, sT.wMinute, sT.wSecond);
					OneUpload(one, t.fn.c_str(), ffid.a_str(), fi);
					if (sw.keep.length())
					{
						ystring te = sw.keep;
						te += L"\\";
						te += fi;
						if (!MoveFile(t.fn.c_str(), te.c_str()))
							wcout << L"File cannot be kept to " << sw.keep << endl;
					}
					else
						t.Delete();
					return;
				}
				if (Remote == 1)
				{
					fi.Format(L"INCR_%04u%02u%02u%02u%02u%02u_%04u%02u%02u%02u%02u%02u.bar",
						sL.wYear, sL.wMonth, sL.wDay, sL.wHour, sL.wMinute, sL.wSecond,
						sT.wYear, sT.wMonth, sT.wDay, sT.wHour, sT.wMinute, sT.wSecond);
					GooUpload(goo, t.fn.c_str(), ffid.a_str(), fi);
					if (sw.keep.length())
					{
						ystring te = sw.keep;
						te += L"\\";
						te += fi;
						if (!MoveFile(t.fn.c_str(), te.c_str()))
							wcout << L"File cannot be kept to " << sw.keep << endl;
					}
					else
						t.Delete();
					return;
				}

				fi.Format(L"\\INCR_%04u%02u%02u%02u%02u%02u_%04u%02u%02u%02u%02u%02u.bar",
					sL.wYear, sL.wMonth, sL.wDay, sL.wHour, sL.wMinute, sL.wSecond,
					sT.wYear,sT.wMonth,sT.wDay,sT.wHour,sT.wMinute,sT.wSecond);
				files[0] += fi;
			}

			BOOL R = MoveFileWithProgress(t.fn.c_str(), files[0].c_str(),
				[](LARGE_INTEGER TotalFileSize,
					LARGE_INTEGER TotalBytesTransferred,
					LARGE_INTEGER StreamSize,
					LARGE_INTEGER StreamBytesTransferred,
					DWORD dwStreamNumber,
					DWORD dwCallbackReason,
					HANDLE hSourceFile,
					HANDLE hDestinationFile,
					LPVOID lpData) -> DWORD
				{
					if (EndProcess)
						return PROGRESS_CANCEL;
					TotalBytesTransferred.QuadPart *= 100;
					TotalBytesTransferred.QuadPart /= TotalFileSize.QuadPart;
					wprintf(L"\rSaving file %llu%%...", TotalBytesTransferred.QuadPart);
					return PROGRESS_CONTINUE;
				}, 0,
				MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
			wprintf(L"\r\n");

			t.Delete();
			if (!R)
				FailWrite(E_FAIL);
		}
	}



	catch (...)
	{

	}

	if (Folder)
	{
		for (auto& a : sw.incr)
			DeleteFile(a.c_str());
	}

	t.Delete();
	if (ccx)
		ccx->r();
	ccx = nullptr;

}
