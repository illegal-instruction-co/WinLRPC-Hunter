#define UNICODE
#define _UNICODE

#include <string>
#include <vector>
#include <string_view>
#include <unordered_set>
#include <algorithm>
#include <cwchar>
#include <iostream>
#include <iomanip>

#include <windows.h>
#include <rpc.h>

#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "ntdll.lib")

using namespace std;

extern "C" {
typedef LONG NTSTATUS;
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	POBJECT_ATTRIBUTES ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;
	PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
typedef struct _OBJECT_DIRECTORY_INFORMATION {
	UNICODE_STRING Name;
	UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;
__declspec(dllimport) NTSTATUS __stdcall NtOpenDirectoryObject(_Out_ PHANDLE, _In_ ACCESS_MASK, _In_ POBJECT_ATTRIBUTES);
__declspec(dllimport) NTSTATUS __stdcall NtQueryDirectoryObject(_In_ HANDLE, _Out_writes_bytes_opt_(Length) PVOID, _In_ ULONG, _In_ BOOLEAN, _In_ BOOLEAN, _Inout_ PULONG, _Out_opt_ PULONG);
}

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif
#ifndef OBJ_CASE_INSENSITIVE
#define OBJ_CASE_INSENSITIVE 0x00000040L
#endif
#ifndef DIRECTORY_QUERY
#define DIRECTORY_QUERY 0x0001
#endif

struct IfHit {
	bool ok;
	USHORT vMaj;
	USHORT vMin;
};
struct EndpointInfo {
	wstring scope;
	DWORD sessionId;
	wstring name;
	wstring ntPath;
};

static void initUnicode(UNICODE_STRING &us, const wchar_t *s) {
	if (!s) {
		us.Buffer = nullptr;
		us.Length = us.MaximumLength = 0;
		return;
	}
	const size_t L = wcslen(s);
	us.Buffer = const_cast<PWSTR>(s);
	us.Length = static_cast<USHORT>(L * sizeof(wchar_t));
	us.MaximumLength = us.Length;
}

static void initObjectAttributes(OBJECT_ATTRIBUTES &oa, UNICODE_STRING &name) {
	oa.Length = sizeof(oa);
	oa.RootDirectory = nullptr;
	oa.ObjectName = &name;
	oa.Attributes = OBJ_CASE_INSENSITIVE;
	oa.SecurityDescriptor = nullptr;
	oa.SecurityQualityOfService = nullptr;
}

static void enumObjectDirectory(const wstring &ntPath, vector<wstring> &outNames) {
	UNICODE_STRING name{};
	initUnicode(name, ntPath.c_str());
	OBJECT_ATTRIBUTES oa{};
	initObjectAttributes(oa, name);
	HANDLE h = nullptr;
	if (!NT_SUCCESS(NtOpenDirectoryObject(&h, DIRECTORY_QUERY, &oa))) return;

	const ULONG bufSize = 32 * 1024;
	vector<unsigned char> buf(bufSize);
	ULONG ctx = 0;
	BOOLEAN restart = TRUE;

	for (;;) {
		ULONG retLen = 0;
		const NTSTATUS st = NtQueryDirectoryObject(h, buf.data(), bufSize, TRUE, restart, &ctx, &retLen);
		restart = FALSE;
		if (!NT_SUCCESS(st)) break;
		const auto info = reinterpret_cast<POBJECT_DIRECTORY_INFORMATION>(buf.data());
		if (info->Name.Buffer && info->Name.Length) {
			wstring nm(info->Name.Buffer, info->Name.Length / sizeof(wchar_t));
			if (!nm.empty()) outNames.push_back(nm);
		}
	}
	CloseHandle(h);
}

static bool parseGuid(wstring_view s, GUID &out) {
	wstring tmp(s);
	RPC_STATUS st = UuidFromStringW(reinterpret_cast<RPC_WSTR>(tmp.data()), &out);
	if (st == RPC_S_OK) return true;
	if (!tmp.empty() && (tmp.front() == L'{' || tmp.front() == L'(')) {
		if (!tmp.empty() && (tmp.back() == L'}' || tmp.back() == L')')) tmp.pop_back();
		tmp.erase(tmp.begin());
		st = UuidFromStringW(reinterpret_cast<RPC_WSTR>(tmp.data()), &out);
		return st == RPC_S_OK;
	}
	return false;
}

static IfHit probeEndpoint(const wstring &endpointName, const GUID &target, unsigned long timeoutMs) {
	RPC_WSTR s = nullptr;
	if (RpcStringBindingComposeW(nullptr,
								reinterpret_cast<RPC_WSTR>(const_cast<wchar_t *>(L"ncalrpc")),
								nullptr,
								reinterpret_cast<RPC_WSTR>(const_cast<wchar_t *>(endpointName.c_str())),
								nullptr,
								&s) != RPC_S_OK) {
		return { false, 0, 0 };
	}
	RPC_BINDING_HANDLE h = nullptr;
	const RPC_STATUS rsFrom = RpcBindingFromStringBindingW(s, &h);
	if (s) RpcStringFreeW(&s);
	if (rsFrom != RPC_S_OK) return { false, 0, 0 };

	RpcBindingSetOption(h, RPC_C_OPT_CALL_TIMEOUT, timeoutMs);

	RPC_IF_ID_VECTOR *ifs = nullptr;
	const RPC_STATUS rsIds = RpcMgmtInqIfIds(h, &ifs);
	if (rsIds != RPC_S_OK || !ifs) {
		RpcBindingFree(&h);
		return { false, 0, 0 };
	}

	IfHit hit{ false, 0, 0 };
	for (unsigned long i = 0; i < ifs->Count; i++) {
		RPC_IF_ID *iid = ifs->IfId[i];
		if (!iid) continue;
		if (InlineIsEqualGUID(iid->Uuid, target)) {
			hit = { true, iid->VersMajor, iid->VersMinor };
			break;
		}
	}
	RpcIfIdVectorFree(&ifs);
	RpcBindingFree(&h);
	return hit;
}

static void collectEndpoints(bool useSpecificSession, DWORD specificSession, DWORD currentSession, vector<EndpointInfo> &out) {
	vector<wstring> names;

	// Global scope
	names.clear();
	enumObjectDirectory(L"\\RPC Control", names);
	for (auto &n : names) out.push_back(EndpointInfo{ L"Global", DWORD(-1), n, L"\\RPC Control" });

	const auto scanSession = [&](DWORD sid) {
		vector<wstring> local;
		wchar_t path[256];
		swprintf(path, 256, L"\\Sessions\\%u\\BaseNamedObjects\\RPC Control", sid);
		enumObjectDirectory(path, local);
		for (auto &n : local) out.push_back(EndpointInfo{ L"Session", sid, n, path });
	};

	if (useSpecificSession) {
		scanSession(specificSession);
	} else {
		scanSession(currentSession);
		for (DWORD s = 0; s <= 8; s++) if (s != currentSession) scanSession(s);
	}

	// de-dup by name + scope + session
	sort(out.begin(), out.end(), [](const EndpointInfo &a, const EndpointInfo &b) {
		if (a.scope != b.scope) return a.scope < b.scope;
		if (a.sessionId != b.sessionId) return a.sessionId < b.sessionId;
		return a.name < b.name;
	});
	out.erase(unique(out.begin(), out.end(), [](const EndpointInfo &a, const EndpointInfo &b) {
				   return a.scope == b.scope && a.sessionId == b.sessionId && a.name == b.name;
			   }),
		out.end());
}

int wmain(int argc, wchar_t **argv) {
	if (argc < 2) {
		wcout << L"Usage: " << argv[0] << L" <IFUUID> [--session <id>] [--timeout <ms>] [--endpoint <name>] [--no-skip]\n";
		return 1;
	}

	GUID target{};
	const wstring targetStr = argv[1];
	if (!parseGuid(targetStr, target)) {
		wcout << L"Invalid GUID: " << targetStr << L"\n";
		return 1;
	}

	DWORD specificSession = 0, currentSession = 0;
	ProcessIdToSessionId(GetCurrentProcessId(), &currentSession);
	bool useSpecificSession = false;
	unsigned long callTimeoutMs = 2000;
	wstring onlyEndpoint;
	bool noSkip = false;

	for (int i = 2; i < argc; i++) {
		if (_wcsicmp(argv[i], L"--session") == 0 && i + 1 < argc) {
			specificSession = static_cast<DWORD>(_wtoi(argv[++i]));
			useSpecificSession = true;
		} else if (_wcsicmp(argv[i], L"--timeout") == 0 && i + 1 < argc) {
			callTimeoutMs = static_cast<unsigned long>(_wtol(argv[++i]));
		} else if (_wcsicmp(argv[i], L"--endpoint") == 0 && i + 1 < argc) {
			onlyEndpoint = argv[++i];
		} else if (_wcsicmp(argv[i], L"--no-skip") == 0) {
			noSkip = true;
		}
	}

	vector<EndpointInfo> endpoints;
	if (onlyEndpoint.empty()) {
		collectEndpoints(useSpecificSession, specificSession, currentSession, endpoints);
	} else {
		// Unknown scope for single endpoint -> show as "Any"
		endpoints.push_back(EndpointInfo{ L"Any", DWORD(-1), onlyEndpoint, L"(manual)" });
	}

	const unordered_set<wstring> defaultSkip = {
		L"LSARPC_ENDPOINT", L"lsapolicylookup", L"samss lpc", L"securityevent", L"audit", L"lsasspirpc", L"LSA_IDPEXT_ENDPOINT", L"LSA_EAS_ENDPOINT", L"protected_storage"
	};

	size_t epWidth = 8, scopeWidth = 6, verWidth = 7, protoWidth = 8, sessWidth = 8;
	for (const auto &e : endpoints) epWidth = max(epWidth, e.name.size());
	epWidth = min<size_t>(epWidth, 80); // keep table tidy

	wcout << L"Scanning " << endpoints.size() << L" endpoints for " << targetStr << L"  (timeout=" << callTimeoutMs << L"ms)\n";

	wcout << left << setw(static_cast<int>(scopeWidth)) << L"Scope" << L"  " << setw(static_cast<int>(sessWidth)) << L"Session" << L"  "
		  << setw(static_cast<int>(epWidth)) << L"Endpoint" << L"  " << setw(static_cast<int>(verWidth)) << L"IfVersion" << L"  "
		  << setw(static_cast<int>(protoWidth)) << L"Protocol" << L"\n";

	wcout << wstring(scopeWidth + 2 + sessWidth + 2 + epWidth + 2 + verWidth + 2 + protoWidth, L'-') << L"\n";

	size_t hits = 0, scanned = 0, skipped = 0;

	for (const auto &ep : endpoints) {
		if (!noSkip && onlyEndpoint.empty() && defaultSkip.find(ep.name) != defaultSkip.end()) {
			skipped++;
			continue;
		}
		scanned++;

		const IfHit hit = probeEndpoint(ep.name, target, callTimeoutMs);
		if (hit.ok) {
			wcout << left << setw(static_cast<int>(scopeWidth)) << ep.scope << L"  ";
			if (ep.scope == L"Session") {
				wcout << setw(static_cast<int>(sessWidth)) << ep.sessionId << L"  ";
			} else {
				wcout << setw(static_cast<int>(sessWidth)) << L"-"
					  << L"  ";
			}
			wcout << setw(static_cast<int>(epWidth)) << ep.name << L"  " << setw(static_cast<int>(verWidth)) << (to_wstring(hit.vMaj) + L"." + to_wstring(hit.vMin))
				  << L"  " << setw(static_cast<int>(protoWidth)) << L"ncalrpc" << L"\n";
			hits++;
			if (!onlyEndpoint.empty()) break;
		}
	}

	wcout << L"\nSummary: " << L"total=" << endpoints.size() << L", scanned=" << scanned << L", skipped=" << skipped << L", hits=" << hits << L"\n";

	if (!hits) {
		wcout << L"No live LRPC endpoint found for " << targetStr << L"\n";
		return 2;
	}
	return 0;
}

void *__RPC_USER midl_user_allocate(size_t s) {
	return HeapAlloc(GetProcessHeap(), 0, s);
}
void __RPC_USER midl_user_free(void *p) {
	if (p) HeapFree(GetProcessHeap(), 0, p);
}
