@echo off
setlocal
set "src_dir=%~dp0"
if not exist build\ mkdir build
pushd build
cl /Zi /nologo /W4 /MD /I%src_dir% %src_dir%main.c %src_dir%zlib.lib
popd
