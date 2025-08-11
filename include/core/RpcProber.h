#pragma once

#include "Types.h"
#include "IfVersionPredicate.h"

#include <vector>

#include <windows.h>

namespace core {

[[nodiscard]] IfHit probeWithAuthSet(const std::wstring&, const GUID&, unsigned long, const std::vector<AuthAttempt>&, const IfVerPred&) noexcept;

}
