@powershell "./gig c++,visualstudio"
@rename .gitignore .old_gitignore
@echo ### Created by gitignore.bat, using gig.ps1 and MyIgnores.txt ### > .gitignore
@type .old_gitignore >> .gitignore
@type MyIgnores.txt >> .gitignore
@del .old_gitignore
@move .gitignore ../.gitignore
