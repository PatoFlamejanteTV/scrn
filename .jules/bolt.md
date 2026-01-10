## 2024-05-23 - GDI Resource Caching
**Learning:** In Windows GDI applications, creating `HDC` (Device Context) and `HBITMAP` objects is an expensive system call. For continuous screen capture, re-allocating these resources every frame introduces significant CPU overhead.
**Action:** Cache `HDC` (Memory DC) and `HBITMAP` using `static` variables (or member variables) to reuse them across frames. However, always `ReleaseDC` the primary screen DC if it's a "Common DC" obtained via `GetDC(NULL)`, as holding it can lock system resources. Only cache the memory-backed resources you own.

## 2024-10-25 - ASCII Conversion Lookups
**Learning:** Performing integer division and multiplication inside a high-frequency pixel loop (240x80 pixels @ 60fps) adds up. Since the input range (grayscale 0-255) is small and fixed, a precomputed lookup table `vector<char>` is significantly faster than repeated arithmetic.
**Action:** Whenever mapping a small, bounded integer range to static values (like ASCII chars), always precompute a lookup table outside the hot loop.

## 2024-10-26 - Buffer Construction Optimization
**Learning:** `std::string::operator+=` inside a tight loop (e.g., per-pixel) incurs significant overhead due to capacity checks and potential reallocations.
**Action:** For fixed-size buffers, use `resize()` once and populate data using direct pointer access or iterators to eliminate method call overhead.
