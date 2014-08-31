@echo off
if "%1"=="" goto errorlabel

echo.
echo = = = DELETING %12 = = =
rd "%12" /s /q

echo.
echo = = = COPY = = =
md "%12"
xcopy "%1\*.*" "%12\" /e

echo.
echo = = = DELETING %12\!OldLevels! = = =
rd "%12\!OldLevels!" /s /q

echo.
echo = = = DELETING %12\Profiles = = =
del "%12\Profiles\." /s /q


echo.
echo copy successful
echo.
goto endlabel

:errorlabel
echo.
echo param1 not specified
echo.

pause

:endlabel
