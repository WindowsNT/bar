#pragma once


std::wstring DispSize(unsigned long long A)
{
	// 999  B
	// 341 KB
	ystring f;
	if (A == 0)
	{
		f.Format(L"         ");
	}
	else
	if (A < 999)
	{
		f.Format(L"%6u  B", A);
	}
	else
	if (A < 1023897)
	{
		f.Format(L"%6.1f KB", ((float)A/1024.0f));
	}
	else
	if (A < 1048471142)
	{
		f.Format(L"%6.1f MB", ((float)A / 1048576.0f));
	}
	else
		f.Format(L"%6.1f GB", ((float)A / 1073741824.0f));
	return f;
}




// Returns 'Y' 'N' 
int AnswerMode = 0;
RWMUTEX mAsk;
int Ask(const wchar_t* q)
{
	if (EndProcess)
		return 'N';
	if (sw.Y)
		return 'Y';
	if (AnswerMode == 1) // Always Y
		return 'Y';
	if (AnswerMode == 2) // Always N
		return 'N';

	RWMUTEXLOCKWRITE r(&mAsk);

	for (;;)
	{
		if (EndProcess)
			return 'N';
		if (sw.Y)
			return 'Y';
		if (AnswerMode == 1) // Always Y
			return 'Y';
		if (AnswerMode == 2) // Always N
			return 'N';

		wprintf(L"%s (Yes/No/Always/neVer/Stop): ", q);
		char v = 0;
		std::cin >> v;
		if (v == 'Y' || v == 'y')
			return 'Y';
		if (v == 'N' || v == 'n')
			return 'N';
		if (v == 'V' || v == 'v')
		{
			AnswerMode = 2;
			return 'N';
		}
		if (v == 'A' || v == 'a')
		{
			AnswerMode = 1;
			return 'Y';
		}
		if (v == 'S' || v == 's')
		{
			EndProcess = true;
			AnswerMode = 2;
			return 'N';
		}
	}

}
