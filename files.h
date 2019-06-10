#pragma once



// CRC-32 class
static const unsigned int crctable[256] = {
	0x00000000U,0x04C11DB7U,0x09823B6EU,0x0D4326D9U,
	0x130476DCU,0x17C56B6BU,0x1A864DB2U,0x1E475005U,
	0x2608EDB8U,0x22C9F00FU,0x2F8AD6D6U,0x2B4BCB61U,
	0x350C9B64U,0x31CD86D3U,0x3C8EA00AU,0x384FBDBDU,
	0x4C11DB70U,0x48D0C6C7U,0x4593E01EU,0x4152FDA9U,
	0x5F15ADACU,0x5BD4B01BU,0x569796C2U,0x52568B75U,
	0x6A1936C8U,0x6ED82B7FU,0x639B0DA6U,0x675A1011U,
	0x791D4014U,0x7DDC5DA3U,0x709F7B7AU,0x745E66CDU,
	0x9823B6E0U,0x9CE2AB57U,0x91A18D8EU,0x95609039U,
	0x8B27C03CU,0x8FE6DD8BU,0x82A5FB52U,0x8664E6E5U,
	0xBE2B5B58U,0xBAEA46EFU,0xB7A96036U,0xB3687D81U,
	0xAD2F2D84U,0xA9EE3033U,0xA4AD16EAU,0xA06C0B5DU,
	0xD4326D90U,0xD0F37027U,0xDDB056FEU,0xD9714B49U,
	0xC7361B4CU,0xC3F706FBU,0xCEB42022U,0xCA753D95U,
	0xF23A8028U,0xF6FB9D9FU,0xFBB8BB46U,0xFF79A6F1U,
	0xE13EF6F4U,0xE5FFEB43U,0xE8BCCD9AU,0xEC7DD02DU,
	0x34867077U,0x30476DC0U,0x3D044B19U,0x39C556AEU,
	0x278206ABU,0x23431B1CU,0x2E003DC5U,0x2AC12072U,
	0x128E9DCFU,0x164F8078U,0x1B0CA6A1U,0x1FCDBB16U,
	0x018AEB13U,0x054BF6A4U,0x0808D07DU,0x0CC9CDCAU,
	0x7897AB07U,0x7C56B6B0U,0x71159069U,0x75D48DDEU,
	0x6B93DDDBU,0x6F52C06CU,0x6211E6B5U,0x66D0FB02U,
	0x5E9F46BFU,0x5A5E5B08U,0x571D7DD1U,0x53DC6066U,
	0x4D9B3063U,0x495A2DD4U,0x44190B0DU,0x40D816BAU,
	0xACA5C697U,0xA864DB20U,0xA527FDF9U,0xA1E6E04EU,
	0xBFA1B04BU,0xBB60ADFCU,0xB6238B25U,0xB2E29692U,
	0x8AAD2B2FU,0x8E6C3698U,0x832F1041U,0x87EE0DF6U,
	0x99A95DF3U,0x9D684044U,0x902B669DU,0x94EA7B2AU,
	0xE0B41DE7U,0xE4750050U,0xE9362689U,0xEDF73B3EU,
	0xF3B06B3BU,0xF771768CU,0xFA325055U,0xFEF34DE2U,
	0xC6BCF05FU,0xC27DEDE8U,0xCF3ECB31U,0xCBFFD686U,
	0xD5B88683U,0xD1799B34U,0xDC3ABDEDU,0xD8FBA05AU,
	0x690CE0EEU,0x6DCDFD59U,0x608EDB80U,0x644FC637U,
	0x7A089632U,0x7EC98B85U,0x738AAD5CU,0x774BB0EBU,
	0x4F040D56U,0x4BC510E1U,0x46863638U,0x42472B8FU,
	0x5C007B8AU,0x58C1663DU,0x558240E4U,0x51435D53U,
	0x251D3B9EU,0x21DC2629U,0x2C9F00F0U,0x285E1D47U,
	0x36194D42U,0x32D850F5U,0x3F9B762CU,0x3B5A6B9BU,
	0x0315D626U,0x07D4CB91U,0x0A97ED48U,0x0E56F0FFU,
	0x1011A0FAU,0x14D0BD4DU,0x19939B94U,0x1D528623U,
	0xF12F560EU,0xF5EE4BB9U,0xF8AD6D60U,0xFC6C70D7U,
	0xE22B20D2U,0xE6EA3D65U,0xEBA91BBCU,0xEF68060BU,
	0xD727BBB6U,0xD3E6A601U,0xDEA580D8U,0xDA649D6FU,
	0xC423CD6AU,0xC0E2D0DDU,0xCDA1F604U,0xC960EBB3U,
	0xBD3E8D7EU,0xB9FF90C9U,0xB4BCB610U,0xB07DABA7U,
	0xAE3AFBA2U,0xAAFBE615U,0xA7B8C0CCU,0xA379DD7BU,
	0x9B3660C6U,0x9FF77D71U,0x92B45BA8U,0x9675461FU,
	0x8832161AU,0x8CF30BADU,0x81B02D74U,0x857130C3U,
	0x5D8A9099U,0x594B8D2EU,0x5408ABF7U,0x50C9B640U,
	0x4E8EE645U,0x4A4FFBF2U,0x470CDD2BU,0x43CDC09CU,
	0x7B827D21U,0x7F436096U,0x7200464FU,0x76C15BF8U,
	0x68860BFDU,0x6C47164AU,0x61043093U,0x65C52D24U,
	0x119B4BE9U,0x155A565EU,0x18197087U,0x1CD86D30U,
	0x029F3D35U,0x065E2082U,0x0B1D065BU,0x0FDC1BECU,
	0x3793A651U,0x3352BBE6U,0x3E119D3FU,0x3AD08088U,
	0x2497D08DU,0x2056CD3AU,0x2D15EBE3U,0x29D4F654U,
	0xC5A92679U,0xC1683BCEU,0xCC2B1D17U,0xC8EA00A0U,
	0xD6AD50A5U,0xD26C4D12U,0xDF2F6BCBU,0xDBEE767CU,
	0xE3A1CBC1U,0xE760D676U,0xEA23F0AFU,0xEEE2ED18U,
	0xF0A5BD1DU,0xF464A0AAU,0xF9278673U,0xFDE69BC4U,
	0x89B8FD09U,0x8D79E0BEU,0x803AC667U,0x84FBDBD0U,
	0x9ABC8BD5U,0x9E7D9662U,0x933EB0BBU,0x97FFAD0CU,
	0xAFB010B1U,0xAB710D06U,0xA6322BDFU,0xA2F33668U,
	0xBCB4666DU,0xB8757BDAU,0xB5365D03U,0xB1F740B4U,
};

class CRC
{
private:



public:

	unsigned long findcrc32(const char* x, const size_t length, const unsigned long startvalue = (unsigned long)-1)
	{
		unsigned long y = startvalue;
		for (size_t i = 0; i < length; i++)
			y = (y >> 8) ^ crctable[((unsigned long)x[i] ^ y) & 0xFF];
		return y;
	}



};


void CheckFiles(const TCHAR* VeryBasePath, const TCHAR* pszPath, TCHAR* pszBase,
	LPARAM data, std::function<HRESULT(const TCHAR*, const TCHAR*, WIN32_FIND_DATA*, LPARAM, int)> HandleFile, int deep)
{
	WIN32_FIND_DATA w32fd;
	HANDLE hFind;
	DWORD dwAtt;
	TCHAR acPath[MAX_PATH];
	TCHAR acBase[MAX_PATH];

	if ('.' == *(pszPath + _tcslen(pszPath) - 1))
		return;

	if (pszBase)
		_stprintf_s(acPath, MAX_PATH, _T("%s\\%s"), pszBase, pszPath);
	else
		_tcscpy_s(acPath, MAX_PATH, pszPath);

	_tcscpy_s(acBase, MAX_PATH, acPath);

	dwAtt = GetFileAttributes(acPath);

	if (0xffffffff == dwAtt)
		// error ...
		return;

	if ((FILE_ATTRIBUTE_DIRECTORY & dwAtt) != 0)
	{
		if ('\\' == acPath[_tcslen(acPath) - 1])
			_tcscat_s(acPath, MAX_PATH, _T("*.*"));
		else
			_tcscat_s(acPath, MAX_PATH, _T("\\*.*"));
	}

	hFind = FindFirstFile(acPath, &w32fd);

	if (hFind == INVALID_HANDLE_VALUE)
		return;

	// recurse if directory...
	if ((FILE_ATTRIBUTE_DIRECTORY & w32fd.dwFileAttributes) != 0 && deep != -2)
	{
		auto res = HandleFile(VeryBasePath, acBase, &w32fd, data, deep);
		if (res == S_OK)
			CheckFiles(VeryBasePath, w32fd.cFileName, acBase, data, HandleFile,
				deep + 1);
	}
	else
		HandleFile(VeryBasePath, acBase, &w32fd, data, deep);

	while (FindNextFile(hFind, &w32fd))
	{
		// recurse if directory...
		if ((FILE_ATTRIBUTE_DIRECTORY & w32fd.dwFileAttributes) != 0 && deep != -2)
		{
			auto res = HandleFile(VeryBasePath, acBase, &w32fd, data, deep);
			if (res == S_OK)
				CheckFiles(VeryBasePath, w32fd.cFileName, acBase, data, HandleFile,
				deep + 1);
		}
		else
			HandleFile(VeryBasePath, acBase, &w32fd, data, deep);
	}

	FindClose(hFind);
}

bool MatchWildcard(const wchar_t* trg, const wchar_t* wild,std::wregex& wr,bool UseWR = false)
{
	using namespace std;
	if (!wild || !trg)
		return false;
	wcmatch res;

	if (UseWR)
	{
		try
		{
			regex_search(trg, res, wr);
			if (res.empty())
				return false;
			return true;
		}
		catch (...)
		{
			return false;
		}
	}


	// say, *.cpp -> ^.*\.cpp$
	std::wstring r;
	for (size_t i = 0; i < wcslen(wild); i++)
	{
		if (wild[i] == 0)
			break;

		if (wild[i] == '\\')
			r += L"\\\\";
		else
		if (wild[i] == '?')
			r += L".?";
		else
		if (wild[i] == '*')
			r += L".*";
		else
		if (wild[i] == '.')
			r += L"\\.";
		else
			r += wild[i];
	}
	try
	{
		wregex rx(r, std::regex::icase);
		wr = rx;
		regex_search(trg, res, rx);
		if (res.empty())
			return false;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

std::map<std::wstring, std::wregex> wexcludemap;
std::map<std::wstring, std::wregex> wincludemap;
bool IncludeItem(const char* fn,bool D)
{
	using namespace std;
	const char* sp = strchr(fn, '\\');
	if (sp)
		sp++;
	else
		sp = fn;

	bool MatchI = true;

	// If there are exclude wildcard expressions, exclusion match --> remove
	for (auto& a : sw.X)
	{
		bool HasS = strchr(a.c_str(), '\\') != 0;
		if (VMatching(HasS ? fn : sp, a.c_str()))
			return false;
/*
		bool UseC = false;
		if (wexcludemap.find(a) != wexcludemap.end())
			UseC = true;

		bool HasS = wcschr(a.c_str(), '\\') != 0;
		auto& rex = wexcludemap[a];
		if (MatchWildcard(HasS ? fn : sp, a.c_str(),rex,UseC))
			return false;
*/	}

	// If there are include wildcard expressions, at least one of them must be matched
	for (auto& a : sw.I)
	{
		if (D)
			break; // Always include directories

		bool HasS = strchr(a.c_str(), '\\') != 0;
		MatchI = false;
		if (VMatching(HasS ? fn : sp,a.c_str()))
		{
			MatchI = true;
			break;
		}

	}

	
	
/*	for (auto& a : sw.I)
	{
		bool UseC = false;
		if (wincludemap.find(a) != wincludemap.end())
			UseC = true;

		bool HasS = wcschr(a.c_str(), '\\') != 0;
		MatchI = false;
		auto& rex = wincludemap[a];
		if (MatchWildcard(HasS ? fn : sp, a.c_str(),rex,UseC))
		{
			MatchI = true;
			break;
		}

	}
	*/

	if (!MatchI)
		return false;
	
/*	// If there are regular expressions, exclusion match -->remove
	for (auto& a : sw.XR)
	{
		bool HasS = wcschr(a.c_str(), '\\') != 0;
		regex rx(ystring(a.c_str()).a_str());
		cmatch res;
		regex_search(ystring(HasS ? fn : sp).a_str(), res, rx);
		if (!res.empty())
			return false;
	}

	// If there are include regular expressions, at least one of them must match
	MatchI = true;
	for (auto& a : sw.IR)
	{
		bool HasS = wcschr(a.c_str(), '\\') != 0;
		MatchI = false;
		regex rx(ystring(a.c_str()).a_str());
		cmatch res;
		regex_search(ystring(HasS ? fn : sp).a_str(), res, rx);
		if (!res.empty())
		{
			MatchI = true;
			break;
		}
	}
	if (!MatchI)
		return false;
		*/

	return true;
}



class shared_handle
{
private:
	HANDLE hX = INVALID_HANDLE_VALUE;
	std::shared_ptr<size_t> ptr = std::make_shared<size_t>();

public:

	// Closing items
	void Close()
	{
		if (!ptr || ptr.use_count() > 1)
		{
			ptr.reset();
			return;
		}
		ptr.reset();
		CloseHandle(hX);
		hX = INVALID_HANDLE_VALUE;
	}

	shared_handle()
	{
		hX = INVALID_HANDLE_VALUE;
	}
	~shared_handle()
	{
		Close();
	}
	shared_handle(const shared_handle & h)
	{
		Dup(h);
	}
	shared_handle(shared_handle && h)
	{
		Move(std::forward<shared_handle>(h));
	}
	shared_handle(HANDLE hY)
	{
		hX = hY;
	}
	shared_handle& operator =(const shared_handle & h)
	{
		Dup(h);
		return *this;
	}
	shared_handle& operator =(shared_handle && h)
	{
		Move(std::forward<shared_handle>(h));
		return *this;
	}

	void Dup(const shared_handle & h)
	{
		Close();
		hX = h.hX;
		ptr = h.ptr;
	}
	void Move(shared_handle && h)
	{
		Close();
		hX = h.hX;
		ptr = h.ptr;
		h.ptr.reset();
		h.hX = INVALID_HANDLE_VALUE;
	}
	operator HANDLE() const
	{
		return hX;
	}
};

class shared_ihandle
{
private:
	HINTERNET hI = 0;
	std::shared_ptr<size_t> ptr = std::make_shared<size_t>();

public:

	// Closing items
	void Close()
	{
		if (!ptr || ptr.use_count() > 1)
		{
			ptr.reset();
			return;
		}
		ptr.reset();
		InternetCloseHandle(hI);
		hI = 0;
	}

	shared_ihandle()
	{
		hI = 0;
	}
	~shared_ihandle()
	{
		Close();
	}
	shared_ihandle(const shared_ihandle& h) = delete;
	shared_ihandle(shared_ihandle&& h)
	{
		Move(std::forward<shared_ihandle>(h));
	}
	shared_ihandle(HINTERNET hY)
	{
		hI = hY;
	}
	shared_ihandle& operator =(const shared_ihandle& h) = delete;
	shared_ihandle& operator =(shared_ihandle&& h)
	{
		Move(std::forward<shared_ihandle>(h));
		return *this;
	}

	void Move(shared_ihandle&& h)
	{
		Close();
		hI = h.hI;
		ptr = h.ptr;
		h.ptr.reset();
		h.hI = 0;
	}
	operator HINTERNET() const
	{
		return hI;
	}
};


struct FMAP
{
	void* p = 0;

	FMAP(HANDLE hM, int pf = FILE_MAP_WRITE)
	{
		p = MapViewOfFile(hM, pf, 0, 0, 0);
	}

	void Unmap()
	{
		if (p)
			UnmapViewOfFile(p);
		p = 0;

	}
	~FMAP()
	{
		Unmap();
	}
};

class TEMPFILE
{
public:

	std::wstring fn;
	shared_handle hF;

	void Close()
	{
		hF.Close();
	}

	void Delete()
	{
		DeleteFile(fn.c_str());
	}

	unsigned long long fp(unsigned long long np,bool S = false)
	{
		LARGE_INTEGER li = { 0 };
		if (S)
		{
			li.QuadPart = np;
			SetFilePointerEx(hF, li, &li, FILE_BEGIN);
		}
		else
			SetFilePointerEx(hF, li, &li, FILE_CURRENT);
		return li.QuadPart;
	}

	HRESULT Write(const char* a, unsigned long long s)
	{
		unsigned long long gb3 = 3ULL * 1024ULL * 1024ULL * 1024ULL;
		if (!a || !s)
			return S_FALSE;
		for (;;)
		{
			DWORD s2 = (DWORD)s;
			if (s > gb3)
				s2 = (DWORD)gb3;
			DWORD A = 0;
			BOOL f = WriteFile(hF, a, s2, &A, 0);
			if (!f)
				return E_FAIL;
			if (A != s2)
				return E_FAIL;
			s -= s2;
			if (s == 0)
				break;
		}
		return S_OK;
	}

	TEMPFILE(const wchar_t* pfx = L"tmp")
	{
		std::vector<wchar_t> td(1000);
		GetTempPathW(1000, td.data());
		std::wstring tdir = td.data();
		tdir += L"\\bar";
		if (!CreateDirectory(tdir.c_str(), 0))
		{
			if (GetLastError() != ERROR_ALREADY_EXISTS)
			{
				tdir = td.data();
			}
		}
		std::vector<wchar_t> tf(1000);
		GetTempFileName(tdir.c_str(), pfx, 0, tf.data());
		DeleteFile(tf.data());
		fn = tf.data();

		hF = CreateFile(fn.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (hF == INVALID_HANDLE_VALUE)
			throw;
	}

	unsigned long long fs()
	{
		LARGE_INTEGER lu = { 0 };
		GetFileSizeEx(hF, &lu);
		return lu.QuadPart;
	}

	shared_handle hMap;
	std::shared_ptr<FMAP> fmp;
	void* Map()
	{
		if (hMap != INVALID_HANDLE_VALUE)
			return fmp->p;
		hMap = CreateFileMapping(hF, 0, PAGE_READWRITE, 0, 0, 0);
		if (hMap != INVALID_HANDLE_VALUE)
			fmp = std::make_shared<FMAP>(hMap);
		return fmp->p;
	}

	void Unmap()
	{
		if (hMap == INVALID_HANDLE_VALUE)
			return;

		fmp->Unmap();
		hMap.Close();
	}
};


struct MAPPER
{
	shared_handle hMap;
	std::shared_ptr<FMAP> fmp;


	MAPPER(const MAPPER&) = delete;
	MAPPER& operator =(const MAPPER&) = delete;

	MAPPER(MAPPER&&m)
	{
		operator =(std::forward<MAPPER>(m));
	}
	MAPPER& operator =(MAPPER&& m)
	{
		ch = m.ch;
		pgg = m.pgg;
		hF = m.hF;
		fmp = m.fmp;
		m.hF = INVALID_HANDLE_VALUE;
		m.fmp = 0;
		return *this;
	}

	HANDLE hF = INVALID_HANDLE_VALUE;
	int pgg;
	bool ch = false;
	MAPPER(HANDLE hX, int pg = PAGE_READONLY)
	{
		hF = hX;
		pgg = pg;
		ch = false;
	}

	MAPPER(const wchar_t* f, int pg = PAGE_READONLY)
	{
		if (pg == PAGE_READWRITE)
			hF = CreateFile(f, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
		else
			hF = CreateFile(f, GENERIC_READ,FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		pgg = pg;
		ch = true;
	}

	unsigned long long fs()
	{
		LARGE_INTEGER lu = { 0 };
		GetFileSizeEx(hF, &lu);
		return lu.QuadPart;
	}


	void* Map()
	{
		if (hMap != INVALID_HANDLE_VALUE && fmp)
			return fmp->p;
		hMap = CreateFileMapping(hF, 0, pgg, 0, 0, 0);
		if (hMap != INVALID_HANDLE_VALUE && hMap != 0)
			fmp = std::make_shared<FMAP>(hMap, pgg == PAGE_READWRITE ? FILE_MAP_WRITE : FILE_MAP_READ);
		if (!fmp)
			return 0;
		return fmp->p;
	}

	void Unmap()
	{
		if (hMap == INVALID_HANDLE_VALUE)
			return;

		if (fmp)
			fmp->Unmap();
		hMap.Close();
	}

	~MAPPER()
	{
		Unmap();
		if (ch)
			CloseHandle(hF);
		hF = INVALID_HANDLE_VALUE;
	}
};


struct FOUNDIT
{
	ystring full;
	ystring rel;
	ystring f;
	WIN32_FIND_DATA w = { 0 };
	TEMPFILE* t = 0;
	unsigned int fid = 0;
	unsigned long long tempfp = 0;
	ITEMMETA tempmeta;
	bool nc = false;
	std::shared_ptr<MAPPER> mapper;
	unsigned long savedcrc = 0; // for dups

	std::shared_ptr<TAGDATA> Linked;
	size_t incridx = 0;
	std::shared_ptr<TAGDATA> LinkForDiff;
	unsigned int linkfid = 0;
	HRESULT diffsearch = E_NOINTERFACE;

	ystring PathOnly()
	{
		std::vector<wchar_t> tx(10000);
		wcscpy_s(tx.data(), 10000,full.c_str());
		PathCchRemoveFileSpec(tx.data(), wcslen(tx.data()));
		return tx.data();
	}

	bool operator <(const FOUNDIT& f)
	{
		if (rel < f.rel)
			return true;
		return false;
	}

	void CloseMapper()
	{
		if (!mapper)
			return;
		mapper->Unmap();
		mapper = nullptr;
	}

	bool OpenMapper()
	{
		if (mapper)
			return true;
		mapper = std::make_shared<MAPPER>(full.c_str(), PAGE_READONLY);
		if (mapper->hF == 0 || mapper->hF == INVALID_HANDLE_VALUE)
		{
//			if (ccx)
//				ccx->Color(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
			wprintf(L"File %s could not be opened.\r\n", full.c_str());
			return false;
		}
		return true;

	}

};

struct ITEMMETAANDFILE
{
	ITEMMETA m;
	TAGDATA td;
	ystring rel;
	unsigned long fid = 0;
};


std::tuple<HRESULT, DWORD, unsigned long long> HandleToHandle(HANDLE hX, HANDLE hY, bool CRCX, unsigned long long mmax = (unsigned long long) - 1)
{
	CRC c;
	using namespace std;
	vector<char> d(1024 * 1024);
	DWORD crc = (DWORD)-1;
	unsigned long long tot = 0;
	for (;;)
	{
		DWORD A = 0;
		unsigned long long ds = 1024 * 1024;
		if (mmax != (unsigned long long) - 1)
			ds = min(ds, mmax);
		if (ds == 0)
			break;

		BOOL b = ReadFile(hX, d.data(), (DWORD)ds, &A, 0);
		if (!b)
			return make_tuple<>(E_FAIL, 0, 0);
		if (A == 0)
			break;
		DWORD A2 = 0;
		b = WriteFile(hY, d.data(), A, &A2, 0);
		if (!b || A != A2)
			return make_tuple<>(E_FAIL, 0, 0);
		tot += A;
		if (CRCX)
			crc = c.findcrc32(d.data(), A, crc);
		mmax -= A;
	}
	return make_tuple<>(S_OK, crc, tot);
}


class HASH
{
	BCRYPT_ALG_HANDLE h;
	BCRYPT_HASH_HANDLE ha;
public:

	HASH(const wchar_t* alg = BCRYPT_SHA256_ALGORITHM)
	{
		BCryptOpenAlgorithmProvider(&h, alg, 0, 0);
		if (h)
			BCryptCreateHash(h, &ha, 0, 0, 0, 0, 0);
	}

	bool hash(const BYTE* d, DWORD sz)
	{
		if (!ha)
			return false;
		auto nt = BCryptHashData(ha, (UCHAR*)d, sz, 0);
		return (nt == 0) ? true : false;
	}

	bool get(std::vector<BYTE>& b)
	{
		DWORD hl;
		ULONG rs;
		if (!ha)
			return false;
		auto nt = BCryptGetProperty(ha, BCRYPT_HASH_LENGTH, (PUCHAR)& hl, sizeof(DWORD), &rs, 0);
		if (nt != 0)
			return false;
		b.resize(hl);
		nt = BCryptFinishHash(ha, b.data(), hl, 0);
		if (nt != 0)
			return false;
		return true;
	}

	~HASH()
	{
		if (ha)
			BCryptDestroyHash(ha);
		ha = 0;
		if (h)
			BCryptCloseAlgorithmProvider(h, 0);
		h = 0;
	}
};

class E
{
	BCRYPT_ALG_HANDLE h = 0;
	BCRYPT_KEY_HANDLE hKey = 0;
	RWMUTEX mu;
public:
	bool Prepare(const char* pwd)
	{
		BCryptOpenAlgorithmProvider(&h, BCRYPT_AES_ALGORITHM, 0, 0);
		if (!h)
			return false;
		auto status = BCryptSetProperty(h, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);

		HASH ha;
		ha.hash((BYTE*)pwd, (DWORD)strlen(pwd));
		std::vector<BYTE> hh;
		ha.get(hh);

		status = BCryptGenerateSymmetricKey(h, &hKey, 0, 0, (PBYTE)hh.data(), (ULONG)hh.size(), 0);
		if (!hKey)
			return false;

		return true;
	}

	void End()
	{
		BCryptDestroyKey(hKey);
		BCryptCloseAlgorithmProvider(h, 0);
	}

	bool Encrypt(std::vector<char>& d,std::vector<char>& r)
	{
		if (d.empty())
		{
			r.resize(0);
			return true;
		}
		RWMUTEXLOCKWRITE l(&mu);
		ULONG ru = 0;
		auto status = BCryptEncrypt(hKey, (PUCHAR)d.data(), (ULONG)d.size(), 0, 0, 0, 0, 0, &ru, BCRYPT_BLOCK_PADDING);
		if (ru == 0)
			return false;
		r.resize(ru);
		status = BCryptEncrypt(hKey, (PUCHAR)d.data(), (ULONG)d.size(), 0, 0, 0, (PUCHAR)r.data(),(ULONG) r.size(), &ru, BCRYPT_BLOCK_PADDING);
		if (status)
			return false;
		r.resize(ru);
		return true;
	}


	bool Decrypt(const char* dd,size_t ss, std::vector<char>& r)
	{
		if (!dd || !ss)
		{
			r.resize(0);
			return true;
		}
		RWMUTEXLOCKWRITE l(&mu);
		ULONG ru = 0;
		auto status = BCryptDecrypt(hKey, (PUCHAR)dd, (ULONG)ss, 0, 0, 0, 0, 0, &ru, BCRYPT_BLOCK_PADDING);
		if (ru == 0)
			return false;
		r.resize(ru);
		status = BCryptDecrypt(hKey, (PUCHAR)dd, (ULONG)ss, 0, 0, 0, (PUCHAR)r.data(), (ULONG)r.size(), &ru, BCRYPT_BLOCK_PADDING);
		if (status)
			return false;
		r.resize(ru);
		return true;
	}

};

