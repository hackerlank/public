@echo off
rem ------make directory------
if exist sdks rmdir /S /Q sdks
if exist ..\sdks rmdir /S /Q ..\sdks
md sdks
cd sdks
if not exist keye md keye
cd keye
if not exist htio md htio
if not exist cache md cache
if not exist allocator md allocator
if not exist mysql_proxy md mysql_proxy
if not exist utility md utility
cd ..
if not exist lib md lib
cd ..
move /Y sdks ../
rem ------copy files------
xcopy /Y /S *.h ..\sdks\keye
copy /Y ..\builds\bin\keye.dll ..\sdks\lib
copy /Y ..\builds\bin\keye_d.dll ..\sdks\lib
copy /Y ..\builds\bin\keye.lib ..\sdks\lib
copy /Y ..\builds\bin\keye_d.lib ..\sdks\lib
