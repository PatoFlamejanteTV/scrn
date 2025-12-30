## 2024-10-24 - Interactive Console Controls
**Learning:** Adding interactive controls (like 'q' for quit) to a continuous CLI loop significantly improves user agency, but requires platform-specific handling (e.g., `<conio.h>` on Windows) that must be guarded to preserve cross-platform buildability.
**Action:** Always wrap `_kbhit()` and `_getch()` in `#ifdef _WIN32` and provide visual cues to the user about available controls.
