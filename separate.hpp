#pragma once


RGF::GOD::DATA fulldata;
unsigned long long fdf = 0;
bool ReadFromDownload(RGF::GOD::DRIVE* drv, char* put, DWORD sz, LPDWORD A)
{

	if (EndProcess)
		return false;

	*A = 0;
	if (fulldata.size() >= sz)
	{
		memcpy(put, fulldata.data(), sz);
		*A = sz;
		fulldata.erase(fulldata.begin(), fulldata.begin() + sz);
		return true;
	}

	// Download 1 MB
	if (auto one = dynamic_cast<RGF::GOD::ONEDRIVE*>(drv))
	{
		RGF::GOD::DATA d;
		auto hr = one->Download(files[0].a_str(), 0, &d, fdf, fdf + (1024 * 1024)*5);
		if (FAILED(hr))
			return false;
		if (d.size() == 0)
		{
			*A = 0;
			return true;
		}
		printf("-");
		fdf += d.size();
		fulldata.insert(fulldata.end(), d.begin(), d.end());
		return ReadFromDownload(drv, put, sz, A);
	}

	// Download 1 MB
	if (auto goo = dynamic_cast<RGF::GOD::GOOGLEDRIVE*>(drv))
	{
		RGF::GOD::DATA d;
		auto hr = goo->Download(files[0].a_str(), 0, &d, fdf, fdf + (1024 * 1024) * 5);
		if (FAILED(hr))
			return false;
		if (d.size() == 0)
		{
			*A = 0;
			return true;
		}
		printf("-");
		fdf += d.size();
		fulldata.insert(fulldata.end(), d.begin(), d.end());
		return ReadFromDownload(drv, put, sz, A);
	}

	return false;
}

HRESULT Separate2(RGF::GOD::DRIVE* drv = 0)
{
	using namespace std;
	if (!drv)
		return E_FAIL;


	try
	{

		// Write header
		TEMPFILE t;
		HEADER t1;
		auto hrw = t.Write((char*)& t1, sizeof(t1));
		FailWrite(hrw);

		unsigned long long totalread = 0;
		vector<char> ds(sizeof(TAG));
		BOOL rd = 0;
		//		map<unsigned int, tuple<ITEMMETA, ystring>> f;
		for (;;)
		{
			TAG* tt = (TAG*)ds.data();
			DWORD A = 0;

			rd = ReadFromDownload(drv,(char*)tt, sizeof(TAG), &A);
			if (!rd)
				throw;
			if (A == 0)
				break;
			totalread += A;

			auto ts = tt->size - sizeof(TAG);
			ds.resize(tt->size);
			tt = (TAG*)ds.data();


			rd = ReadFromDownload(drv, (char*)(ds.data() + sizeof(TAG)), (DWORD)ts, &A);
			if (!rd)
				throw;

			if (A != ts)
				throw;
			totalread += A;

			if (tt->code == (unsigned int)TAGCODE::HEADER)
				continue;

			if (tt->code == (unsigned int)TAGCODE::ITEMMETA)
			{
				ITEMMETA* m = (ITEMMETA*)tt;
				char* fd = ((char*)m) + sizeof(ITEMMETA);
				ystring fi = fd;
				//				f[m->fid] = make_tuple<ITEMMETA, ystring>(std::forward<ITEMMETA>(*m), forward<ystring>(fi));

								// Add the item
				auto hrw = t.Write((const char*)m, sizeof(ITEMMETA));
				FailWrite(hrw);

				// And the file name
				auto a = fi.a_str();
				hrw = t.Write(a, strlen(a) + 1);
				FailWrite(hrw);
				continue;
			}

			if (tt->code == (unsigned int)TAGCODE::ITEMDATA)
			{
				ITEMDATA* m = (ITEMDATA*)tt;
				if (m->dtt == (unsigned int)DATATYPE::DATA)
				{
					printf("\r\n");
					break;
				}

				if (m->dtt == (unsigned int)DATATYPE::SIG)
				{
					auto hrw = t.Write((const char*)m, sizeof(ITEMDATA));
					FailWrite(hrw);

					// And the data
					hrw = t.Write((const char*)(ds.data() + sizeof(ITEMDATA)), m->follows);
					FailWrite(hrw);
				}
			}
		}

		t.Close();
		if (sw.T == 0 && EndProcess == 0)
		{
			BOOL R = MoveFileEx(t.fn.c_str(), files[1].c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
			if (!R)
			{
				t.Delete();
				FailWrite(E_FAIL);
			}
		}
		fdf = 0;
		fulldata.clear();
		printf("\r\n");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


HRESULT Separate(RGF::GOD::DRIVE* drv = 0)
{
	using namespace std;
	if (drv)
		return Separate2(drv);
	if (files.size() != 2)
	{
		Help(0, "z");
		return E_INVALIDARG;
	}

	TEMPFILE t;
	CONSOLECOLOR cc;

	shared_ihandle hI1, hI2;



	shared_handle hX = CreateFile(files[0].c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (hX == INVALID_HANDLE_VALUE)
	{
		// Check URL

		hI1 = InternetOpen(L"BAR Archiver", INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
		hI2 = InternetOpenUrl(hI1, files[0].c_str(), 0, 0, 0, 0);
		if (!hI2)
		{
			cc.Color(FOREGROUND_RED | FOREGROUND_INTENSITY);
			wcout << L"Cannot open " << files[0].c_str() << endl;
			return E_UNEXPECTED;
		}
	}

	try
	{

		// Write header
		HEADER t1;
		auto hrw = t.Write((char*)& t1, sizeof(t1));
		FailWrite(hrw);

		unsigned long long totalread = 0;
		vector<char> ds(sizeof(TAG));
		BOOL rd = 0;
//		map<unsigned int, tuple<ITEMMETA, ystring>> f;
		for (;;)
		{
			TAG* tt = (TAG*)ds.data();
			DWORD A = 0;
			if (hI2)
				rd = InternetReadFile(hI2, (void*)tt, sizeof(TAG), &A);
			else
				rd = ReadFile(hX, (void*)tt, sizeof(TAG), &A, 0);
			
			if (!rd)
				throw;
			if (A == 0)
				break;
			totalread += A;

			auto ts = tt->size - sizeof(TAG);
			ds.resize(tt->size);
			tt = (TAG*)ds.data();


			if (hI2)
				rd = InternetReadFile(hI2, (void*)(ds.data() + sizeof(TAG)),(DWORD) ts, &A);
			else
				rd = ReadFile(hX, (void*)(ds.data() + sizeof(TAG)), (DWORD)ts, &A, 0);
			if (!rd)
				throw;

			if (A != ts)
				throw;
			totalread += A;

			if (tt->code == (unsigned int)TAGCODE::HEADER)
				continue;

			if (tt->code == (unsigned int)TAGCODE::ITEMMETA)
			{
				ITEMMETA* m = (ITEMMETA*)tt;
				char* fd = ((char*)m) + sizeof(ITEMMETA);
				ystring fi = fd;
//				f[m->fid] = make_tuple<ITEMMETA, ystring>(std::forward<ITEMMETA>(*m), forward<ystring>(fi));

				// Add the item
				auto hrw = t.Write((const char*)m, sizeof(ITEMMETA));
				FailWrite(hrw);

				// And the file name
				auto a = fi.a_str();
				hrw = t.Write(a, strlen(a) + 1);
				FailWrite(hrw);
				continue;
			}

			if (tt->code == (unsigned int)TAGCODE::ITEMDATA)
			{
				ITEMDATA* m = (ITEMDATA*)tt;
				if (m->dtt == (unsigned int)DATATYPE::DATA)
					break;

				if (m->dtt == (unsigned int)DATATYPE::SIG)
				{
					auto hrw = t.Write((const char*)m, sizeof(ITEMDATA));
					FailWrite(hrw);

					// And the data
					hrw = t.Write((const char*)(ds.data() + sizeof(ITEMDATA)), m->follows);
					FailWrite(hrw);
				}
			}
		}

		t.Close();
		if (sw.T == 0 && EndProcess == 0)
		{
			BOOL R = MoveFileEx(t.fn.c_str(), files[1].c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
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
		cc.Color(FOREGROUND_RED | FOREGROUND_INTENSITY);
		return E_UNEXPECTED;
	}


}
