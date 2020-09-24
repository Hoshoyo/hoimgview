@echo off

pushd bin
call cl /MD /I../include /Zi /nologo /Fe:hoimgview.exe ../src/*.c /link ../lib/freetype.lib ../lib/GLFW/glfw3.lib ../icon.res kernel32.lib opengl32.lib user32.lib gdi32.lib shell32.lib
popd