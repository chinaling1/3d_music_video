
@echo off
echo ====================================
echo   Organize Project Structure
echo ====================================
echo.

cd /d "e:\3d"

echo [1/4] Creating scripts directory...
if not exist "scripts" mkdir scripts
echo.

echo [2/4] Moving script files to scripts/...
move /Y build_and_test.bat scripts\
move /Y check_env.bat scripts\
move /Y check_env_en.bat scripts\
move /Y cleanup_and_push.bat scripts\
move /Y fix_cleanup.bat scripts\
move /Y git_commit.ps1 scripts\
move /Y push_to_gitee.bat scripts\
move /Y push_to_github.bat scripts\
move /Y quick_test.bat scripts\
move /Y run_tests.bat scripts\
move /Y run_tests.sh scripts\
move /Y verify_installation.bat scripts\
move /Y organize_project.bat scripts\
echo.

echo [3/4] Git status...
git status
echo.

echo [4/4] Committing changes...
git add -u
git add scripts/
git commit -m "Organize project - move scripts to scripts/ directory"
echo.

echo ====================================
echo   Done!
echo ====================================
echo.
echo Now you can push using:
echo   git push -u gitee master
echo   git push -u origin master
echo.
pause
