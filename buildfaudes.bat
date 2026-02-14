echo "================================== build libFAUDES with MSVC toolchain"

echo "=== (need to configure sources before e.g. in an MSYS environment)"
set FAUDES_SRC=.\libFAUDES_source
set FAUDES_LIB=.\libFAUDES_win

echo "=== sanity check for libFAUDES working directory"
if not exist %FAUDES_SRC%\include\cfl_baseset.h (
  echo "ERROR: invalid libFAUDES source"
  exit /b
)

echo "=== prepare detination"
del /S /Q %FAUDES_LIB%
rmdir /S /Q %FAUDES_LIB%
mkdir %FAUDES_LIB%
xcopy /E /I /Y /Q %FAUDES_SRC% %FAUDES_LIB%\build


echo "==== borrow GNU make, e.g. from MSYS2 installation"
set GNUMAKE=C:\msys64\ucrt64\bin\mingw32-make.exe 

echo "==== set paths for MSVC command line tools"
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64"

echo "==== build libFAUDES static"
%GNUMAKE% -C %FAUDES_LIB%\build FAUDES_PLATFORM=cl_win clean
%GNUMAKE% -C %FAUDES_LIB%\build FAUDES_PLATFORM=cl_win libfaudes FAUDES_LINKING="static" -j 

echo "==== validate build"
if not exist %FAUDES_LIB%\build\faudes.lib (
  echo "ERROR: no faudes.lib found"
  exit /b
)

echo "=== harvest result and clean up"
copy %FAUDES_LIB%\build\faudes.lib %FAUDES_LIB%
xcopy /E /I /Y /Q %FAUDES_LIB%\build\include %FAUDES_LIB%\include
del /S /Q %FAUDES_LIB%\build
rmdir /S /Q %FAUDES_LIB%\build
