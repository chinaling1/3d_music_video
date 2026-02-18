
# Git commit script
Set-Location "e:\3d"

Write-Host "Committing changes..." -ForegroundColor Green
git commit -m "Initial commit - V3D Studio with 3D modeling, animation, audio, and AI features"

Write-Host "Pushing to GitHub..." -ForegroundColor Green
git push -u origin main
