cp -f -r ./* ../../james-bern.github.io/
cd ..
exec ./poosh.bat
cd ../james-bern.github.io/
git add --all
git commit -m "CS3XX poosh.bat"
git push
