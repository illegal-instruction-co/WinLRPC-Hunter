#include "io/OutputWriters.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

using namespace io;

using namespace core;

void io::writeTableHeader(size_t epWidth) {
	size_t scopeW = 6, sessW = 7, verW = 8, protoW = 8, authW = 18;
	wcout << left << setw((int)scopeW) << L"Scope" << L"  " << setw((int)sessW) << L"Session" << L"  " << setw((int)epWidth) << L"Endpoint" << L"  "
		  << setw((int)verW) << L"IfVersion" << L"  " << setw((int)protoW) << L"Protocol" << L"  " << setw((int)authW) << L"AuthAccepted" << L"\n";
	wcout << wstring(scopeW + 2 + sessW + 2 + epWidth + 2 + verW + 2 + protoW + 2 + authW, L'-') << L"\n";
}

void io::writeTableRow(const EndpointInfo& ep, const IfHit& hit, size_t epWidth) {
	size_t scopeW = 6, sessW = 7, verW = 8, protoW = 8, authW = 18;
	wstring ver = hit.ok ? to_wstring(hit.vMaj) + L"." + to_wstring(hit.vMin) : L"-";
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

	wcout << left << setw((int)scopeW) << ep.scope << L"  " << setw((int)sessW) << (ep.scope == L"Session" ? to_wstring(ep.sessionId) : L"-") << L"  "
		  << setw((int)epWidth) << ep.name << L"  " << setw((int)verW) << ver << L"  " << setw((int)protoW) << L"ncalrpc" << L"  " << setw((int)authW)
		  << auth << L"\n";
}
