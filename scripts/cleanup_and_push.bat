
@echo off
echo ====================================
echo   V3D Studio - Cleanup and Push
echo ====================================
echo.

cd /d "e:\3d"

echo [1/5] Creating personal info directory...
if not exist "E:\个人信息" mkdir "E:\个人信息"
echo.

echo [2/5] Moving personal info files...
echo Moving report files...
move /Y FINAL_BUILD_REPORT*.md "E:\个人信息\" 2>nul
move /Y BUILD_*.md "E:\个人信息\" 2>nul
move /Y FINAL_PROJECT*.md "E:\个人信息\" 2>nul
move /Y PROJECT_*.md "E:\个人信息\" 2>nul
move /Y ISSUES_*.md "E:\个人信息\" 2>nul
move /Y TEST_*.md "E:\个人信息\" 2>nul
move /Y USER_GUIDE.md "E:\个人信息\" 2>nul
move /Y INSTALLATION_GUIDE.md "E:\个人信息\" 2>nul
move /Y QUICK_START.md "E:\个人信息\" 2>nul
echo.

echo [3/5] Git status...
git status
echo.

echo [4/5] Committing changes...
git add -A
git commit -m "Cleanup - remove personal info files, keep README and LICENSE"
echo.

echo [5/5] Pushing to repositories...
echo Pushing to Gitee...
git push -u gitee master
echo.
echo Pushing to GitHub...
git push -u origin master
echo.

echo ====================================
echo   Done!
echo ====================================
pause
