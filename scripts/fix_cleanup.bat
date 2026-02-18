
@echo off
echo ====================================
echo   Fix Cleanup - Remove Personal Info
echo ====================================
echo.

cd /d "e:\3d"

echo [1/6] Creating personal info directory...
if not exist "E:\personal_info" mkdir "E:\personal_info"
echo.

echo [2/6] Removing files from git...
git rm --cached BUILD_*.md
git rm --cached FINAL_*.md
git rm --cached PROJECT_*.md
git rm --cached ISSUES_*.md
git rm --cached TEST_*.md
git rm --cached USER_GUIDE.md
git rm --cached INSTALLATION_GUIDE.md
git rm --cached QUICK_START.md
echo.

echo [3/6] Moving files to E:\personal_info...
move /Y BUILD_*.md "E:\personal_info\"
move /Y FINAL_*.md "E:\personal_info\"
move /Y PROJECT_*.md "E:\personal_info\"
move /Y ISSUES_*.md "E:\personal_info\"
move /Y TEST_*.md "E:\personal_info\"
move /Y USER_GUIDE.md "E:\personal_info\"
move /Y INSTALLATION_GUIDE.md "E:\personal_info\"
move /Y QUICK_START.md "E:\personal_info\"
echo.

echo [4/6] Updating .gitignore...
echo. >> .gitignore
echo # Personal info reports >> .gitignore
echo BUILD_*.md >> .gitignore
echo FINAL_*.md >> .gitignore
echo PROJECT_*.md >> .gitignore
echo ISSUES_*.md >> .gitignore
echo TEST_*.md >> .gitignore
echo USER_GUIDE.md >> .gitignore
echo INSTALLATION_GUIDE.md >> .gitignore
echo QUICK_START.md >> .gitignore
echo.

echo [5/6] Committing changes...
git add .gitignore
git add -u
git commit -m "Remove personal info files and update .gitignore"
echo.

echo [6/6] Pushing to repositories...
git push -u gitee master
git push -u origin master
echo.

echo ====================================
echo   Done!
echo ====================================
pause
