#pragma once

enum class TAGCODE
{
	INVALID = 0,
	HEADER = 1,
	ITEMMETA = 2,
	ITEMDATA = 3,
	DIGITALSIGNATURE = 4,
};

enum class DATACOMP
{
	UNC = 0,
	// and COMPRESS_ALGORITHM_MSZIP, COMPRESS_ALGORITHM_XPRESS, COMPRESS_ALGORITHM_XPRESS_HUFF, COMPRESS_ALGORITHM_LZMS
};

enum class DATATYPE
{
	NONE = 0,
	DATA = 1,
	SIG = 2,
	DIFF = 3,
};

struct alignas(8) TAG
{
	unsigned long long code = (unsigned int)TAGCODE::INVALID;
	unsigned long long size = 0;
};

struct alignas(8) HEADER
{
	TAG tag = { (unsigned int)TAGCODE::HEADER, sizeof(HEADER)};
	char rsv[4] = "BAR";
	char BarType = 0; // rsv
	unsigned int major = BAR_MAJ_VERSION;
	unsigned int minor = BAR_MIN_VERSION;
};

struct alignas(8) ITEMMETA
{
	TAG tag = { (unsigned int)TAGCODE::ITEMMETA, sizeof(ITEMMETA)};
	unsigned int fid = 0; // file ID
	DWORD attr = 0;
	unsigned long long t1 = 0, t2 = 0, t3 = 0; // 3 times 
	unsigned long long uncsize = 0;
	unsigned long long cmpsize = 0;
	unsigned long unccrc = 0;
	unsigned long cmpcrc = 0;
	
	// Follows file name in UTF-8

};


struct alignas(8) ITEMDATA
{
	TAG tag = { (unsigned int)TAGCODE::ITEMDATA, sizeof(ITEMDATA)};
	unsigned int fid = 0; // file ID
	unsigned int cmpt = (unsigned int)DATACOMP::UNC;
	unsigned int dtt = (unsigned int)DATATYPE::NONE;
	unsigned int enc = 0;
	unsigned long long follows = 0;
};

struct alignas(8) DSIG
{
	TAG tag = { (unsigned int)TAGCODE::DIGITALSIGNATURE, sizeof(DSIG) };
};


struct TAGDATA2
{
	unsigned int ctype = 0;
	unsigned int etype = 0;
	unsigned int type = (unsigned int)DATATYPE::NONE;
	unsigned long long ofs = 0;
	unsigned long long s = 0;
	ITEMDATA copy;
};

struct TAGDATA
{
	unsigned long long ofs = 0;
	ystring fil; // if meta
	std::vector<char> d;
	std::vector<TAGDATA2> data;
	bool fnd = 0;

	bool operator<(const TAGDATA& t)
	{
		if (fil < t.fil)
			return true;
		return false;
	}	
	bool operator== (const TAGDATA& t)
	{
		if (fil == t.fil)
			return true;
		return false;
	}
};

inline unsigned long TagCollect(const char* m, unsigned long long fs, std::list<TAGDATA>& tags,std::vector<TAGDATA>& vtags)
{
	std::map<unsigned int, std::list<TAGDATA>::iterator> mint;
	unsigned long maxfid = 1;
	for (unsigned long long i = 0; i < fs;)
	{
		TAG* t = (TAG*)(m + i);
		if (i == 0 && t->code != (unsigned int)TAGCODE::HEADER)
			return false;

		if (t->code == (unsigned int)TAGCODE::HEADER)
		{
			HEADER* d = (HEADER*)t;
			if (strcmp(d->rsv, "BAR") != 0)
				return false;
		}

		auto ts = t->size;
		if (t->code == (unsigned int)TAGCODE::ITEMDATA)
		{
			ITEMDATA* d = (ITEMDATA*)t;

			// Find the fid in a previously saved tag
			auto tm = mint[d->fid];
			if (tm == tags.end())
				return false;
			TAG* t2 = (TAG*)tm->d.data();
			if (t2->code == (unsigned int)TAGCODE::ITEMMETA)
			{
				ITEMMETA* mx = (ITEMMETA*)t2;
				if (mx->fid != d->fid)
					return false;
				TAGDATA2 td2;
				td2.ofs = i + sizeof(ITEMDATA);
				td2.s = d->follows;
				td2.type = d->dtt;
				td2.ctype = d->cmpt;
				td2.etype = d->enc;
				td2.copy = *d;
				tm->data.push_back(td2);
			}

/*
			for (auto& tx : tags)
			{
				TAG* t2 = (TAG*)tx.d.data();
				if (t2->code == (unsigned int)TAGCODE::ITEMMETA)
				{
					ITEMMETA* mx = (ITEMMETA*)t2;
					if (mx->fid == d->fid)
					{
						TAGDATA2 td2;
						td2.ofs = i + sizeof(ITEMDATA);
						td2.s = d->follows;
						td2.type = d->dtt;
						td2.ctype = d->cmpt;		
						td2.copy = *d;
						tx.data.push_back(td2);
						break;
					}
				}
			}
			*/

			i += ts;
			continue;
		}

		TAGDATA td;

		if (t->code == (unsigned int)TAGCODE::ITEMMETA)
		{
			ITEMMETA* d = (ITEMMETA*)t;
			char* fn = ((char*)d) + sizeof(ITEMMETA);
			td.fil = fn;
			if (maxfid < d->fid)
				maxfid = d->fid;
		}

		td.ofs = i;
		td.d.resize(ts);
		memcpy(td.d.data(), t, td.d.size());

		tags.push_back(td);
		if (t->code == (unsigned int)TAGCODE::ITEMMETA)
		{
			auto lastit = tags.end();
			lastit--;
			ITEMMETA* d = (ITEMMETA*)t;
			mint[d->fid] = lastit;
		}

		i += t->size;
	}
	tags.sort();
	vtags.resize(tags.size());
	std::copy(tags.begin(), tags.end(), vtags.begin());
	return maxfid;
}



#include <compressapi.h>

class COMPRESSIONAPI
{
private:
	COMPRESSOR_HANDLE ch = 0;
	DECOMPRESSOR_HANDLE dh = 0;
	DWORD alg;


public:

	void constr(DWORD algorithm = COMPRESS_ALGORITHM_LZMS)
	{
		alg = algorithm;
		CreateCompressor(alg, 0, &ch);
		CreateDecompressor(alg, 0, &dh);
		//		SetCompressorInformation(ch, COMPRESS_INFORMATION_CLASS_LEVEL,)
	}

	void free()
	{
		if (ch)
			CloseCompressor(ch);
		if (dh)
			CloseDecompressor(dh);
		ch = 0;
		dh = 0;
	}

	COMPRESSIONAPI(DWORD a = COMPRESS_ALGORITHM_LZMS)
	{
		constr(a);
	}

	~COMPRESSIONAPI()
	{
		free();
	}

	HANDLE chh() { return ch; }
	HANDLE dhh() { return dh; }

	HRESULT COMPRESSIONAPI::CompressX(const char* b, size_t sz, std::vector<char>& r)
	{
		if (sz == 0)
		{
			r.resize(0);
			return S_FALSE;
		}
		if (!b)
			return E_POINTER;
		SIZE_T sc = 0;
		Compress(ch, (void*)b, sz, (void*)0, (size_t)0, &sc);
		if (!sc)
			return E_FAIL;
		r.resize(sc);
		BOOL x = Compress(ch, (void*)b, sz, r.data(), sc, &sc);
		if (!x)
			return E_FAIL;
		r.resize(sc);
		return S_OK;
	}

	HRESULT COMPRESSIONAPI::DecompressX(const char* b, size_t sz, std::vector<char>& r)
	{
		if (sz == 0)
		{
			r.resize(0);
			return S_FALSE;
		}
		if (!b)
			return E_POINTER;
		SIZE_T sc = 0;
		Decompress(dh, (void*)b, sz, (void*)0, (size_t)0, &sc);
		r.resize(sc);
		BOOL x = Decompress(dh, (void*)b, sz, r.data(), sc, &sc);
		if (!x)
			return E_FAIL;
		r.resize(sc);
		return S_OK;
	}
};

class CONSOLECOLOR
{
	HANDLE hConsole = 0;
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	WORD saved_attributes;
public:

	CONSOLECOLOR()
	{
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
		saved_attributes = consoleInfo.wAttributes;
	}

	void Color(WORD c)
	{
		SetConsoleTextAttribute(hConsole, c);
	}

	void r()
	{
		SetConsoleTextAttribute(hConsole, saved_attributes);
	}

	~CONSOLECOLOR()
	{
		r();
	}
};


