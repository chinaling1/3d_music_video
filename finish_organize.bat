
@echo off
echo ====================================
echo   Finish Organize and Push
echo ====================================
echo.

cd /d "e:\3d"

echo [1/3] Adding changes...
git add -A
echo.

echo [2/3] Committing...
git commit -m "Organize project structure - move scripts to scripts/ directory"
echo.

echo [3/3] Pushing to repositories...
echo Pushing to Gitee...
git push -u gitee master
echo.
echo Pushing to GitHub...
git push -u origin master
echo.

echo ====================================
echo   Done!
echo ====================================
echo.
echo Project structure now clean!
echo.
pause
