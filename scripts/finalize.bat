
@echo off
echo ====================================
echo   Finalize - Complete All Tasks
echo ====================================
echo.

cd /d "e:\3d"

echo [1/5] Moving remaining scripts to scripts/...
if exist "finish_organize.bat" move /Y finish_organize.bat scripts\
if exist "cleanup_last.bat" move /Y cleanup_last.bat scripts\
echo.

echo [2/5] Moving this script to scripts/...
move /Y finalize.bat scripts\
echo.

echo [3/5] Adding changes to git...
git add README.md README_EN.md
git add -u
echo.

echo [4/5] Committing...
git commit -m "Update README - new project structure with scripts/ directory"
echo.

echo [5/5] Pushing to repositories...
git push -u gitee master
git push -u origin master
echo.

echo ====================================
echo   All Done!
echo ====================================
echo.
echo Project is now clean and organized!
echo.
pause
