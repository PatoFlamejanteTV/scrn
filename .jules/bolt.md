## 2024-05-23 - GDI Resource Caching
**Learning:** In Windows GDI applications, creating `HDC` (Device Context) and `HBITMAP` objects is an expensive system call. For continuous screen capture, re-allocating these resources every frame introduces significant CPU overhead.
**Action:** Cache `HDC` (Memory DC) and `HBITMAP` using `static` variables (or member variables) to reuse them across frames. However, always `ReleaseDC` the primary screen DC if it's a "Common DC" obtained via `GetDC(NULL)`, as holding it can lock system resources. Only cache the memory-backed resources you own.
