@echo off
REM Move to parent directory
cd ..

REM Delete generated files
del /Q Makefile
del /Q moc_*.*
del /Q qmake_*.*
del /Q qmlcache_*.*
del /Q qrc_*.*
del /Q src_*.*
del /Q *.o
del /Q *.moc
del /Q target_*.sh

REM Delete build directories
rmdir /S /Q release
rmdir /S /Q debug
rmdir /S /Q profile

echo Clean completed.
pause
