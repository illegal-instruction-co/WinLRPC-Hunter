#pragma once

#include "core/Types.h"
#include "core/IfVersionPredicate.h"

#include <string>
#include <vector>

namespace io {

struct Args final {
	std::wstring ifUuid;
	bool useSpecificSession = false;
	uint32_t specificSession = 0;
	unsigned long timeoutMs = 2000;
	std::wstring onlyEndpoint;
	bool noSkip = false;
	bool json = false;
	size_t concurrency = 32;
	core::IfVerPred verPred{};

	[[nodiscard]] std::vector<core::AuthAttempt> authPlan{core::AuthAttempt::NONE, core::AuthAttempt::NEGOTIATE, core::AuthAttempt::NTLM,
														  core::AuthAttempt::KERBEROS};
};

[[nodiscard]] bool parseArgs(int, wchar_t**, Args&);
void printUsage(const wchar_t*);

} // namespace io
