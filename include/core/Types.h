#pragma once

#include <string>
#include <vector>

#include <cstdint>

namespace core {

enum class AuthAttempt
{
	NONE,
	NTLM,
	NEGOTIATE,
	KERBEROS
};

struct EndpointInfo final {
	std::wstring scope; // "Global" | "Session" | "Any"
	uint32_t sessionId;
	std::wstring name; // ALPC port name
	std::wstring ntPath;
};

struct IfHit final {
	bool ok = false;
	uint16_t vMaj = 0;
	uint16_t vMin = 0;
	std::vector<AuthAttempt> acceptedAuth;
};

} // namespace core
