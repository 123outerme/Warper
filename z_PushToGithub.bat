@set project=Warper
@set commitMessage=""
@IF [%1]==[] (
	set commitMessage=Updated
) ELSE (
	set commitMessage=%1
)
@set branch=""
@IF [%2]==[] (
	set branch=master
) ELSE (
	set branch=%2
)
@git add .
@git commit -m%commitMessage%
@git remote add origin https://github.com/123outerme/%project%
@git push origin %branch%
@pause Done! Press anything to continue.