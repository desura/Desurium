#!/bin/bash
#
# Warning: This file is auto generated. Do not edit
# (Template: 0 is path to open)
#

function error()
{
	# Trim newlines etc that may cause dialog issues
	MSG="`echo "$1" | sed '/./,$!d'`"

	# Print to stderr regardless
	echo "$MSG" >&2
	
	# Try using zenity (nearly all GTK systems)
	zenity --error --title="Desura" --text="$MSG"
	if [ "$?" -ne 127 ]; then
		return
	fi

	# Try using kdialog (nearly all KDE systems)
	kdialog --title "Desura" --error "$MSG"
	if [ "$?" -ne 127 ]; then
		return
	fi

	# Try using gxmessage (some lightweight GTK systems)
	gxmessage -wrap -title "Desura" -center -buttons GTK_STOCK_OK -default OK "$MSG"
	if [ "$?" -ne 127 ]; then
		return
	fi

	# Try using gxmessage (some lightweight GTK systems)
	gmessage -wrap -title "Desura" -center -buttons GTK_STOCK_OK -default OK "$MSG"
	if [ "$?" -ne 127 ]; then
		return
	fi
	
	# Try using xmessage (nearly all X systems)
	xmessage -title "Desura" -center -buttons OK -default OK -xrm '*message.scrollVertical: Never' "$MSG"
	if [ "$?" -ne 127 ]; then
		return
	fi
}

xdg-open {0}

if [ $? == 127 ]; then
	gnome-open {0}
fi

if [ $? == 127 ]; then
	error "Failed to find xdg-open or gnome-open. Cant launch {0}"
fi
