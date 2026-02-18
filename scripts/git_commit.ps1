
# Git commit script
Set-Location "e:\3d"

Write-Host "Adding changes..." -ForegroundColor Green
git add .

Write-Host "Committing changes..." -ForegroundColor Green
git commit -m "Update README and add push scripts"

Write-Host "Pushing to GitHub..." -ForegroundColor Green
git push -u origin master
