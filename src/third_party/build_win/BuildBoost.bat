cd ..\boost

IF NOT EXIST bjam.exe (
	cmd /C bootstrap.bat
)

bjam.exe stage --with-date_time --with-filesystem --with-thread --with-system variant=debug link=static threading=multi runtime-link=shared
bjam.exe stage --with-date_time --with-filesystem --with-thread --with-system variant=release link=static threading=multi runtime-link=shared
bjam.exe stage --with-date_time --with-filesystem --with-thread --with-system variant=release link=static threading=multi runtime-link=static


cd ..\build_win