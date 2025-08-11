#pragma once

#include <string>
#include <optional>

namespace core {

struct IfVerPred final {

	enum Kind
	{
		ANY,
		EQ,
		GE,
		LE,
		RANGE
	} kind{ANY};

	uint16_t aMaj = 0;
	uint16_t aMin = 0;
	uint16_t bMaj = 0;
	uint16_t bMin = 0;

	[[nodiscard]] bool match(uint16_t, uint16_t) const noexcept;
};

[[nodiscard]] bool parseIfVerPredicate(const std::wstring&, IfVerPred&);

} // namespace core
