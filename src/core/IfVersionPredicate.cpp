#include "core/IfVersionPredicate.h"

#include <string>

using namespace std;

using namespace core;

namespace {
inline bool parseUShort(const wstring& s, uint16_t& out) {
	if (s.empty())
		return false;

	wchar_t* end = nullptr;
	unsigned long v = wcstoul(s.c_str(), &end, 10);

	if (!end || *end)
		return false;

	if (v > 0xFFFFul)
		return false;

	out = static_cast<uint16_t>(v);

	return true;
}

inline bool parseMajMin(const wstring& mm, uint16_t& maj, uint16_t& min) {
	const auto p = mm.find(L'.');

	if (p == wstring::npos)
		return false;

	return parseUShort(mm.substr(0, p), maj) && parseUShort(mm.substr(p + 1), min);
}
} // namespace

bool IfVerPred::match(uint16_t maj, uint16_t min) const noexcept {
	switch (kind) {
	case ANY:
		return true;
	case EQ:
		return maj == aMaj && min == aMin;
	case GE:
		return (maj > aMaj) || (maj == aMaj && min >= aMin);
	case LE:
		return (maj < aMaj) || (maj == aMaj && min <= aMin);
	case RANGE: {
		bool ge = (maj > aMaj) || (maj == aMaj && min >= aMin);
		bool le = (maj < bMaj) || (maj == bMaj && min <= bMin);

		return ge && le;
	}
	}
	return true;
}

bool core::parseIfVerPredicate(const wstring& spec, IfVerPred& out) {
	if (spec.empty()) {
		out = {};

		return true;
	}

	wstring t = spec;
	while (!t.empty() && iswspace(t.front()))
		t.erase(t.begin());

	while (!t.empty() && iswspace(t.back()))
		t.pop_back();

	if (t.empty()) {
		out = {};

		return true;
	}

	if (t.rfind(L">=", 0) == 0) {
		if (!parseMajMin(t.substr(2), out.aMaj, out.aMin))
			return false;

		out.kind = IfVerPred::GE;
		return true;
	}

	if (t.rfind(L"<=", 0) == 0) {
		if (!parseMajMin(t.substr(2), out.aMaj, out.aMin))
			return false;

		out.kind = IfVerPred::LE;
		return true;
	}

	if (t.rfind(L"==", 0) == 0) {
		if (!parseMajMin(t.substr(2), out.aMaj, out.aMin))
			return false;

		out.kind = IfVerPred::EQ;
		return true;
	}

	if (const auto dash = t.find(L'-'); dash != wstring::npos) {
		if (!parseMajMin(t.substr(0, dash), out.aMaj, out.aMin))
			return false;

		if (!parseMajMin(t.substr(dash + 1), out.bMaj, out.bMin))
			return false;

		out.kind = IfVerPred::RANGE;
		return true;
	}

	if (parseMajMin(t, out.aMaj, out.aMin)) {
		out.kind = IfVerPred::EQ;

		return true;
	}

	return false;
}
