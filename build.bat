@echo off
setlocal
set "src_dir=%~dp0"
if not exist build\ mkdir build
pushd build
cl /Zi /nologo /W4 /MD /D_CRT_SECURE_NO_WARNINGS /I%src_dir% %src_dir%main.c %src_dir%microtar.c %src_dir%zlib.lib
popd
