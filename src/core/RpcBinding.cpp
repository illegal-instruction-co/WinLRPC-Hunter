#include "core/RpcBinding.h"

using namespace std;

using namespace core;

RPC_STATUS core::makeBindingNcalrpc(const wstring& endpoint, unsigned long callTimeoutMs, UniqueRpcBinding& out) noexcept {
	UniqueRpcString str;

	RPC_STATUS rs = RpcStringBindingComposeW(nullptr, reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(L"ncalrpc")), nullptr,
											 reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(endpoint.c_str())), nullptr, &str.s);

	if (rs != RPC_S_OK)
		return rs;

	rs = RpcBindingFromStringBindingW(str, &out.b);
	if (rs != RPC_S_OK)
		return rs;

	RpcBindingSetOption(out, RPC_C_OPT_CALL_TIMEOUT, callTimeoutMs);

	return RPC_S_OK;
}

RPC_STATUS core::setAuth(RPC_BINDING_HANDLE h, AuthAttempt a) noexcept {

	switch (a) {
	case AuthAttempt::NONE:
		return RPC_S_OK;
	case AuthAttempt::NTLM:
		return RpcBindingSetAuthInfoExW(h, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_AUTHN_WINNT, nullptr, 0, nullptr);
	case AuthAttempt::NEGOTIATE:
		return RpcBindingSetAuthInfoExW(h, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_AUTHN_GSS_NEGOTIATE, nullptr, 0, nullptr);
	case AuthAttempt::KERBEROS:
		return RpcBindingSetAuthInfoExW(h, nullptr, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_AUTHN_GSS_KERBEROS, nullptr, 0, nullptr);
	default:
		return RPC_S_INVALID_ARG;
	}

	return RPC_S_INVALID_ARG;
}
