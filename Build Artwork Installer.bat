@Echo off
goto getadminwrites >NUL

:start
CD "%~dp0"
CALL:STARTTIME
CLS
COLOR 60
TITLE Build Gamers ( Xbox Artwork Installer Edits )

SET XBE_PATCH=tools\xbepatch\xbepatch.exe

SET COMPRESS_FILE=XBMC4XBOX.zip
SET COMPRESS=C:\Program Files\7-zip\7z.exe
SET COMPRESS_OPTS=a %COMPRESS_FILE%

SET Silent=0
SET SkipCompression=0
SET Clean=1
SET Compile=1

IF "%VS71COMNTOOLS%"=="" (
  SET NET="%ProgramFiles%\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.com"
) ELSE (
  SET NET="%VS71COMNTOOLS%\..\IDE\devenv.com"
)

IF NOT EXIST %NET% (
  CALL:ERROR "Visual Studio .NET 2003 was not found."
  GOTO:EOF
)

SET "DEST=BUILD Xbox Artwork Installer"

SET VS_PATH=.
SET VS_SOL=xbmc_artwork_installer.sln
SET VS_CONF=Release
SET VS_BIN=default.xbe

IF %Silent% EQU 0 (
  IF EXIST %VS_PATH%\%VS_CONF%\%VS_BIN% (
    SET Clean=0
  )
)

IF %Compile% EQU 1 (
  CALL:COMPILE
)

CALL:MAKE_BUILD %DEST%

pause
GOTO:EOF
  
:COMPILE
  ECHO Wait while preparing the build.
  ECHO ------------------------------------------------------------
  Echo CStdString strOutPutPathHeaderFile("E:/TDATA/Rocky5 needs these Logs/Xbox Artwork Installer/");>"xbmc\lib\libPython\XBPyErrorPath.h"
  IF %Clean% EQU 1 (
    ECHO Cleaning Solution...
    %NET% %VS_PATH%\%VS_SOL% /clean %VS_CONF%
    DEL %VS_PATH%\%VS_CONF%\xbmc.map 2>NUL
  )
  ECHO Compiling Solution...
  call "Fix GIT Empty Folders.bat"
  %NET% %VS_PATH%\%VS_SOL% /build %VS_CONF%
  IF NOT EXIST %VS_PATH%\%VS_CONF%\%VS_BIN% (
    CALL:ERROR "%VS_BIN% failed to build!  See .\%VS_CONF%\BuildLog.htm for details."
    PAUSE
    EXIT
  )
  ECHO Done!
  ECHO ------------------------------------------------------------
  GOTO:EOF

:MAKE_BUILD

  RMDIR "%DEST%" /S /Q
  if not exist "%DEST%" md "%DEST%"
  
  ECHO Copying files to %DEST% ...
  
  xcopy /Y "%VS_PATH%\%VS_CONF%\%VS_BIN%" "%DEST%\"
  
  IF "%DEST%" EQU "%DEST%" (
    ECHO - XBE Patching %VS_PATH%\%VS_CONF%\%VS_BIN%
    %XBE_PATCH% "%DEST%\%VS_BIN%"
    ECHO - Patching Done!
  )
  
  CALL:STOPTIME
  
  if "%clean%"=="0" pause & Goto start
  GOTO:EOF

:STARTTIME
  for /F "tokens=1-4 delims=:,." %%a in ("%time%") do (
    set startHour=%%a
    set startMinute=%%b
    set startSecond=%%c
    set startMillisecond=%%d
  )
  GOTO:EOF

:STOPTIME
  for /F "tokens=1-4 delims=:,." %%a in ("%time%") do (
    set endHour=%%a
    set endMinute=%%b
    set endSecond=%%c
    set endMillisecond=%%d
  )

  set /A startTotal=(startHour * 360000) + (startMinute * 6000) + (startSecond * 100) + startMillisecond
  set /A endTotal=(endHour * 360000) + (endMinute * 6000) + (endSecond * 100) + endMillisecond

  if %endTotal% LSS %startTotal% (
    set /A endTotal+=8640000  REM 24 hours in milliseconds
  )

  set /A elapsed=endTotal - startTotal

  REM Convert elapsed time to hours, minutes, seconds
  set /A hours=elapsed / 360000
  set /A minutes=(elapsed %% 360000) / 6000
  set /A seconds=(elapsed %% 6000) / 100

  TITLE Build Gamers ( Xbox Artwork Installer Edits ) Build Time: %minutes% Mins %seconds% Secs
  GOTO:EOF
  
:ERROR
  ECHO ------------------------------------------------------------
  ECHO !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-
  ECHO    ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR ERROR
  ECHO !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-
  ECHO ERROR %DEST%
  ECHO ------------------------------------------------------------
  GOTO:EOF
  
:GETADMINWRITES
  REM  --> Check for permissions
  IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
  >nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
  ) ELSE (
  >nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
  )
  REM --> If error flag set, we do not have admin.
  if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPROMPT
  ) else ( goto GOTADMIN )

  :UACPROMPT
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params = %*:"=""
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params%", "", "runas", 1 >> "%temp%\getadmin.vbs"
    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

  :GOTADMIN
    pushd "%CD%"
    CD /D "%~dp0"
     goto start