@echo off
echo Formatting all C++ files recursively...

rem Formatting C++ files specifically in the "src" folder (and all its subfolders, if it exists)
if exist "src\" (
    for /R "src" %%f in (*.cpp *.h *.hpp) do (
        if exist "%%f" (
            echo Formatting: "%%f"
            clang-format -i "%%f"
        )
    )
)

rem Formatting C++ files specifically in the "include" folder (and all its subfolders, if it exists)
if exist "include\" (
    for /R "include" %%f in (*.cpp *.h *.hpp) do (
        if exist "%%f" (
            echo Formatting: "%%f"
            clang-format -i "%%f"
        )
    )
)

echo.
echo Operation complete.
pause
