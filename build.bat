@echo off
setlocal
set "src_dir=%~dp0"
set cflags=/Zi /nologo /W4 /MD /D_CRT_SECURE_NO_WARNINGS /I%src_dir% /Fetarball_extract.exe
if not exist build\ mkdir build
pushd build
cl %cflags% %src_dir%main.c %src_dir%microtar.c %src_dir%zlib.lib kernel32.lib
popd
