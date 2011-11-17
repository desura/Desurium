echo ----------------------------------------------

echo Build Path: %1
echo Target Name: %2

mkdir ..\..\..\build_win\lib

echo ----------------------------------------------
echo Copying %2.lib
copy ".\build\%1\%2.lib" "..\..\..\build_win\lib\%2.lib"

echo Copying %2.pdb
copy ".\build\%1\%2.pdb" "..\..\..\build_win\lib\%2.pdb"
echo ----------------------------------------------