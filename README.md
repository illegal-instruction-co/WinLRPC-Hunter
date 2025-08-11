# WinLRPC-Hunter

**High-Precision Targeted UUID Scanner for Windows Local RPC Endpoints**

WinLRPC-Hunter is a modular, C++20-compliant command-line tool for scanning and probing **Windows Local RPC (ncalrpc)** endpoints to locate a specific interface UUID.  
It enumerates both global and per-session `\RPC Control` directories, connects to each endpoint, and verifies live matches using `RpcMgmtInqIfIds`.  
This enables security researchers, reverse engineers, and developers to quickly identify which endpoints are hosting the target interface, with minimal noise and maximum accuracy.

---

## Features
- Enumerates **global** and **session-specific** Local RPC (`ncalrpc`) endpoints
- Connects to each endpoint and verifies **live interface UUIDs** (no stale registrations)
- **Unique Selection Point**: scans **only** for the specified UUID and ignores all others
- Detects **unregistered but active** LRPC endpoints
- Differentiates Global and Session namespaces in output
- Clean CLI output with hit summaries
- **JSON output mode** for integration with other tools
- Adjustable concurrency for faster scanning on multi-core systems
- Interface version predicate filtering (e.g., `>=1.0`, `==2.3`)
- Configurable RPC authentication type (`auto`, `none`, `ntlm`, `negotiate`, `kerberos`, `all`)
- Modular C++20 codebase, SonarQube quality-gate compliant

---

## What is "Unique Selection Point"?
Unlike general RPC scanners that dump all interfaces, WinLRPC-Hunter focuses solely on **one target UUID** you provide.

This targeted approach:
- Removes noise from unrelated endpoints  
- Speeds up scanning in large environments  
- Reduces false positives when hunting a specific RPC interface  

### Workflow Diagram

```
+-----------------------+
| Enumerate Endpoints   |
| (Global + Sessions)   |
+----------+------------+
           |
           v
+-----------------------+
| For each endpoint:    |
| Query live UUIDs      |
+----------+------------+
           |
           v
+-----------------------+
| Match against target  |
| UUID only             |
+----------+------------+
           |
      +----+----+
      |  HIT    |
      +---------+
```

This ensures the scan results contain **only endpoints that actually host the specified UUID**.

---

## Why use WinLRPC-Hunter instead of rpcdump?
| Feature                    | rpcdump                     | WinLRPC-Hunter |
|----------------------------|------------------------------|----------------|
| Source                     | Endpoint Mapper              | NT Object Manager + live binding |
| LRPC detection             | Limited / often invisible    | Full coverage |
| UUID filtering             | Limited / none               | **Yes (Unique Selection Point)** |
| Session separation         | No                           | Yes |
| False positives            | High (stale registrations)   | Low (live endpoints only) |
| CLI output clarity         | Minimal                      | Yes |
| JSON output                | No                           | Yes |
| Concurrency control        | No                           | Yes |
| Interface version filter   | No                           | Yes |
| Auth type selection        | No                           | Yes |

---

## Usage
```powershell
WinLRPC-Hunter.exe <IFUUID> [--session <id>] [--timeout <ms>] [--endpoint <name>] [--no-skip] [--json] [--concurrency <1-128>] [--ifver <pred>] [--auth auto|none|ntlm|negotiate|kerberos|all]
```

### Arguments
- `<IFUUID>`: Target interface UUID (required)
- `--session <id>`: Scan only a specific session ID
- `--timeout <ms>`: RPC call timeout in milliseconds (default: 2000)
- `--endpoint <name>`: Probe only a specific endpoint by name
- `--no-skip`: Do not skip default ignored endpoints (e.g., LSA)
- `--json`: Output results in JSON format
- `--concurrency <1-128>`: Number of concurrent scan threads (default: CPU cores)
- `--ifver <pred>`: Filter by interface version (e.g., `>=1.0`)
- `--auth <mode>`: RPC authentication type (`auto`, `none`, `ntlm`, `negotiate`, `kerberos`, `all`)

---

## Example
```powershell
WinLRPC-Hunter.exe 12345678-1234-1234-1234-123456789abc --json --concurrency 16 --ifver ">=1.0" --auth kerberos
```

Example output:
```
Scanning 336 endpoints for {12345678-1234-1234-1234-123456789abc} (timeout=2000ms)
Scope   Session  Endpoint                                                           IfVersion  Protocol
-------------------------------------------------------------------------------------------------------
Global  -        MyEndpointName                                                     1.0        ncalrpc
Session 1        MyOtherEndpoint                                                    1.0        ncalrpc

Summary: total=336, scanned=329, skipped=7, hits=2
```

JSON mode example:
```json
{
  "scope": "Global",
  "sessionId": null,
  "endpoint": "MyEndpointName",
  "ifVersion": "1.0",
  "protocol": "ncalrpc",
  "uuid": "12345678-1234-1234-1234-123456789abc"
}
```

---

## Build
### Prerequisites
- **Windows 10/11**
- **MSVC (Visual Studio 2022 or later)** with C++20 enabled
- **CMake** (for modular build system)

### Build Steps
```powershell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Linking
- **Libraries**: `rpcrt4.lib`, `ntdll.lib`

---

## License
MIT
