set PATH=%PATH%;c:\Program Files (x86)\Windows Kits\10\bin\10.0.15063.0\x86\;c:\Program Files (x86)\NSIS\Bin

del makeuninst.exe
del uninst.exe

signtool sign /tr http://timestamp.comodoca.com /td sha256 /fd sha256 /a "..\..\bin\Krkal.exe"
if ERRORLEVEL 1 goto error

makensis.exe -DOutputUninst krkal25.nsi
if ERRORLEVEL 1 goto error

makeuninst.exe

signtool sign /tr http://timestamp.comodoca.com /td sha256 /fd sha256 /a uninst.exe
if ERRORLEVEL 1 goto error

makensis.exe -DUseUninst krkal25.nsi
if ERRORLEVEL 1 goto error

signtool sign /tr http://timestamp.comodoca.com /td sha256 /fd sha256 /a Krkal25.exe
if ERRORLEVEL 1 goto error

goto end
:error
echo DOSLO K CHYBE!

:end