#!/bin/sh

# check if we have given a patch file
if [ ! -f $1 ]; then
	echo "=== no patch file specified ==="
	exit 1
fi

# define a location for the patch file backup
PATCH_BAK=$(pwd)/bak_$(basename $1)

# creating a backup if not exist
if [ ! -f $PATCH_BAK ]; then
	echo "=== creating patch backup to $PATCH_BAK ==="
	cp $1 $PATCH_BAK
fi

# first we try to merge the given patch
patch -p0 -N --merge --dry-run -i $1
TRY_PATCH_STATUS=$?
echo "=== fake patch step returns with $TRY_PATCH_STATUS ==="

# we revert the patched sources with the backed up file
if [ $TRY_PATCH_STATUS = "1" ]; then
	echo "=== try to revert patch ==="
	patch -p0 -N -R --dry-run -i $PATCH_BAK
	TRY_REVERT_PATCH_STATUS=$?
	echo "=== fake revert patch returns with $TRY_REVERT_PATCH_STATUS ==="
	if [ $TRY_REVERT_PATCH_STATUS = "0" ]; then
		echo "=== revert patch ==="
		patch -p0 -N -R -i $PATCH_BAK
		echo "=== reapllying patch ==="
		patch -p0 -N -i $1

	# if reverting goes wrong we give out a message
	else
		echo "=== reverting patch failed, please remove all files from this subproject ==="
		exit 1
	fi

# everything was okay in the test run, we will apply this patch
elif [ $TRY_PATCH_STATUS = "0" ]; then
	echo "=== test succeeded. Apply patch ==="
	patch -p0 -N --merge -i $1
# if everything goes wrong, we give out a message
else
	echo "=== patch step failed, please remove all files from this subproject ==="
	exit 1
fi

# after everything is okay, we should backup the new patch file
echo "=== backing up new patch file ==="
cp $1 $PATCH_BAK
