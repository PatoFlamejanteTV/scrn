## 2024-05-24 - [Compiler Hardening and API Safety]
**Vulnerability:**
1. Default CMake configurations often lack security hardening flags, leaving binaries vulnerable to stack buffer overflows and control flow hijacking.
2. The `GetDIBits` Windows API returns 0 on failure, but this return value was ignored, potentially leading to processing of uninitialized or stale buffer data.

**Learning:**
1. Security is not just code; it's also build configuration. Standard flags like `/GS` (MSVC) and `-fstack-protector` (GCC) should be standard for C++ projects.
2. Silent failure of system APIs (like `GetDIBits`) is a common source of robustness issues and potential information leaks (if stale buffer data is displayed).

**Prevention:**
1. Always add a "Security" section to `CMakeLists.txt` to enable platform-specific hardening flags.
2. Check return values of all system APIs, even "reliable" ones, to fail safe.

## 2024-05-25 - [Secure Memory Erasure]
**Vulnerability:**
Sensitive data (like screen captures) stored in standard containers (e.g., `std::vector`) remains in heap memory after the object is destroyed, potentially allowing data recovery by other processes or crash dumps.

**Learning:**
Standard C++ containers do not zero-out memory upon destruction.

**Prevention:**
Use a RAII wrapper (e.g., `SecureBuffer`) that explicitly calls platform-specific memory wiping functions (like `SecureZeroMemory` on Windows or `explicit_bzero` on Linux) in its destructor.
