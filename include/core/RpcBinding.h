#pragma once
#include "Types.h"

#include <string>

#include <rpc.h>
#include <windows.h>

namespace core {

struct UniqueRpcString final {
	RPC_WSTR s = nullptr;

	__forceinline ~UniqueRpcString() {
		if (s)
			RpcStringFreeW(&s);
	}

	__forceinline operator RPC_WSTR() const noexcept {
		return s;
	}
};

struct UniqueRpcBinding final {
	RPC_BINDING_HANDLE b = nullptr;

	__forceinline ~UniqueRpcBinding() {
		if (b)
			RpcBindingFree(&b);
	}

	__forceinline operator RPC_BINDING_HANDLE() const noexcept {
		return b;
	}
};

[[nodiscard]] RPC_STATUS makeBindingNcalrpc(const std::wstring&, unsigned long, UniqueRpcBinding&) noexcept;
[[nodiscard]] RPC_STATUS setAuth(RPC_BINDING_HANDLE, AuthAttempt) noexcept;

} // namespace core
