@echo off


call  "\\DESKTOP-MLCR1I9\E$\Program Files\Polyspace\R2021a\bin\win64\checkMATLABRootForDriveMap.exe" "\\DESKTOP-MLCR1I9\E$\Program Files\Polyspace\R2021a"  > mlEnv.txt
for /f %%a in (mlEnv.txt) do set "%%a"\n
cd .

if "%1"=="" ("E:\Program Files\Polyspace\R2021a\bin\win64\gmake" MATLAB_ROOT=%MATLAB_ROOT% ALT_MATLAB_ROOT=%ALT_MATLAB_ROOT% MATLAB_BIN=%MATLAB_BIN% ALT_MATLAB_BIN=%ALT_MATLAB_BIN%  -f cal_corr_rtw.mk all) else ("E:\Program Files\Polyspace\R2021a\bin\win64\gmake" MATLAB_ROOT=%MATLAB_ROOT% ALT_MATLAB_ROOT=%ALT_MATLAB_ROOT% MATLAB_BIN=%MATLAB_BIN% ALT_MATLAB_BIN=%ALT_MATLAB_BIN%  -f cal_corr_rtw.mk %1)
@if errorlevel 1 goto error_exit

exit /B 0

:error_exit
echo The make command returned an error of %errorlevel%
exit /B 1