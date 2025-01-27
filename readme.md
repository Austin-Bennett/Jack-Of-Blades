# Jack Of Blades

code is in the source folder **besides** UIDesigner.cpp, which is a little program to help me place UI features

The 2 executables `main.exe` (the game) and `UIDesigner.exe` ive compiled with static-libc++ and static-libgcc and all the necessary dll's are included, they are in the static executables folder

**If you need to compile it on your machine so it will work heres how:**

Download msys64 from [here](https://www.msys2.org/), or follow this guide from [microsoft](https://code.visualstudio.com/docs/cpp/config-mingw)
If you followed the guide so you have mingw installed you're good, if not open MSYS2 and run this command:
`pacman -S mingw-w64-ucrt-x86_64-toolchain`

then you need to run this command for raylib (the graphics library I use):
`pacman -S mingw-w64-x86_64-raylib`

and finally theres an included zip folder called 'Austin Utils', its my custom utility library and all you have to do is drag all the files inside the 'include' folder into msys64/mingw64/include

finally, run this command to compile:
`g++ -Wall -Werror -Wno-class-memaccess -g -std=c++20 -o UIDesigner.exe UIDesigner.cpp -I C:\msys64\mingw64\include -L C:\msys64\mingw64\lib -lraylib -lopengl32 -lgdi32 -lwinmm -L. -lAustinUtils`

and then just `./main`