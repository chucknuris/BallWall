# BallWall

Files:
	ballwall.cpp             (main code)
	loadCompileShaders.cpp   (code to load and compile shaders)
	camera.cpp               (class camera code)
	loadCompileShaders.h     (header file)
	camera.h                 (header file)
	shader.vert              (vertex shader code)
	shader.frag              (fragment shader code)
	makefile                 (MacOS g++ makefile)
	makefileLinux.mak        (Linux g++ makefile)
	makefileMinGW-w64.mak    (MSYS2 - MinGW-w64 makefile)

Folders:
	vsproject                (contains the MSVC solution and project files)
	glew-2.2.0               (glew v2.2.0 MSVC library for 32 and 64 bits)
	freeglut                 (freeglut v3.0.0 MSVC library for 32 and 64 bits)



To compile in MacOS (not tested):

in a terminal console move to the folder where the code are, and type: make



To compile in Linux (not tested):

Install freeglut and glew devel packages
in a terminal console move to the folder where the code are, and type: make -f makefileLinux.mak



To compile with MSYS2 - MinGW-w64 (Windows):

Open the msys64\msys2 console
To install freeglut type: pacman -S mingw-w64-x86_64-freeglut
To install glew type: pacman -S mingw-w64-x86_64-glew
close the msys2 console.
Open the msys64\mingw64 console
using cd command, move to the folder where the code are, and type: make -f makefileMinGW-w64.mak



Compiling with Visual Studio Community 2019:

Open the file vsproject\ballwall.sln
Select Configuration Release and Platform x64. Build the solution.
To run the program from the Visual Studio Console, first copy the shader.vert, shader.frag and freeglut\bin\x64\freeglut.dll files to the folder vsproject.
To run the program from the file explorer, first copy the file freeglut\bin\x64\freeglut.dll to the folder where the executable is located.

glew MSVC library downloaded from:
https://github.com/nigels-com/glew
Windows binaries for 32-bit and 64-bit

freeglut MSVC library downloaded from:
https://www.transmissionzero.co.uk/software/freeglut-devel/
freeglut 3.0.0 MSVC Package