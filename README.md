# Code Finder
This program scans the games memory looking for the partial **ASBZKeypadBase** class signiture with a multithreaded array of bytes scan. With an offset of 0x372, which is only relative to my scan as the initial bytes of the class change as they form a pointer to some dynamic memory region, the **ASBZKeypadBase.Code** field can be read.

This is cheating so don't use this program if you don't want to be a cheater :/
