@echo off
rem Public domain
rem http://unlicense.org/
rem Created by Grigore Stefan <g_stefan@yahoo.com>

SETLOCAL ENABLEDELAYEDEXPANSION

echo -^> github-update quantum-script-extension-crypt

git fetch --prune --prune-tags
git add --all
git commit -m "Update"
git push
