#include "util/Strings.h"

using namespace std;

using namespace util;

bool util::parseGuid(const wstring& s, GUID& out) noexcept {
	wstring t = s;

	if (!t.empty() && (t.front() == L'{' || t.front() == L'('))
		t.erase(t.begin());

	if (!t.empty() && (t.back() == L'}' || t.back() == L')'))
		t.pop_back();

	return UuidFromStringW(reinterpret_cast<RPC_WSTR>(t.data()), &out) == RPC_S_OK;
}

wstring util::toLower(wstring s) {
	for (auto& c : s)
		c = (wchar_t)towlower(c);

	return s;
}

wstring util::trim(wstring s) {
	while (!s.empty() && iswspace(s.front()))
		s.erase(s.begin());

	while (!s.empty() && iswspace(s.back()))
		s.pop_back();

	return s;
}
