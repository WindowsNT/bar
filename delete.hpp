

void Delete()
{
	using namespace std;
	if (files.size() < 2)
	{
		Help(0, "d");
		return;
	}

	TEMPFILE t;
	CONSOLECOLOR cc;
	shared_ptr<MAPPER> m = make_shared<MAPPER>(files[0].c_str(), PAGE_READONLY);
	const char* map = (const char*)m->Map();
	if (!map)
	{
		cc.Color(FOREGROUND_RED | FOREGROUND_INTENSITY);
		wcout << L"Cannot open " << files[0].c_str() << endl;
		return;
	}

	auto fs = m->fs();
	cc.Color(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	wcout << L"Reading " << files[0].c_str() << endl;

	list<TAGDATA> tags;
	vector<TAGDATA> vtags;
	if (!TagCollect(map, fs, tags, vtags))
	{
		cc.Color(FOREGROUND_RED | FOREGROUND_INTENSITY);
		wcout << L"Not a BAR file" << endl;
		return;
	}

	// Write header
	try
	{
		HEADER t1;
		auto hrw = t.Write((char*)& t1, sizeof(t1));
		FailWrite(hrw);

		// Add to sw.I the file list
		for (size_t ii = 1; ii < files.size(); ii++)
		{
			sw.I.push_back(files[ii].a_str());
		}

		cc.r();
		for (auto& tx : tags)
		{
			if (EndProcess)
				break;
			TAG* tt = (TAG*)tx.d.data();
			if (tt->code == (unsigned int)TAGCODE::ITEMMETA)
			{
				tx.fnd = false;
				ITEMMETA* h = (ITEMMETA*)tt;
				char* pp = (char*)tt;
				vector<char> fn;
				fn.resize(h->tag.size - sizeof(ITEMMETA));
				memcpy(fn.data(), pp + sizeof(ITEMMETA), fn.size());
				ystring fnn = fn.data();

				// Delete ?
				tx.fnd = false;
				bool id = IncludeItem(fnn.a_str(), false);
				if (id)
					tx.fnd = true;

				if (tx.fnd)
				{
					wprintf(L"Deleting %s...\r\n", fnn.c_str());
					continue;
				}

				// Put the reference
				auto hrw = t.Write((const char*)h, sizeof(ITEMMETA));
				FailWrite(hrw);

				// And the file name
				auto a = fnn.a_str();
				hrw = t.Write(a, strlen(a) + 1);
				FailWrite(hrw);
			}
		}

		// And copy the data (signatures)
		for (auto& tx : tags)
		{
			if (EndProcess)
				break;
			if (tx.fnd)
				continue;
			TAG* tt = (TAG*)tx.d.data();
			if (tt->code == (unsigned int)TAGCODE::ITEMMETA)
			{
				for (auto& dd : tx.data)
				{
					if (dd.type == (unsigned int)DATATYPE::SIG)
					{
						// Put the reference
						auto hrw = t.Write((const char*)& dd.copy, sizeof(ITEMDATA));
						FailWrite(hrw);

						// And the data
						hrw = t.Write((const char*)(map + dd.ofs), dd.s);
						FailWrite(hrw);
					}
				}
			}
		}

		// And copy the data
		for (auto& tx : tags)
		{
			if (EndProcess)
				break;
			if (tx.fnd)
				continue;
			TAG* tt = (TAG*)tx.d.data();
			if (tt->code == (unsigned int)TAGCODE::ITEMMETA)
			{
				for (auto& dd : tx.data)
				{
					if (dd.type == (unsigned int)DATATYPE::DATA)
					{
						// Put the reference
						auto hrw = t.Write((const char*)& dd.copy, sizeof(ITEMDATA));
						FailWrite(hrw);

						// And the data
						hrw = t.Write((const char*)(map + dd.ofs), dd.s);
						FailWrite(hrw);
					}
				}
			}
		}




		m->Unmap();
		m = nullptr;
		
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
	cc.r();
}
