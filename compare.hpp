#pragma once


void Compare(bool R = false)
{
	Extract(true, R, true);
	for (auto& r : CompareList)
	{
		if (r.w.nFileSizeHigh == 0 && r.w.nFileSizeLow == 0)
			continue;
		Warnings->emplace_back(ystring().Format(L"%s failed", r.rel.c_str()));
	}
}
