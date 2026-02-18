
@echo off
echo ====================================
echo   Commit and Push to GitHub
echo ====================================
echo.

cd /d "e:\3d"

echo [1/4] Moving finalize.bat to scripts/...
if exist "finalize.bat" move /Y finalize.bat scripts\
echo.

echo [2/4] Adding changes to git...
git add -A
echo.

echo [3/4] Committing...
git commit -m "Complete project organization - move all scripts to scripts/, update README"
echo.

echo [4/4] Pushing to GitHub...
git push -u origin master
echo.

echo ====================================
echo   Done!
echo ====================================
echo.
echo Now pushing to Gitee too...
git push -u gitee master
echo.
echo All done!
echo.
pause
