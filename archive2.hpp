#pragma once
#include "ThreadPool/ThreadPool.h"
bool UseTP2 = true;

std::shared_ptr<CONSOLECOLOR> ccx;
tlock<int> arlock;
volatile LONG64 jt = 0;
E ee;

class V
{
public:
	V()
	{
		InterlockedIncrement64(&jt);
	}
	~V()
	{
		InterlockedDecrement64(&jt);
	}
	static void W()
	{
		while (jt > 0)
			Sleep(100);
	}
};

void FailWrite(HRESULT hrw)
{
	if (FAILED(hrw))
	{
		if (ccx)
			ccx->Color(FOREGROUND_RED | FOREGROUND_INTENSITY);
		wprintf(L"Archive could not be written.\r\n");
		if (ccx)
		{
			ccx->r();
			ccx = 0;
		}
		throw;
	}
}


void CALLBACK CreateAndWriteHeader(PTP_CALLBACK_INSTANCE, PVOID vv, PTP_WORK)
{
	V vvv;
	if (EndProcess)
		return;
	FOUNDIT* f = (FOUNDIT*)vv;
	auto& fi = *f;

	ITEMMETA me;

	if (fi.Linked)
	{
		me = *(ITEMMETA*)fi.Linked->d.data();
	}
	else
	{
		me.attr = fi.w.dwFileAttributes;
		ULARGE_INTEGER li;
		li.HighPart = fi.w.nFileSizeHigh;
		li.LowPart = fi.w.nFileSizeLow;
		me.uncsize = li.QuadPart;
		me.cmpsize = 0;// d.size();

		if (fi.OpenMapper())
		{
			GetFileTime(fi.mapper->hF, (LPFILETIME)& me.t1, (LPFILETIME)& me.t2, (LPFILETIME)& me.t3);

			CRC c;
			void* map = fi.mapper->Map();
			me.unccrc = c.findcrc32((const char*)map, fi.mapper->fs());
			f->savedcrc = me.unccrc;
			fi.mapper->Unmap();
		}
		else
			Warnings->emplace_back(ystring().Format(L"Cannot open file %s",fi.full.c_str()));

		fi.CloseMapper();
		me.tag.size = sizeof(ITEMMETA) + strlen(fi.rel.a_str()) + 1;
	}

	arlock.writelock([&](int& fid)
		{
			me.fid = fi.fid;
			fi.tempfp = fi.t->fp(0);
			fi.tempmeta = me;
			auto hrw = fi.t->Write((const char*)& me, sizeof(me));
			FailWrite(hrw);

			// And the file name
			auto a = fi.rel.a_str();
			hrw = fi.t->Write(a, strlen(a) + 1);
			FailWrite(hrw);
		});
}


tlock<std::map<unsigned long, unsigned long>> fidcrc;
void CALLBACK CreateAndWriteData(PTP_CALLBACK_INSTANCE, PVOID vv, PTP_WORK w)
{
	V vvv;
	if (EndProcess)
		return;

	int algo = COMPRESS_ALGORITHM_LZMS;

	FOUNDIT* f = (FOUNDIT*)vv;
	if (w)
	{
		CoInitializeEx(0, COINIT_MULTITHREADED);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	}

	COMPRESSIONAPI c(algo);
	if (!f->OpenMapper())
	{
		Warnings->emplace_back(ystring().Format(L"Cannot open file %s", f->full.c_str()));
		return;
	}
	void* map = f->mapper->Map();
	std::vector<char> d;

	if (f->mapper->fs() == 0)
		f->nc = 1;

	if (map)
	{
		if (f->nc)
		{
			d.resize(f->mapper->fs());
			memcpy(d.data(), map, f->mapper->fs());
		}
		else
		{
			// Check dups
			if (f->savedcrc && sw.dup)
			{
				unsigned long efid = fidcrc->operator[](f->savedcrc);
				if (efid > 0)
				{
					algo = COMPRESS_ALGORITHM_DUP;
					d.resize(sizeof(efid));
					memcpy(d.data(), &efid, sizeof(efid));
					Dups++;
				}

				fidcrc->operator[](f->savedcrc) = f->fid;
			}
			if (algo != COMPRESS_ALGORITHM_DUP)
			{
				auto hr = c.CompressX((const char*)map, f->mapper->fs(), d);
				if (FAILED(hr))
				{
					f->nc = 1;
					d.resize(f->mapper->fs());
					memcpy(d.data(), map, f->mapper->fs());
				}
			}
		}
		if (sw.pwd.length())
		{
			std::vector<char> d2;
			if (!ee.Encrypt(d, d2))
				throw;
			d = d2;
		}
	}
	

	arlock.writelock([&](int& fid)
		{
			// The data
//			wprintf(L"Adding %s...\r\n", f->rel.c_str());
			ITEMDATA da;
			da.fid = f->fid;
			da.dtt = (unsigned int)DATATYPE::DATA;
			da.cmpt = (unsigned int)algo;
			if (f->nc)
				da.cmpt = (unsigned int)0;
			da.enc = 0;
			if (sw.pwd.length())
				da.enc = 1;
			da.tag.size += d.size();
			da.follows = d.size();
			auto hrw = f->t->Write((const char*)& da, sizeof(da));
			FailWrite(hrw);

			// And the data
			hrw = f->t->Write((const char*)d.data(), d.size());
			FailWrite(hrw);

			// Get fp
			auto fpc = f->t->fp(0);

			// And update meta
			f->t->fp(f->tempfp, true);
			if (true)
			{
				// 
				CRC c;
				f->tempmeta.cmpcrc = c.findcrc32((const char*)d.data(), d.size());
			}

			f->tempmeta.cmpsize = d.size();
			hrw = f->t->Write((const char*)& f->tempmeta, sizeof(f->tempmeta));
			FailWrite(hrw);

			// Set fp
			f->t->fp(fpc, true);

			// Free the mapper
			f->CloseMapper();
		});
	return;
};


void CALLBACK CreateAndWriteSignature(PTP_CALLBACK_INSTANCE, PVOID vv, PTP_WORK w)
{
	V vvv;
	if (EndProcess)
		return;
	FOUNDIT* f = (FOUNDIT*)vv;
	if (w)
	{
		CoInitializeEx(0, COINIT_MULTITHREADED);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	}

	DIFFLIB::DIFF diff;
	DIFFLIB::MemoryDiffWriter d;
	if (!f->OpenMapper())
	{
		Warnings->emplace_back(ystring().Format(L"Cannot open file %s", f->full.c_str()));
		return;
	}
	void* map = f->mapper->Map();
	DIFFLIB::MemoryRdcFileReader mr((const char*)map, f->mapper->fs());
	auto hr = diff.GenerateSignature(&mr, d);
	if (FAILED(hr))
	{
		ccx->Color(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
		Warnings->emplace_back(ystring().Format(L"Generating signature for %s failed", f->full.c_str()));
		return;
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
			da.fid = f->fid;
			da.dtt = (unsigned int)DATATYPE::SIG;
			da.cmpt = (unsigned int)DATACOMP::UNC;
			da.tag.size += d.sz();
			da.follows = d.sz();
			auto hrw = f->t->Write((const char*)& da, sizeof(da));
			FailWrite(hrw);

			// And the data
			hrw = f->t->Write((const char*)d.p().data(), d.sz());
			FailWrite(hrw);
		});

	if (f)
		f->CloseMapper();
	return;
};


HRESULT FindFileInTags(std::list<TAGDATA>& tags,std::vector<TAGDATA>& vtags,ystring& fi, ITEMMETAANDFILE& imf,FOUNDIT* CheckSame)
{
	using namespace std;

	TAGDATA td;
	td.fil = fi;
	auto foou = std::lower_bound(vtags.begin(), vtags.end(), td);
//	auto foou = std::find(tags.begin(), tags.end(), td);
	if (foou == vtags.end())
		return E_NOINTERFACE;

	TAG* tt = (TAG*)foou->d.data();
	if (tt->code == (unsigned int)TAGCODE::ITEMMETA)
	{
		ITEMMETA* m = (ITEMMETA*)tt;
		char* fn = (char*)(foou->d.data() + sizeof(ITEMMETA));
		ystring fz = fn;
		if (fz == fi)
		{
			foou->fnd = 1;
			imf.m = *m;
			imf.rel = fz;
			imf.td = *foou;
			imf.fid = m->fid;

			if (!CheckSame)
				return S_OK;

			// Size same?
			ULARGE_INTEGER li = { 0 };
			li.HighPart = CheckSame->w.nFileSizeHigh;
			li.LowPart = CheckSame->w.nFileSizeLow;
			if (m->uncsize != li.QuadPart)
				return S_FALSE;

			// CRC or time same?
			unsigned long long t3 = 0;
			if (!CheckSame->OpenMapper())
				return S_FALSE;
			GetFileTime(CheckSame->mapper->hF, 0, 0, (LPFILETIME)& t3);
			unsigned long tcrc = 0;
			if (true)
			{
				CRC c;
				void* map = CheckSame->mapper->Map();
				tcrc = c.findcrc32((const char*)map, CheckSame->mapper->fs());
			}

			bool Same = false;
			if (m->unccrc != 0 && m->unccrc == tcrc)
				Same = true;
			if (m->unccrc == 0)
			{
				// Check times
				if (m->t3 == t3)
					Same = true;
			}
			CheckSame->CloseMapper();

			if (!Same)
				return S_FALSE;
			return S_OK;
		}
	}
	return E_NOINTERFACE;
}


void BuildFileList(std::list<FOUNDIT>& items,tpoollib::tpool<>& pool)
{
	wprintf(L"Building file list...");
	using namespace std;
	for (size_t i = 1; i < files.size(); i++)
	{
		std::wstring fi2 = files[i];
		wstring pd;
		CheckFiles(fi2.c_str(), fi2.c_str(), 0, (LPARAM)& files,
			[&](const TCHAR* a1, const TCHAR* a2, WIN32_FIND_DATA* e, LPARAM, int dep) -> HRESULT
			{
				if (dep > 0 && !sw.R)
					return S_OK;

				if (EndProcess)
					return E_FAIL;

				ystring fi = a2;
				fi += L"\\";
				fi += e->cFileName;

				if (e->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					auto i = IncludeItem(fi.a_str(),true);
					if (!i)
						return S_FALSE;
					return S_OK;
				}

				// Not here, to threads
				// File
				auto i = IncludeItem(fi.a_str(),false);
				if (!i)
					return S_OK;

				// Check known extension
				bool NC = false;
				auto ext = PathFindExtension(fi.c_str());

				auto nexts = {L".jpg",L".rar",L".zip",L".png",L".mp3",L".mp4",L".7z"};
				for (auto& n : nexts)
				{
					if (ext && _wcsicmp(ext, n) == 0)
					{
						NC = true;
						break;
					}
				}


				FOUNDIT it;
				it.f = e->cFileName;
				it.full = fi;
				it.rel = fi.c_str() + wcslen(a1) + 1;
				it.w = *e;
				it.nc = NC;
				items.push_back(it);
				return S_OK;

			}
		, 0);
	}
	wprintf(L"sorting...");
	items.sort();
	wprintf(L"done.\r\n");
}

void Archive2()
{
	using namespace std;
	if (files.size() < 2)
	{
		Help(0, "a");
		return;
	}

	// files[0] = archive to create
	// files[...] = directories

	ccx = make_shared<CONSOLECOLOR>();
	TEMPFILE t;
//		void List(); List();

	if (sw.pwd.length())
	{
		if (!ee.Prepare(sw.pwd.a_str()))
		{
			throw 0;
		}
	}
		
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
		BuildFileList(items,p);

		// Existing file
		list<TAGDATA> tags;
		vector<TAGDATA> vtags;
		shared_ptr<MAPPER> me = make_shared<MAPPER>(files[0].c_str(), PAGE_READONLY);
		const char* omap = (const char*)me->Map();
		if (omap && !sw.O)
			TagCollect((const char*)omap, me->fs(), tags,vtags);

		// Find files already in old archive?
		if (!tags.empty())
		{
			for (auto& fi : items)
			{
				ITEMMETAANDFILE imf;
				auto hr = FindFileInTags(tags, vtags,fi.rel, imf, &fi);
				if (SUCCEEDED(hr))
				{
					if (hr == S_OK)
					{
						// Same
						fi.Linked = make_shared<TAGDATA>(imf.td);
					}
				}
			}

			// Old archive has more to add?
			for (auto& t : vtags)
			{
				TAG* tt = (TAG*)t.d.data();
				if (tt->code != (unsigned int)TAGCODE::ITEMMETA)
					continue;
				if (t.fnd == 1)
					continue;

				char* fn = (char*)(t.d.data() + sizeof(ITEMMETA));
				ystring fz = fn;

				// Add this existing to the items, it's not found in the new file specs
				FOUNDIT fit;
				fit.rel = fz;
				fit.Linked = make_shared<TAGDATA>(t);
				items.push_back(fit);
			}

		}

		// Create the file entries
		unsigned int fid = 0;
		auto itemss = items.size();
		size_t iidx = 0;
		for (auto& fi : items)
		{
			if (EndProcess)
				break;
			iidx++;
			if ((iidx % 10) == 0 || iidx == itemss)
				wprintf(L"\rPreparing items %zu of %zu ...", iidx, itemss);
			fi.t = &t;
			

			fid++;
			fi.fid = fid;

			if (sw.threads == 0)
				CreateAndWriteHeader(0, (PVOID)&fi, 0);
			else
			{
				if (UseTP2)
				{
					p2results.emplace_back(p2.enqueue([&]() -> int {
						CreateAndWriteHeader(0, (PVOID)& fi,(PTP_WORK)1);
						return 1;
						}));
				}
				else
				{
					auto workit = p.CreateItem<PTP_WORK, PTP_WORK_CALLBACK>(CreateAndWriteHeader, (PVOID)& fi);
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
		V::W();
		wprintf(L"\r\n");


		// Put the signatures
		if (sw.sigs)
		{

			// Create signatures on top
			auto itemss = items.size();
			size_t iidx = 0;
			for (auto& fi : items)
			{
				if (EndProcess)
					break;
				iidx++;
				if ((iidx % 10) == 0 || iidx == itemss)
					wprintf(L"\rCreating signature %zu of %zu ...", iidx, itemss);
				fi.t = &t;

				// Check existing
				bool ExistingSig = false;
				if (omap && fi.Linked)
				{
					for (auto& aa : fi.Linked->data)
					{
						if (aa.type == (unsigned int)DATATYPE::SIG)
						{
							// The data
							ITEMDATA da;
							da.fid = fi.fid;
							da.dtt = aa.type;
							da.cmpt = aa.ctype;
							da.enc = aa.etype;
							da.tag.size += aa.s;
							da.follows = aa.s;
							auto hrw = fi.t->Write((const char*)& da, sizeof(da));
							FailWrite(hrw);

							// And the data
							hrw = fi.t->Write((const char*)omap + aa.ofs, aa.s);
							FailWrite(hrw);

							ExistingSig = true;
							break;
						}
					}
				}

				if (ExistingSig)
					continue;


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
					size_t iidx = 0;
					for (auto& e : p2results)
					{
//						if (EndProcess)
	//						break;
						e.get();
						iidx++;
//						if ((iidx % 10) == 0 || iidx == itemss)
//							wprintf(L"\rCompleting item %zu/%zu  ...", iidx, itemss);
					}
					p2results.clear();
				}
				else
					p.Join();
			}
			V::W();
			wprintf(L"\r\n");
		}



		// And the data
		itemss = items.size();
		iidx = 0;
		for (auto& fi : items)
		{
			if (EndProcess)
				break;
			iidx++;
			if ((iidx % 10) == 0 || iidx == itemss)
				wprintf(L"\rAdding item %zu/%zu  ...", iidx, itemss);
			fi.t = &t;

			// Check existing
			bool ExistingData = false;
			if (omap && fi.Linked)
			{
				for (auto& aa : fi.Linked->data)
				{
					if (aa.type == (unsigned int)DATATYPE::DATA)
					{
						// The data
						ITEMDATA da;
						da.fid = fi.fid;
						da.dtt = aa.type;
						da.cmpt = aa.ctype;
						da.enc = aa.etype;
						da.tag.size += aa.s;
						da.follows = aa.s;
						auto hrw = fi.t->Write((const char*)& da, sizeof(da));
						FailWrite(hrw);

						// And the data
						hrw = fi.t->Write((const char*)omap + aa.ofs, aa.s);
						FailWrite(hrw);

						ExistingData = true;

						// Free the mapper
						fi.mapper = nullptr;
						break;
					}
				}
			}

			if (ExistingData)
				continue;

			if (sw.threads == 0)
				CreateAndWriteData(0, (PVOID)&fi, 0);
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
		if (sw.threads > 0)
		{
			if (UseTP2)
			{
				size_t iidx = 0;
				for (auto& e : p2results)
				{
					//						if (EndProcess)
						//						break;
					e.get();
					iidx++;
					if ((iidx % 10) == 0 || iidx == itemss)
						wprintf(L"\rAdding item %zu/%zu  ...", iidx, itemss);
				}
				p2results.clear();
			}
			else
				p.Join();
		}
		V::W();
		wprintf(L"\r\n");

		if (omap)
			me = nullptr;
		t.Close();

		if (sw.T == 0 && EndProcess == 0)
		{
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
				},0,
				MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
			wprintf(L"\r\n");

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
	if (ccx)
		ccx->r();
	ccx = nullptr;
	t.Close();
	t.Delete();
}