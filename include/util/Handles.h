#pragma once
#include <utility>

#include <windows.h>

namespace util {

struct UniqueHandle final {
	HANDLE h = nullptr;

	__forceinline ~UniqueHandle() {
		if (h)
			CloseHandle(h);
	}

	__forceinline UniqueHandle() = default;

	__forceinline UniqueHandle(const UniqueHandle&) = delete;
	__forceinline UniqueHandle& operator=(const UniqueHandle&) = delete;
	__forceinline UniqueHandle(UniqueHandle&& o) noexcept : h(std::exchange(o.h, nullptr)) {}
	__forceinline UniqueHandle& operator=(UniqueHandle&& o) noexcept {
		if (this != &o) {
			if (h)
				CloseHandle(h);

			h = std::exchange(o.h, nullptr);
		}

		return *this;
	}

	__forceinline operator HANDLE() const noexcept {
		return h;
	}
};

} // namespace util
