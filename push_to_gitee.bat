
@echo off
echo ====================================
echo   V3D Studio - Push to Gitee
echo ====================================
echo.

cd /d "e:\3d"

echo [1/3] Checking git status...
git status
echo.

echo [2/3] Committing changes...
git commit -m "Update README - add pre-compiled executable info"
echo.

echo [3/3] Pushing to Gitee...
git push -u gitee master
echo.

echo ====================================
echo   Done!
echo ====================================
pause
