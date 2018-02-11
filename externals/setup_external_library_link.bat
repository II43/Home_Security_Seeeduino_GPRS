@echo off
setlocal
echo Setting up symbolic links in Arduino library folders for the externals
echo This is required as there seems to be noother way how to make Arduino IDE aware of these libraries
echo. 
echo Geting path to my documents folder:
for /f "tokens=3* delims= " %%a in ('reg query "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" /v "Personal"') do (set documentsfolder=%%a)
echo %documentsfolder%
echo.
set libfolder=%documentsfolder%\Arduino\libraries
IF NOT EXIST "%libfolder%" (
	echo Cannot find user library folder:
	echo %libfolder%
	echo.
	echo Failed!
	EXIT /B 1
echo Yes 
) ELSE (
	echo Setting up links
	
	IF NOT EXIST "%libfolder%\Adafruit_SleepyDog" (
		echo Adafruit_SleepyDog to "%libfolder%\Adafruit_SleepyDog"
		mklink /J "%libfolder%\Adafruit_SleepyDog" ".\Adafruit_SleepyDog"
	) ELSE (
		echo Adafruit_SleepyDog in "%libfolder%\Adafruit_SleepyDog" already exists - no action
	)
	IF NOT EXIST "%libfolder%\Adafruit_SleepyDog" (
		echo Failed to setup "%libfolder%\Adafruit_SleepyDog"
		echo.
		echo Failed!
		EXIT /B 2
	)
	
	IF NOT EXIST "%libfolder%\SdFat" (
		echo SdFat to "%libfolder%\SdFat"
		mklink /J "%libfolder%\SdFat" ".\SdFat"
	) ELSE (
		echo SdFat in "%libfolder%\SdFat" already exists - no action
	)
	IF NOT EXIST "%libfolder%\SdFat" (
		echo Failed to setup "%libfolder%\SdFat"
		echo.
		echo Failed!
		EXIT /B 2
	)
	
	IF NOT EXIST "%libfolder%\Seeeduino_GPRS" (
		echo Seeeduino_GPRS to "%libfolder%\Seeeduino_GPRS"
		mklink /J "%libfolder%\Seeeduino_GPRS" ".\Seeeduino_GPRS"
	) ELSE (
		echo Seeeduino_GPRS in "%libfolder%\Seeeduino_GPRS" already exists - no action
	)
	IF NOT EXIST "%libfolder%\Seeeduino_GPRS" (
		echo Failed to setup "%libfolder%\Seeeduino_GPRS"
		echo.
		echo Failed!
		EXIT /B 2
	)
)

echo.
echo Done!
endlocal