echo y | copy new\*.* .
echo y | copy /b ks_version.h		+ blank		ks_version.h

echo y | del ..\scripts\*.code
echo y | copy /b CreateCompiledScripts.no	+ blank	CreateCompiledScripts.h
