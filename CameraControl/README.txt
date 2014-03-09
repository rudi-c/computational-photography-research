This software requires Visual Studio 2010 and Qt4, a framework 
for creating user interfaces, to be compiled. It has been tested
with Windows 7 and Windows 8.

Make sure to download an install "Qt4.X for Windows (VS 2010)" and
the "Visual Studio Add-in for Qt4". The program has no been tested
with Qt5 and would probably require additional configuration
to get it to work. In addition, Qt4 only supports Visual Studio 2010
or earlier, and not the express version. The full version of Visual
Studio can be downloaded for free on Dreamspark if you are a student.

Problem : Visual Studio generates compiler errors "Moc'ing *.h The
System cannot find the path specified."
Solution : There is a problem with the Qt installation. Make sure
Qt and the Qt add-in for Visual Studio are installed (Qt should
appear in menu bar).

Problem : Qt is installed but when compiling, Visual Studio says that
"There's no Qt veresion assigned to this project for platform Win32.
 Please use the 'change Qt version' feature and choose a valid Qt
 version for this platform."
Solution : As stated in the error message, assign a Qt version to this
project. To do so, right click on the project (CameraAdjunct) >
Qt Project Settings and choose the right item in the dropdown menu for
the Version field.

Problem : The program fails to find EDSDKTypes.h and/or EDSDK.h when
compiling or cannot open file EDSDK.lib
Solution : The files in the CameraAdjunct/CanonSDK folder are not in
the compiler's include paths. Right-Click on CameraAdjunct project
and change the settings in C/C++ > Additional Include Directories to
add the full path to that folder (can be abbreviated with
$(SolutionDir)$(ProjectName)\CanonSDK ). Similarly, the path needs
to be added to Linker > Additional Library Directories.
Problem : The program compiles, but when running it, an error message
appears stating that the program is unable to find the entry path
for some Qt package.
Solution : This often means that Visual Studio find multiple versions
of the same DLL and is unable to decide which one to use. This can
happen when multiple versions of Qt are installed or when the DLL in
the executable folder (CameraAdjunct/Debug or CameraAdjunct/Release)
do not match the ones of the Qt installation.

To fix this, delete the Debug and Release folder and in the Visual
Studio menu, Build > Clean Solution. Alternatively, copy the correct
DLLs into the executable folder.

Problem : The program compiles but fails to run as it fails to find
Qt DLLs or EDSDK.DLL.
Solution : Either copy the required DLLs into the executable folder,
or add the path to those DLLs in 
Project Properties > Debugging > Environment