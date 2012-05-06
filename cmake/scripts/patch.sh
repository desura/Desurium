#!/bin/sh
patch -p0 -N --merge --dry-run -i $1
TRY_PATCH_STATUS=$?
echo "=== fake patch step returns with $TRY_PATCH_STATUS ==="
if [ $TRY_PATCH_STATUS = "1" ]; then
	echo "=== try to revert patch ==="
	patch -p0 -N -R --dry-run -i $1
	TRY_REVERT_PATCH_STATUS=$?
	echo "=== fake revert patch returns with $TRY_REVERT_PATCH_STATUS ==="
	if [ $TRY_REVERT_PATCH_STATUS = "0" ]; then
		echo "=== revert patch ==="
		patch -p0 -N -R -i $1
		echo "=== reapllying patch ==="
		patch -p0 -N -i $1
	else
		echo "=== reverting patch failed, please remove all files from this subproject"
		exit 1
	fi
elif [ $TRY_PATCH_STATUS = "0" ]; then
	echo "=== test succeeded. Apply patch"
	patch -p0 -N --merge -i $1
else
	echo "=== patch step failed, please remove all files from this subproject"
	exit 1
fi
