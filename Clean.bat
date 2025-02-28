@Echo off

:start
CD "%~dp0"
CALL:STARTTIME
CLS
COLOR 0A
TITLE Clean Gamers ( XBMC4Gamers Edits )

IF "%VS71COMNTOOLS%"=="" (
  SET NET="%ProgramFiles%\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.com"
) ELSE (
  SET NET="%VS71COMNTOOLS%\..\IDE\devenv.com"
)

IF NOT EXIST %NET% (
  CALL:ERROR "Visual Studio .NET 2003 was not found."
  GOTO:EOF
)

ECHO Wait while cleaning the builds.
ECHO ------------------------------------------------------------

SET VS_PATH=.
SET VS_SOL=xbmc.sln
SET VS_CONF=Release
CALL:COMPILE

SET VS_SOL=xbmc.sln
SET VS_CONF=Release_LTCG
CALL:COMPILE

CALL:MAKE_BUILD

pause
GOTO:EOF
  
:COMPILE
  del /q "xbmc\lib\libPython\XBPyErrorPath.h" 2>NUL
  ECHO Cleaning Solution...
  %NET% %VS_PATH%\%VS_SOL% /clean %VS_CONF%
  DEL %VS_PATH%\%VS_CONF%\xbmc.map 2>NUL
  GOTO:EOF

:MAKE_BUILD
  ECHO Done!
  ECHO ------------------------------------------------------------
  
  CALL:STOPTIME

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

  TITLE Clean Gamers ( XBMC4Gamers Edits ) Build Time: %minutes% Mins %seconds% Secs
  GOTO:EOF