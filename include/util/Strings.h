#pragma once

#include <string>
#include <optional>

#include <windows.h>

namespace util {

[[nodiscard]] bool parseGuid(const std::wstring&, GUID&) noexcept;
[[nodiscard]] std::wstring toLower(std::wstring);
[[nodiscard]] std::wstring trim(std::wstring);

} // namespace util
