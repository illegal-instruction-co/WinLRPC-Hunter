#include "core/Types.h"
#include "core/EndpointCollector.h"
#include "core/RpcProber.h"
#include "core/IfVersionPredicate.h"

#include "io/Cli.h"
#include "io/OutputWriters.h"

#include "util/Strings.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <algorithm>

using namespace std;

using namespace core;

using namespace io;

using namespace util;

int wmain(int argc, wchar_t** argv) {
	Args args{};
	if (!parseArgs(argc, argv, args)) {
		printUsage(argv[0]);
		return 1;
	}

	::GUID target{};
	if (!parseGuid(args.ifUuid, target)) {
		wprintf(L"Invalid GUID\n");
		return 1;
	}

	DWORD currentSession = 0;
	ProcessIdToSessionId(GetCurrentProcessId(), &currentSession);
	vector<EndpointInfo> endpoints;

	if (args.onlyEndpoint.empty())
		collectEndpoints(args.useSpecificSession, args.specificSession, currentSession, endpoints);
	else
		endpoints.push_back({L"Any", (uint32_t)-1, args.onlyEndpoint, L"(manual)"});

	size_t epWidth = 8;
	for (auto& e : endpoints)
		epWidth = min<size_t>(max(epWidth, e.name.size()), 80);

	if (!args.json) {
		wprintf(L"Scanning %zu endpoints for %s (timeout=%lu, conc=%zu)\n", endpoints.size(), args.ifUuid.c_str(), args.timeoutMs, args.concurrency);
		writeTableHeader(epWidth);
	}

	atomic<size_t> hits{0}, scanned{0}, failures{0};
	mutex outM;

	size_t idx = 0;
	auto worker = [&] {
		for (;;) {
			size_t i = idx++;
			if (i >= endpoints.size())
				break;
			const auto& ep = endpoints[i];
			scanned++;

			auto hit = probeWithAuthSet(ep.name, target, args.timeoutMs, args.authPlan, args.verPred);
			if (hit.ok)
				hits++;

			lock_guard lk(outM);
			if (args.json)
				writeJsonLine(ep, hit, target);
			else
				writeTableRow(ep, hit, epWidth);
		}
	};

	vector<jthread> pool;
	pool.reserve(args.concurrency);
	for (size_t t = 0; t < args.concurrency; t++)
		pool.emplace_back(worker);
	pool.clear(); // join

	if (!args.json)
		wprintf(L"\nSummary: total=%zu, scanned=%zu, hits=%zu, failures=%zu\n", endpoints.size(), scanned.load(), hits.load(), failures.load());

	if (failures > 0 && hits == 0)
		return 3;
	if (hits == 0)
		return 2;
	return 0;
}
