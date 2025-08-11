#include "io/Cli.h"
#include "util/Strings.h"
#include "core/IfVersionPredicate.h"

using namespace std;
using namespace io;
using namespace util;
using namespace core;

void io::printUsage(const wchar_t* exe) {
	wprintf(L"Usage: %s <IFUUID> [--session <id>] [--timeout <ms>] [--endpoint <name>] "
			L"[--no-skip] [--json] [--concurrency <1-128>] [--ifver <pred>] "
			L"[--auth auto|none|ntlm|negotiate|kerberos|all]\n",
			exe);
}

bool io::parseArgs(int argc, wchar_t** argv, Args& a) {
	if (argc < 2)
		return false;
	a.ifUuid = argv[1];

	for (int i = 2; i < argc; i++) {
		if (_wcsicmp(argv[i], L"--session") == 0 && i + 1 < argc) {
			a.specificSession = (uint32_t)_wtoi(argv[++i]);
			a.useSpecificSession = true;
			continue;
		}
		if (_wcsicmp(argv[i], L"--timeout") == 0 && i + 1 < argc) {
			a.timeoutMs = (unsigned long)_wtol(argv[++i]);
			continue;
		}
		if (_wcsicmp(argv[i], L"--endpoint") == 0 && i + 1 < argc) {
			a.onlyEndpoint = argv[++i];
			continue;
		}
		if (_wcsicmp(argv[i], L"--no-skip") == 0) {
			a.noSkip = true;
			continue;
		}
		if (_wcsicmp(argv[i], L"--json") == 0) {
			a.json = true;
			continue;
		}
		if (_wcsicmp(argv[i], L"--concurrency") == 0 && i + 1 < argc) {
			long v = _wtol(argv[++i]);
			if (v < 1)
				v = 1;
			if (v > 128)
				v = 128;
			a.concurrency = (size_t)v;
			continue;
		}
		if (_wcsicmp(argv[i], L"--ifver") == 0 && i + 1 < argc) {
			IfVerPred p{};
			if (!parseIfVerPredicate(argv[++i], p))
				return false;
			a.verPred = p;
			continue;
		}
		if (_wcsicmp(argv[i], L"--auth") == 0 && i + 1 < argc) {
			wstring v = toLower(argv[++i]);
			if (v == L"auto" || v == L"all")
				a.authPlan = {AuthAttempt::NONE, AuthAttempt::NTLM, AuthAttempt::NEGOTIATE, AuthAttempt::KERBEROS};
			else if (v == L"none")
				a.authPlan = {AuthAttempt::NONE};
			else if (v == L"ntlm")
				a.authPlan = {AuthAttempt::NTLM};
			else if (v == L"negotiate")
				a.authPlan = {AuthAttempt::NEGOTIATE};
			else if (v == L"kerberos")
				a.authPlan = {AuthAttempt::KERBEROS};
			else
				return false;
			continue;
		}
		return false;
	}
	return true;
}
