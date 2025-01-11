@echo off
echo Running make with arguments: %*
make %*
echo Execution completed.
echo Deleting binaries...
make clean_build %*
echo Binaries deleted :D
