# R6-Dumper-External

External Cheat, Offset dumper for the external cheat, and a driver to get the cheat and dumper to work.

What else do you need?

!-This source is leaked and not made by me-!

## تحلیل فارسی
برای مطالعه گزارش فارسی در مورد نحوه عملکرد دامپر و چیت، فایل [ANALYSIS_FA.md](ANALYSIS_FA.md) را مطالعه کنید.

## Technical Analysis (English Summary)
- **r6dumper**: Uses multi-threaded signature scanning and string XRef analysis to locate memory offsets.
- **memdrv**: A kernel-level driver used to bypass anti-cheat protections and provide high-level memory access.
- **r6external**: An external cheat that utilizes the driver to read/write game memory for features like Glow ESP, No Recoil, and more.
