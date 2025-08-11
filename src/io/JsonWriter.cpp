#include "io/OutputWriters.h"

#include <iostream>
#include <string>

#include <rpc.h>

using namespace std;

using namespace io;

using namespace core;

void io::writeJsonLine(const EndpointInfo& ep, const IfHit& hit, const GUID& target) {
	RPC_WSTR g = nullptr;
	UuidToStringW(reinterpret_cast<UUID const*>(&target), &g);

	wstring auth;
	for (size_t i = 0; i < hit.acceptedAuth.size(); i++) {
		if (i)
			auth += L",";
		switch (hit.acceptedAuth[i]) {
		case AuthAttempt::NONE:
			auth += L"none";
			break;
		case AuthAttempt::NTLM:
			auth += L"ntlm";
			break;
		case AuthAttempt::NEGOTIATE:
			auth += L"negotiate";
			break;
		case AuthAttempt::KERBEROS:
			auth += L"kerberos";
			break;
		}
	}

	wcout << L"{\"scope\":\"" << ep.scope << L"\",\"session\":" << (ep.scope == L"Session" ? ep.sessionId : (uint32_t)-1) << L",\"endpoint\":\"" << ep.name
		  << L"\",\"if_ver\":\"" << (hit.ok ? to_wstring(hit.vMaj) + L"." + to_wstring(hit.vMin) : L"")
		  << L"\",\"protocol\":\"ncalrpc\",\"auth_fingerprint\":\"" << auth << L"\",\"if_uuid\":\"" << (g ? reinterpret_cast<const wchar_t*>(g) : L"")
		  << L"\"}\n";

	if (g)
		RpcStringFreeW(&g);
}
