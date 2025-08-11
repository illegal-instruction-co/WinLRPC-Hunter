#pragma once

#include <windows.h>

namespace util {

void initUnicode(UNICODE_STRING&, const wchar_t*) noexcept;
void initObjectAttributes(OBJECT_ATTRIBUTES&, UNICODE_STRING&) noexcept;

} // namespace util
