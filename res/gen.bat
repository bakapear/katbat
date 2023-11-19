@echo off

pushd "%~dp0"

dir icons /b /a-d > list.txt

echo MAINICON ICON icon.ico > resources.rc

for /f "tokens=*" %%a in ('type list.txt') do (
    >>resources.rc echo %%a ICON icons/%%a
)

del list.txt

windres resources.rc -O coff -o resources.res

del resources.rc

popd