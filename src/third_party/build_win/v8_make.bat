@echo off

IF [%1]==[debug] (

	echo Debug Build

	scons -C ..\v8 env="PATH:%PATH%,INCLUDE:%INCLUDE%,LIB:%LIB%" snapshot=on -j 12 mode=debug msvcrt=shared

) ELSE (

	IF [%1]==[static] (

		echo Static Build

		scons -C ..\v8 env="PATH:%PATH%,INCLUDE:%INCLUDE%,LIB:%LIB%" snapshot=on -j 12 msvcrt=static

	) ELSE (

		echo Release build

		scons -C ..\v8 env="PATH:%PATH%,INCLUDE:%INCLUDE%,LIB:%LIB%" snapshot=on -j 12 msvcrt=shared
	)
)

@echo on

EXIT