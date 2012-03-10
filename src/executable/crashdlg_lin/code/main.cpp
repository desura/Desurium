/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/


#include "Common.h"
#include <gtk/gtk.h>

#include "util/UtilLinux.h"
#include <branding/branding.h>

std::string GetAppPath()
{
	ERROR_OUTPUT(__func__);
	char result[PATH_MAX] = {0};
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

	if (count <= 0)
	{
		ERROR_OUTPUT("Could not get program directory!");
		return "";
	}

	for(size_t i = count - 1; i > 0; i--)
	{
		if(result[i] == '/')
		{
			result[i] = '\0';
			break;
		}
	}

	std::string sResult(result);
	size_t pos = sResult.find_last_of('/');

	// we actually want one folder above where we are due to executable being in desura/bin/
	sResult = sResult.substr(0, pos);

	ERROR_OUTPUT(sResult.c_str());
	return sResult;
}

bool ChangeToAppDir()
{
	ERROR_OUTPUT(__func__);
	std::string appPath = GetAppPath();
	ERROR_OUTPUT(appPath.c_str());
	if(chdir(appPath.c_str()) != 0)
	{
		ERROR_OUTPUT("Failed to change working directory!");
		return false;
	}

	return true;
}


void on_response(GtkDialog *dialog, gint response, gpointer data)
{
	if (response == 1)
	{
		UTIL::LIN::launchProcess("desura");
	}
	else if (response == 2)
	{
		std::map<std::string, std::string> info;
		info["cla"] = "--forceupdate";
		
		UTIL::LIN::launchProcess("desura", info);		
	}
	
	gtk_main_quit();
}

int main(int argc, char** argv)
{
	gtk_init(&argc, &argv);
	
	const char* text =  "An error has occured that is preventing " PRODUCT_NAME " from running correctly. "
						"Restarting " PRODUCT_NAME " should allow you to continue, otherwise you can clean "
						"your " PRODUCT_NAME " install by forcing an update.";
	
	if (!ChangeToAppDir())
		return 1;
	
	GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, "%s", text);
	
	gtk_window_set_title(GTK_WINDOW(dialog), PRODUCT_NAME " Error");

	gtk_dialog_add_button(GTK_DIALOG(dialog), "Restart " PRODUCT_NAME, 1);
	gtk_dialog_add_button(GTK_DIALOG(dialog), "Force Update", 2);
	gtk_dialog_add_button(GTK_DIALOG(dialog), "Exit", 3);
	
	g_signal_connect(G_OBJECT(dialog), "delete-event", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(on_response), NULL);
	
	gtk_window_set_keep_above(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	gtk_widget_show_all(dialog);
	
	gtk_main();
	
	return 0;
}


