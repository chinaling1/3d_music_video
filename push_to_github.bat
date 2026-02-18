
@echo off
echo ====================================
echo   V3D Studio - Git Push to GitHub
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

echo [4/4] Pushing to GitHub...
git push -u origin master
echo.

echo ====================================
echo   Done!
echo ====================================
pause
