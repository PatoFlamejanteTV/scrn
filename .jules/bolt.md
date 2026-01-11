## 2024-05-23 - GDI Resource Caching
**Learning:** In Windows GDI applications, creating `HDC` (Device Context) and `HBITMAP` objects is an expensive system call. For continuous screen capture, re-allocating these resources every frame introduces significant CPU overhead.
**Action:** Cache `HDC` (Memory DC) and `HBITMAP` using `static` variables (or member variables) to reuse them across frames. However, always `ReleaseDC` the primary screen DC if it's a "Common DC" obtained via `GetDC(NULL)`, as holding it can lock system resources. Only cache the memory-backed resources you own.

## 2024-10-25 - ASCII Conversion Lookups
**Learning:** Performing integer division and multiplication inside a high-frequency pixel loop (240x80 pixels @ 60fps) adds up. Since the input range (grayscale 0-255) is small and fixed, a precomputed lookup table `vector<char>` is significantly faster than repeated arithmetic.
**Action:** Whenever mapping a small, bounded integer range to static values (like ASCII chars), always precompute a lookup table outside the hot loop.

## 2024-10-25 - String Construction Optimization
**Learning:** Repeatedly calling `std::string::operator+=` in a tight loop incurs significant overhead due to capacity checks and potential reallocations, even with `reserve()`. In benchmarking, replacing `+=` with `resize()` + direct pointer access provided a ~2x speedup (28ms vs 55ms per 1000 frames) for constructing the ASCII frame.
**Action:** For constructing large, fixed-size buffers frame-by-frame, resize the buffer once and write directly to it using a pointer or iterator, rather than using repeated append operations.
