#dest dir needs to be in %1

RMDIR /S /Q %1\data

..\tools\process_data_folder.exe data %1\data _win _lin
..\tools\process_data_folder.exe ..\src\branding\data %1\data _win _lin