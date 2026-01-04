## 2024-05-23 - GDI Resource Caching
**Learning:** In Windows GDI applications, creating `HDC` (Device Context) and `HBITMAP` objects is an expensive system call. For continuous screen capture, re-allocating these resources every frame introduces significant CPU overhead.
**Action:** Cache `HDC` (Memory DC) and `HBITMAP` using `static` variables (or member variables) to reuse them across frames. However, always `ReleaseDC` the primary screen DC if it's a "Common DC" obtained via `GetDC(NULL)`, as holding it can lock system resources. Only cache the memory-backed resources you own.

## 2025-05-23 - ASCII Conversion Lookup Table
**Learning:** Performing arithmetic (multiplication/division) to map a grayscale value (0-255) to a ramp index for every pixel in a 1080p+ equivalent buffer is a major bottleneck (~60% of frame time).
**Action:** Precompute the mapping into a `char lookup[256]` array. Combined with direct buffer access (avoiding `string::operator+=`), this yields a ~2.2x speedup in the conversion loop.
