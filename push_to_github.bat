
@echo off
echo ====================================
echo   V3D Studio - Git Push to GitHub
echo ====================================
echo.

cd /d "e:\3d"

echo [1/3] Checking git status...
git status
echo.

echo [2/3] Creating initial commit...
git commit -m "Initial commit - V3D Studio with 3D modeling, animation, audio, and AI features"
echo.

echo [3/3] Pushing to GitHub...
git push -u origin main
echo.

echo ====================================
echo   Done!
echo ====================================
pause
