@set project=Warper
@IF EXIST "%cd%\%project%\.git" (
	cd /d "%cd%\%project%\.git"
	for /F "delims=" %%i in ('dir /b') do (rmdir "%%i" /s/q || del "%%i" /s/q)
	cd ..
)

@git remote add origin https://github.com/123outerme/%project%.git
git pull origin master --allow-unrelated-histories
@rem @pause Done! Press anything to continue.