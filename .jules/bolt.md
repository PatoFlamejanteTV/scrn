## 2024-05-23 - GDI Resource Caching
**Learning:** In Windows GDI applications, creating `HDC` (Device Context) and `HBITMAP` objects is an expensive system call. For continuous screen capture, re-allocating these resources every frame introduces significant CPU overhead.
**Action:** Cache `HDC` (Memory DC) and `HBITMAP` using `static` variables (or member variables) to reuse them across frames. However, always `ReleaseDC` the primary screen DC if it's a "Common DC" obtained via `GetDC(NULL)`, as holding it can lock system resources. Only cache the memory-backed resources you own.
## 2026-01-05 - Lookup Table Optimization
**Learning:** Precomputing a lookup table for ASCII mapping (replacing per-pixel division) yielded a ~14% speedup in benchmarks. However, the theoretical max index for the lookup table must be carefully validated against buffer size, requiring clamping to 255 to ensure safety even with valid input ranges.
**Action:** Always clamp array indices when using lookup tables for calculated values, even if the math 'should' be safe, to prevent buffer overflows.
