
RMDIR /S /Q debug_win\data

..\tools\process_data_folder.exe data debug_win\data _win _lin
..\tools\process_data_folder.exe ..\src\branding\data debug_win\data _win _lin


RMDIR /S /Q release_win\data

..\tools\process_data_folder.exe data release_win\data _win _lin
..\tools\process_data_folder.exe ..\src\branding\data release_win\data _win _lin

pause