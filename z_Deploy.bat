@ROBOCOPY %cd% "%cd%\warper-bin\build" *.c *.h *.ico *.sh a_makeWindows.bat Makefile
@ROBOCOPY %cd% "%cd%\warper-bin\execute" *.txt *.png *.ttf *.ogg *.mp3 /MIR /XD dirs "%cd%\warper" "%cd%\warper-bin" "%cd%\.github" "%cd%\.git" "%cd%\bin" "%cd%\builds" "%cd%\documentation" "%cd%\obj" "%cd%\media" "%cd%\docs"
@cd warper-bin\build
make thenClean
@copy %cd%\*.a %cd%\..\..\warper
@move %cd%\*.a %cd%\..
@move %cd%\*.exe %cd%\..\..\warper\execute
@cd ..\..
@del %cd%\builds\warper-bin.zip
@ROBOCOPY %cd% "%cd%\warper\execute" *.txt *.png *.ttf *.ogg *.mp3 /MIR /XD dirs "%cd%\warper" "%cd%\warper-bin" "%cd%\.github" "%cd%\.git" "%cd%\bin" "%cd%\builds" "%cd%\documentation" "%cd%\obj" "%cd%\media" "%cd%\docs" "%cd%\saves"
@cd warper-bin
@del %cd%\execute\Warper.exe
@rem zip it up...
@%cd%\..\7za.exe a -y -tzip "%cd%\..\builds\warper-bin.zip" "%cd%" -mx5
@rem done...
@del %cd%\..\builds\warper.zip
@rem zip it up...
cd ..
@%cd%\7za.exe a -y -tzip "%cd%\builds\warper.zip" "%cd%\warper" -mx5
@rem done...