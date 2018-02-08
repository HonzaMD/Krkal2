@echo off
echo CREATE CURRENT SETUP
echo. 
echo This will copy binaries from ..\..\bin to ..\..bin2
echo ..\..bin2 will be deleted!
echo it will filter out subversion shits and also !OldLevels! directory
echo.
echo Setup executeble will be created in ..\..\ (trunk root)
echo.
pause


call createtempbin.cmd ..\..\bin
"C:\Program Files\NSIS\makensis.exe" fullcurrentbin.nsi

pause

:konec