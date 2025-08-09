# WinLRPC-Hunter

**Targeted UUID scanner for Windows Local RPC endpoints**

WinLRPC-Hunter is a command-line tool for scanning and probing **Windows Local RPC (ncalrpc)** endpoints to locate a specific interface UUID.  
It enumerates both global and per-session `\RPC Control` directories, connects to each endpoint, and checks for a live match using `RpcMgmtInqIfIds`.  
This helps security researchers, reverse engineers, and developers quickly identify which endpoints are hosting the target interface.

---

## Features
- Enumerates global and session-specific Local RPC (`ncalrpc`) endpoints
- Connects to each endpoint and verifies live interface UUIDs
- **Unique Selection Point**: scans **only** for the specified UUID and ignores all others
- Detects unregistered but active LRPC endpoints
- Differentiates Global and Session namespaces in output
- Clean CLI output with hit summary

---

## What is "Unique Selection Point"?
Unlike general RPC scanners that list every interface, WinLRPC-Hunter focuses solely on **one target UUID** you provide.  
This targeted approach:
- Removes noise from unrelated endpoints  
- Speeds up scanning in large environments  
- Reduces false positives when hunting a specific RPC interface  

### Diagram

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
| UUID only              |
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

---

## Usage
```powershell
WinLRPC-Hunter.exe <IFUUID> [--session <id>] [--timeout <ms>] [--endpoint <name>] [--no-skip]
```

### Arguments
- `<IFUUID>`: Target interface UUID
- `--session <id>`: Scan only a specific session
- `--timeout <ms>`: RPC call timeout in milliseconds (default: 2000)
- `--endpoint <name>`: Probe only a single endpoint by name
- `--no-skip`: Do not skip default ignored endpoints (LSA, etc.)

---

## Example
```powershell
WinLRPC-Hunter.exe <uuid> --no-skip
```

Example output:
```
Scanning 336 endpoints for <uuid>  (timeout=2000ms)
Scope   Session  Endpoint                                                           IfVersion  Protocol
-------------------------------------------------------------------------------------------------------
Global  -        xxx                                                                1.0        ncalrpc
Session 1        xxx                                                                1.0        ncalrpc

Summary: total=336, scanned=329, skipped=7, hits=2
```

---

## Build
Compile with MSVC:
```powershell
cl /std:c++20 /O2 /EHsc WinLRPC-Hunter.cpp /link rpcrt4.lib ntdll.lib
```

---

## License
MIT
