
@echo off
echo ====================================
echo   V3D Studio - Push to Gitee
echo ====================================
echo.

cd /d "e:\3d"

echo [1/4] Checking git status...
git status
echo.

echo [2/4] Adding changes...
git add .
echo.

echo [3/4] Committing changes...
git commit -m "Update README and add push scripts"
echo.

echo [4/4] Pushing to Gitee...
git push -u gitee master
echo.

echo ====================================
echo   Done!
echo ====================================
pause
