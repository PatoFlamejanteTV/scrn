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

## 2024-05-26 - [Secure Reallocation with Custom Allocators]
**Vulnerability:**
Standard `std::vector` reallocation logic allocates a new block, moves elements, and then frees the old block. This freeing mechanism (using standard `delete`) does NOT wipe the old memory, leaving copies of sensitive data on the heap during buffer growth.

**Learning:**
A simple RAII wrapper around `std::vector` is insufficient for preventing data remanence during resize operations.

**Prevention:**
Use a custom C++ allocator (e.g., `SecureAllocator`) that overrides `deallocate` to call `SecureZeroMemory` (or equivalent) before freeing memory. This ensures that every time memory is returned to the OS—whether via destruction or reallocation—it is first wiped. Additionally, explicitly delete copy constructors/assignment operators on sensitive wrappers to prevent accidental data duplication.
