#!/bin/bash

# Variables
sourceDir=$PWD								# Starting directory
gypApp=../src/third_party/gyp/gyp 			# gyp executable
logDir=$sourceDir/logs						# Log dir
logFile=$logDir/linux_compile.log			# Log file
didFail=0									# Fail flag
missingCount=0								# missing file count
undefCount=0								# Undefined count
errorCount=0								# Error count
warningCount=0								# Warning count
lineCount=0									# Number of lines
fileCount=0									# Number of files
makeConcurrency="-j $MAKE_CONCURRENCY"		# Command to pass to make for concurrency
noPause="false"								# Whether or not to pause
makeReturn=0								# Return value of make
projectCount=0								# Project number reference
projectNum=-1								# Project number to run
globalFail=0								# one project failed
useClang="false"							# Use clang
release="false"								# Release build
color="true"								# Enable color
cleanTarget="false"							# Clean build folder


ce='\e[00m'		#color end
cgr='\e[01;30m'	#color bold grey
cr='\e[01;31m'  #color bold red
cg='\e[01;32m'	#color bold green
cy='\e[01;33m'	#color bold yellow
cb='\e[01;34m'	#color bold blue
cv='\e[01;35m'	#color bold violet
ca='\e[01;36m'	#color bold aqua
cw='\e[01;37m'	#color bold white

# Append new make targets here:
buildDirs=(
			"managers"
			"mdump"
			"threads"
			"umcf"
			"util_fs"
			"util"
			"wx_controls"
			"ipc_pipe"
			"scriptengine"
			"mcfcore"
			"webcore"
			"usercore"
			"uicore"
			"servicecore"
			"crashuploader"
			"scriptcore"
			"bootloader_lin"
			"bittest_lin"
			"crashdlg_lin"
			"process_data_folder"
			)
			
while [ $# -gt 0 ]; do
	# Clean if we need to
	if [ "$1" == "--clean" -o "$1" == "-X" ]; then
		echo -n "Cleaning out build files... "
		mv ../build_out/lin_release/lib/libflashplayer_64.so ../build_out/lin_release/
		mv ../build_out/lin_release/lib/libflashplayer_32_chrome.so ../build_out/lin_release/
		mv ../build_out/lin_debug/lib/libflashplayer_64.so ../build_out/lin_debug/
		mv ../build_out/lin_debug/lib/libflashplayer_32_chrome.so ../build_out/lin_debug/
		rm -rf ../out/ *.mk ../Makefile ../build_out/lin_release/lib/* ../build_out/lin_release/bin/* ../build_out/lin_release/data/ ../build_out/lin_debug/data/ ../build_out/lin_debug/lib/* ../build_out/lin_debug/bin/* ../build_out/lin_release/cache ../build_out/lin_release/*.desktop ../build_out/lin_release/version ../build_out/lin_release/settings ../build_out/lin_debug/cache ../build_out/lin_debug/*.desktop ../build_out/lin_debug/version ../build_out/lin_debug/settings ../build_out/lin_debug/desura_update.mcf ../build_out/lin_release/desura_update.mcf ../build_out/lin_debug/desura_appfiles.xml ../build_out/lin_release/desura_appfiles.xml desura_upload.mcf desura*.deb desura*.tar.gz
		mv ../build_out/lin_release/libflashplayer_64.so ../build_out/lin_release/lib/
		mv ../build_out/lin_release/libflashplayer_32_chrome.so ../build_out/lin_release/lib/
		mv ../build_out/lin_debug/libflashplayer_64.so ../build_out/lin_debug/lib/
		mv ../build_out/lin_debug/libflashplayer_32_chrome.so ../build_out/lin_debug/lib/
		echo -en "Complete.\nRestoring release files... "
		svn up ../build_out/lin_release ../build_out/lin_debug &>/dev/null
		echo -e "Complete."
		exit
	elif [ "$1" == "--linecount" -o "$1" == "-lc" ]; then
		echo -e "Counting lines of source code ...\n"
	
		for currentTarget in ${buildDirs[@]}; do
			echo -ne "\t$currentTarget... "
			lineCount=$((lineCount + `find $currentTarget -iname *.cpp | grep -v "\.svn" | while read LINE; do cat $LINE; done | wc -l`))
			lineCount=$((lineCount + `find $currentTarget -iname *.h | grep -v "\.svn" | while read LINE; do cat $LINE; done | wc -l`))
			fileCount=$((fileCount + `find $currentTarget -iname *.cpp | grep -v "\.svn" | wc -l`))
			fileCount=$((fileCount + `find $currentTarget -iname *.h | grep -v "\.svn" | wc -l`))
			echo "done!"
		done
	
		echo -e "\nComplete. $lineCount lines in $fileCount files, ${#buildDirs[@]} projects"
		exit
	elif [ "$1" == "--list" -o "$1" == "-l" ]; then
		echo -e "Listing projects configured to build:"
	
		for currentTarget in ${buildDirs[@]}; do
			echo -e "\t$projectCount:\t$currentTarget"
			projectCount=$((projectCount + 1));
		done

		exit
	elif [ "$1" == "--smp" -o "$1" == "-s" ]; then
		shift
		if [ -z $1 ]; then
			echo "Error: no concurrency level passed!" 1>&2
			exit 1
		fi
	
		makeConcurrency="-j $1"
		shift
	elif [ "$1" == "--help" -o "$1" == "-h" ]; then
		echo "Usage: $0 <options>"
		echo -e "Options:"
		echo -e "\t-c  or --clang\t\t-\tCompile using clang (instead of GCC -- implies --dogyp)"
		echo -e "\t-l  or --list\t\t-\tLists all projects that will be built"
		echo -e "\t-X  or --clean\t\t-\tClean out compiled files then exit"
		echo -e "\t-lc or --linecount\t-\tCount lines, files and projects then exit"
		echo -e "\t-s  or --smp X\t\t-\tRun make with '-j X' where X is concurrent processes"
		echo -e "\t\t\t\t\t(Defaults to the value of MAKE_CONCURRENCY if not specified)"
		echo -e "\t-h  or --help\t\t-\tShows this help menu"
		echo -e "\t-np or --nopause\t-\tDoesn't pause to show error log"
		echo -e "\t-o  or --only x\t\t-\tOnly build project number 'x'"
		echo -e "\t-r  or --release\t-\tDo a release build"
		echo -e "\t-nc or --nocolor\t-\tDont output color"
		echo -e "\t-nc or --nocolour\t-\tLike above but British"
		echo -e "\t-co or --cleanout\t-\tClean the desura files from the out dir"
		exit 0
	elif [ "$1" == "--nopause" -o "$1" == "-np" ]; then
		noPause="true"
		shift
	elif [ "$1" == "--only" -o "$1" == "-o" ]; then
		if [ ${#2} -eq 0 ]; then
			echo "A project number must be specified if you use --only"
			exit 1
		else
			projectNum=$2
			shift
		fi
		
		shift
	elif [ "$1" == "--clang" -o "$1" == "-c" ]; then
		useClang="true"
		shift
	elif [ "$1" == "--release" -o "$1" == "-r" ]; then
		release="true"
		shift
	elif [ "$1" == "--nocolor" -o "$1" == "--nocolour" -o "$1" == "-nc" ]; then
		color="false"
		shift
	elif [ "$1" == "--cleanout" -o "$1" == "-co"  ]; then
		cleanTarget="true"
		shift
	else
		echo "Unknown argument \"$1\". Please run \"$0 --help\" for more information"
		exit 2
	fi
done

if [ "$color" == "false" ]; then
	ce=""	#color end
	cg=""	#color green
	cr=""	#color red
	cp=""	#color pink
	cl=""	#color lime
fi

mkdir -p $logDir
echo > $logFile

# Check for common include file
if [ ! -e ../src/common.gypi ]; then
	echo "Missing common.gypi. Can't continue"
	exit 1
fi

# Check for gyp
if [ ! -e $gypApp ]; then
	echo "Missing third_party/gyp/gyp. Can't continue"
	exit 2
fi

# Output starting info
echo ""
echo ""
echo -en "Building Desura: Target "

if [ "$release" == "true" ]; then
	echo -e "${cr}Release${ce}"
else
	echo -e "${cg}Debug${ce}"
fi

echo ""
echo "Settings:"

if [ "$makeConcurrency" == "" ]; then
	echo -e "\tConcurrency mode is\t${cg}OFF${ce}"
else
	echo -e "\tConcurrency mode is\t${cr}'$makeConcurrency'${ce}"
fi

if [ "$noPause" == "true" ]; then
	echo -e "\tPause for log is\t${cr}OFF${ce}"
else
	echo -e "\tPause for log is\t${cg}ON${ce}"
fi

if [ $projectNum -ne -1 ]; then
	echo -e "\tCompiling project(s)\t${cr}$projectNum${ce}"
else
	echo -e "\tCompiling project(s)\t${cg}ALL${ce}"
fi

if [ "$cleanTarget" == "true" ]; then
	echo -e "\tClean Desura Files is\t${cr}ON${ce}"
else
	echo -e "\tClean Desura Files is\t${cg}OFF${ce}"
fi

if [ "$useClang" == "true" ]; then
	echo -e "\tClang compile is\t${cr}ON${ce}"
	echo -e "\t\tPlease note that the Makefile will now be in"
	echo -e "\t\tClang mode. To return to GCC mode, you will"
	echo -e "\t\tneed to run $0 --dogyp again (without --clang)"
else
	echo -e "\tClang compile is\t${cg}OFF${ce}"
fi

echo -e "Starting Linux compile!\n"

startTime=`date`

touch $logFile

function copyLib()
{
	if [ -e "$1" ]; then
		cp $1 build_out/$outDest/lib_extra/.
	else
		echo -e "\t\tWarning not copying $1. This should not be a public build."
	fi	
}

function copyFiles()
{
	outDest=debug_lin
	outTarget=Debug

	if [ "$1" == "true" ]; then
		outDest=release_lin
		outTarget=Release
	fi

	echo
	echo "Moving files ($outTarget):"

	echo "[$PWD]"

	mkdir -p build_out/$outDest/bin
	mkdir -p build_out/$outDest/lib
	mkdir -p build_out/$outDest/lib_extra

	if [ -e out/$outTarget/process_data_folder ]; then
		echo -ne "\process_data_folder\t-\t"
		cp out/$outTarget/process_data_folder tools/.
		echo "Done"
	fi

	if [ -e out/$outTarget/desura ]; then	
		echo -ne "\tdesura\t\t-\t"
		cp out/$outTarget/desura build_out/$outDest/bin/
		echo "Done"
	fi

	if [ x"$(uname -m)" != x"x86_64" -a -e out/$outTarget/bittest ]; then # 64 bit
		echo -ne "\tbittest\t\t-\t"
		cp out/$outTarget/bittest build_out/$outDest/bin/
		echo "Done"
	fi

	if [ -e out/$outTarget/crashdlg ]; then	
		echo -ne "\tcrashdlg\t-\t"
		cp out/$outTarget/crashdlg build_out/$outDest/bin/
		echo "Done"
	fi

	if [ -d out/$outTarget/lib.target ]; then
		echo -ne "\tshared libs\t-\t"
		cd out/$outTarget/lib.target
		
		for LIB in ./*; do

			if [ -e "../../../build_out/$outDest/lib/$LIB" ]; then
				rm ../../../build_out/$outDest/lib/$LIB
			fi

			ln -s $PWD/$LIB ../../../build_out/$outDest/lib/$LIB
		done
	
		cd ../../..
		echo "Done"
	fi
	
	echo -en "\tmisc libs\t-\t"

	if [ "$(uname -m)" == "x86_64" ]; then
		cp /usr/local/lib64/libgcc_s.so.1 build_out/$outDest/lib/
		cp /usr/local/lib64/libstdc++.so.6.0.14 build_out/$outDest/lib/libstdc++.so.6
	else
		cp /usr/local/lib/libgcc_s.so.1 build_out/$outDest/lib/
		cp /usr/local/lib/libstdc++.so.6.0.14 build_out/$outDest/lib/libstdc++.so.6
	fi	

	echo -e "Done"

	if [ -e build_out/$outDest/lib/libcurl_desura.so.4 ]; then 
		echo -ne "\tlibssl\t\t-\t"
		cp `ldd build_out/$outDest/lib/libcurl_desura.so.4 | grep ssl | cut -d " " -f 3` build_out/$outDest/lib_extra/.
		echo "Done" 
	fi

	if [ -e build_out/$outDest/lib_extra/libssl* ]; then
		echo -ne "\tlibcrypto\t-\t"
		cp `ldd build_out/$outDest/lib_extra/libssl* | grep crypto | cut -d " " -f 3` build_out/$outDest/lib_extra/.
		echo "Done"
	fi
		

	echo -e "\tExtra libs"
	
	copyLib "/usr/lib/libjpeg.so.62"
	copyLib "/usr/lib/libpng12.so.0"
	copyLib "/usr/lib/libz.so.1"
	
	copyLib "/usr/lib/libnspr4.so.0d"
	copyLib "/usr/lib/libnss3.so.1d"
	copyLib "/usr/lib/libnssutil3.so.1d"
	copyLib "/usr/lib/libplc4.so.0d"
	copyLib "/usr/lib/libplds4.so.0d"
	copyLib "/usr/lib/libsmime3.so.1d"
	copyLib "/usr/lib/libssl3.so.1d"

	copyLib "/usr/lib/libgconf-2.so.4"

	echo -e "\tDone"

	echo -ne "\tCreating data dir:\t-\t" 
	cd build_out
	./copy_data.sh $outDest
	cd ..
	echo "Done"


	if [ "$outDest" == "debug_lin" ]; then

		echo -ne "\tCreating sym links to release\t-\t"

		OLDDIR=$PWD
		cd build_out/release_lin/bin
		
		if [ -L ../../debug_lin/desura ]; then
			rm ../../debug_lin/desura
		fi

		ln -s $PWD/../desura ../../debug_lin/desura

		for FILE in *; do
			if [ -L ../../debug_lin/bin/$FILE ]; then
				rm ../../debug_lin/bin/$FILE
			fi

			if [ -e ../../debug_lin/bin/$FILE ]; then
				continue
			fi

			ln -s $PWD/$FILE ../../debug_lin/bin/$FILE
		done

		cd $OLDDIR

		echo "Done"
	fi
}

function runMakeProcess() 
{
	didFail=0
	
	missingCount=`cat $logFile | grep -i "No such file or directory" | wc -l` 
	undefCount=`cat $logFile | grep -i "undefined" | wc -l` 
	errorCount=`cat $logFile | grep -i "error:" | wc -l` 
	warningCount=`cat $logFile | grep "warning:" | wc -l`

	echo -ne "\tMake...\t\t"
	make $makeConcurrency $1 $2 2>>$logFile 1>>$logFile
	# Save the return value as the if statement can lose it
	local makeReturn=$?

	missingCount=$((`cat $logFile | grep -i "No such file or directory" | wc -l`-$missingCount))
	undefCount=$((`cat $logFile | grep -i "undefined" | wc -l`-$undefCount))
	errorCount=$((`cat $logFile | grep -i "error:" | wc -l`-$errorCount))
	warningCount=$((`cat $logFile | grep -i "warning:" | wc -l`-$warningCount))	
	
	if [ $makeReturn -gt 0 ]; then

		#return 2 norm means undefined or did not find target
		if [ $undefCount -eq 0 -a $errorCount -eq 0 ]; then
			echo -e "${cr}WARNING!${ce}\n\t>> Make returned $makeReturn for $currentTarget <<"
			didFail=2
		else
			echo -e "${cr}Failed!${ce}"
			didFail=1
		fi
	else
		# Everything went well!
		echo -e "${cg}Done!${ce}"
	fi	
}

function dispMakeResults()
{
	if [ $warningCount -ne 0 ]; then
		echo -e "\t${cp}Warnings:${ce}\t${cp}$warningCount${ce}"
	fi

	if [ $missingCount -ne 0 ]; then
		echo -e "\t${cr}Missing:${ce}\t${cr}$missingCount${ce}"
	fi

	if [ $undefCount -ne 0 ]; then
		echo -e "\t${cr}Undefined:${ce}\t${cr}$undefCount${ce}"
	fi

	if [ $errorCount -ne 0 ]; then
		echo -e "\t${cr}Errors:${ce}\t\t${cr}$errorCount${ce}"
	fi	
}

function runGypProcess()
{
	echo 'Checking if gyp gen is needed...'
	
	#$1 is force run
	
	local runGyp=0
	didFail=0
	
	# Pass build.gyp to gyp and log its output
	if [ $1 -eq 1 -o ! -f "build.Makefile" ]; then
		runGyp=1
		echo -ne "\tgyp...\t\t"
		$gypApp build.gyp --toplevel-dir=.. --depth=. 2>>$logFile 1>>$logFile
	fi

	# If gyp returned > 0, there was a problem
	if [ $runGyp -eq 1 -a $? -gt 0 ]; then
		echo -e "${cr}Failed!${ce}\n"
		echo -e "\t>> Errors seemed to occur during the gyp process for $currentTarget. <<"
		didFail=1
	elif [ $runGyp -eq 1 ]; then
		# It succeeded, so call make
		echo -e "${cg}Done!${ce}\n"
	fi
}

if [ "$useClang" == "true" ]; then
	echo " ---------------- Using clang -----------------" >> $logFile
	export CC=clang			# compiler
	export CXX=clang++		# c++ compiler
	export builddir_name=llvm	# output dir
	export GYP_DEFINES='clang=1'	# gyp define
	
	runGypProcess 1
else
	runGypProcess 1
fi

if [ $didFail -ne 0 ]; then
	#need to do something if gyp failed
	globalFail=1
else
	cd ..

	# Iterate through build targets
	for currentTarget in ${buildDirs[@]}; do
		if [ x"$currentTarget" == x"bittest_lin" -a x"$(uname -m)" == x"x86_64" ]; then continue; fi

		if [ $projectNum -ne -1 ]; then
			if [ $projectCount -ne $projectNum ]; then
				projectCount=$((projectCount + 1))
				continue
			fi
		fi

		echo -e "${cl}Building $currentTarget ($projectCount):${ce}"
		echo "-----------------------------------------------------------" >> $logFile
		echo -e " Building project $currentTarget" >> $logFile
		echo "-----------------------------------------------------------" >> $logFile

		makeArgs=$currentTarget

		if [ "$release" == "true" ]; then
			makeArgs=" BUILDTYPE=Release $currentTarget"
		fi

		#run make the first time
		runMakeProcess $makeArgs $1

		# Blank line
		echo

		echo "" >> $logFile
		echo "" >> $logFile

		if [ $didFail -gt 0 ]; then
			globalFail=1
		fi

		missingCount=0
		undefCount=0
		errorCount=0
		warningCount=0
		projectCount=$((projectCount + 1))
	done

	# Finishing output
	echo "Linux compile finished!"
	echo -e "Start time:\t$startTime"
	echo -ne "Finish time:\t"
	date
fi

if [ $globalFail -gt 0 ]; then
	
	echo 
	echo "There were compile errors. Press [enter] to review the log"
	if [ "$noPause" == "false" ]; then	
		read
		less $logFile
	fi
else
	# Call function to copy all files
	copyFiles $release
	
	if [ "$cleanTarget" == "true" ]; then
		rm -fr build_out/$outDest/common
		rm -fr build_out/$outDest/.settings
	fi

	if [ ! -e build_out/$outDest/version -a "$outDest" == "debug_lin" ]; then
		echo -ne "Generating version file\t-\t"
		if [ "$(uname -m)" == "x86_64" ]; then
			echo -ne "BRANCH=520\nBUILD=0" > build_out/$outDest/version
		else
			echo -ne "BRANCH=510\nBUILD=0" > build_out/$outDest/version
		fi
		echo "Done"
	fi
fi

# Change back to the source dir and reset counts
cd $sourceDir

exit $didFail
