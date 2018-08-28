@ROBOCOPY %cd% "%cd%\cs-bin\build" *.c *.h *.ico *.sh a_makeWindows.bat Makefile
@ROBOCOPY %cd% "%cd%\cs-bin\execute" *.txt *.png *.ttf *.ogg *.mp3 /MIR /XD dirs "%cd%\cs" "%cd%\cs-bin" "%cd%\.github" "%cd%\.git" "%cd%\bin" "%cd%\builds" "%cd%\documentation" "%cd%\obj" "%cd%\media" "%cd%\docs"
@cd cs-bin\build
make thenClean
make example clean
@copy %cd%\*.a %cd%\..\..\cs
@move %cd%\*.a %cd%\..
@move %cd%\*.exe %cd%\..\..\cs\execute
@cd ..\..
@del %cd%\builds\cs-bin.zip
@ROBOCOPY %cd% "%cd%\cs\execute" *.txt *.png *.ttf *.ogg *.mp3 /MIR /XD dirs "%cd%\cs" "%cd%\cs-bin" "%cd%\.github" "%cd%\.git" "%cd%\bin" "%cd%\builds" "%cd%\documentation" "%cd%\obj" "%cd%\media" "%cd%\docs" "%cd%\saves"
@cd cs-bin
@rem zip it up...
@%cd%\..\7za.exe a -y -tzip "%cd%\..\builds\cs-bin.zip" "%cd%" -mx5
@rem done...
@del %cd%\..\builds\cs.zip
@rem zip it up...
cd ..
@%cd%\7za.exe a -y -tzip "%cd%\builds\cs.zip" "%cd%\cs" -mx5
@rem done...