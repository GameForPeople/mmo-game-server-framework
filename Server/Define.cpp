#include "pch.h"

#include "Define.h"

namespace CHARACTER_CONVERTER
{
	void SetLocaleToKorea()
	{
		_wsetlocale(LC_ALL, L"korean");

		//26444 왜 때문에, 굳이 필요 없이, L-Value를 만들어야하는가;
		auto oldLocale = std::wcout.imbue(std::locale("koeran"));
	}
}