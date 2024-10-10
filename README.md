# Code Finder
This program scans the games memory looking for the partial **ASBZKeypadBase** class signiture with a multithreaded array of bytes scan. With an offset of 0x372, which is only relative to my scan as the initial bytes of the class change as they form a pointer to some dynamic memory region, the **ASBZKeypadBase.Code** field can be read.

To build this, download the [GCC/MinGW](https://winlibs.com/#download-release) binaries and run build.bat (don't forget to add the bin folder to the PATH system environment variable). If you need more help, [this](https://www.youtube.com/watch?v=k6juv3mIr9o) video shows you how to do it step-by-step.

This is cheating so don't use this program if you don't want to be a cheater :/
