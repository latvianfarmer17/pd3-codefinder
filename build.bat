@echo off

cls

windres ./res/CodeFinder.rc -o ./res/CodeFinder.o

IF %ERRORLEVEL% NEQ 0 (
	pause
	exit /b %ERRORLEVEL%
)

IF NOT EXIST bin\ (mkdir bin)

gcc ./src/Main.c ./src/Array.c ./src/Memory.c ./res/CodeFinder.o -Wall -mwindows -Os -s -o ./bin/CodeFinder.exe

IF %ERRORLEVEL% NEQ 0 (
	pause
	exit /b %ERRORLEVEL%
)