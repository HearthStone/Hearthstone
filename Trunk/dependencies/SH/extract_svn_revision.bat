REM sadly CMake cannot directly execute the shell scripts under msyssvn even if they are associated
@echo off
start SH.exe extract_svn_revision.sh
