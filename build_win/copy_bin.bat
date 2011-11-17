echo ----------------------------------------------

cd

echo Build Path: %1
echo Target Name: %2
echo Target Ext: %3

SET Target=release_win
SET TargetRoot=..\..\..\build_out
SET BuildWin=..\..\..\build_win

IF [%1]==[Debug] (
	SET Target=debug_win
)

SET TargetFolder=%TargetRoot%\%Target%

IF [%3]==[.dll] (
	SET TargetFolder=%TargetRoot%\%Target%\bin
)

mkdir %TargetFolder%

echo ----------------------------------------------
echo Copying %2%3
copy .\build\%1\%2%3 %BuildWin%\bin\%2%3

IF [%3]==[.dll] (
	echo Copying %2.lib
	copy .\build\%1\%2.lib %BuildWin%\lib\%2.lib
)

echo Copying %2.pdb
copy .\build\%1\%2.pdb %BuildWin%\lib\%2.pdb
copy .\build\%1\%2.pdb %BuildWin%\bin\%2.pdb

echo copying %2%3 to build folder
copy .\build\%1\%2%3 %TargetFolder%\%2%3


echo ----------------------------------------------
echo %1%