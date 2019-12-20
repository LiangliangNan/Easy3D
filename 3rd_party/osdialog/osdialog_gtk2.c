#include "osdialog.h"
#include <assert.h>
#include <string.h>
#include <gtk/gtk.h>



int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message) {
	assert(gtk_init_check(NULL, NULL));

	GtkMessageType messageType;
	switch (level) {
		default:
		case OSDIALOG_INFO: messageType = GTK_MESSAGE_INFO; break;
		case OSDIALOG_WARNING: messageType = GTK_MESSAGE_WARNING; break;
		case OSDIALOG_ERROR: messageType = GTK_MESSAGE_ERROR; break;
	}

	GtkButtonsType buttonsType;
	switch (buttons) {
		default:
		case OSDIALOG_OK: buttonsType = GTK_BUTTONS_OK; break;
		case OSDIALOG_OK_CANCEL: buttonsType = GTK_BUTTONS_OK_CANCEL; break;
		case OSDIALOG_YES_NO: buttonsType = GTK_BUTTONS_YES_NO; break;
	}

	GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, messageType, buttonsType, "%s", message);

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();

	return (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_YES);
}


char *osdialog_prompt(osdialog_message_level level, const char *message, const char *text) {
	assert(gtk_init_check(NULL, NULL));

	GtkMessageType messageType;
	switch (level) {
		default:
		case OSDIALOG_INFO: messageType = GTK_MESSAGE_INFO; break;
		case OSDIALOG_WARNING: messageType = GTK_MESSAGE_WARNING; break;
		case OSDIALOG_ERROR: messageType = GTK_MESSAGE_ERROR; break;
	}

	GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, messageType, GTK_BUTTONS_OK_CANCEL, "%s", message);

	GtkWidget *entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), text);

	GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_add(GTK_CONTAINER(content_area), entry);
	gtk_widget_show_all(dialog);

	gint button = gtk_dialog_run(GTK_DIALOG(dialog));
	const char *result_str = gtk_entry_get_text(GTK_ENTRY(entry));

	char *result = NULL;
	if (button == GTK_RESPONSE_OK) {
		result = osdialog_strndup(result_str, strlen(result_str));
	}
	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();

	return result;
}


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, osdialog_filters *filters) {
	assert(gtk_init_check(NULL, NULL));

	GtkFileChooserAction gtkAction;
	const char *title;
	const char *acceptText;
	if (action == OSDIALOG_OPEN) {
		title = "Open File";
		acceptText = "Open";
		gtkAction = GTK_FILE_CHOOSER_ACTION_OPEN;
	}
	else if (action == OSDIALOG_OPEN_DIR) {
		title = "Open Folder";
		acceptText = "Open Folder";
		gtkAction = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	}
	else {
		title = "Save File";
		acceptText = "Save";
		gtkAction = GTK_FILE_CHOOSER_ACTION_SAVE;
	}

	GtkWidget *dialog = gtk_file_chooser_dialog_new(
		title,
		NULL,
		gtkAction,
		"_Cancel", GTK_RESPONSE_CANCEL,
		acceptText, GTK_RESPONSE_ACCEPT,
		NULL);

	for (; filters; filters = filters->next) {
		GtkFileFilter *fileFilter = gtk_file_filter_new();
		gtk_file_filter_set_name(fileFilter, filters->name);
		for (osdialog_filter_patterns *patterns = filters->patterns; patterns; patterns = patterns->next) {
			char patternBuf[1024];
			snprintf(patternBuf, sizeof(patternBuf), "*.%s", patterns->pattern);
			gtk_file_filter_add_pattern(fileFilter, patternBuf);
		}
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), fileFilter);
	}

	if (action == OSDIALOG_SAVE)
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (path)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);

	if (action == OSDIALOG_SAVE && filename)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);

	char *chosen_filename = NULL;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		chosen_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}
	gtk_widget_destroy(dialog);

	char *result = NULL;
	if (chosen_filename) {
		result = osdialog_strndup(chosen_filename, strlen(chosen_filename));
		g_free(chosen_filename);
	}

	while (gtk_events_pending())
		gtk_main_iteration();
	return result;
}


int osdialog_color_picker(osdialog_color *color, int opacity) {
	if (!color)
		return 0;
	assert(gtk_init_check(NULL, NULL));

#ifdef OSDIALOG_GTK3
	GtkWidget *dialog = gtk_color_chooser_dialog_new ("Color", NULL);
	GtkColorChooser *colorsel = GTK_COLOR_CHOOSER(dialog);
	gtk_color_chooser_set_use_alpha(colorsel, opacity);
#else
	GtkWidget *dialog = gtk_color_selection_dialog_new("Color");
	GtkColorSelection *colorsel = GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(dialog)));
	gtk_color_selection_set_has_opacity_control(colorsel, opacity);
#endif

	int result = 0;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
#ifdef OSDIALOG_GTK3
		GdkRGBA c;
		gtk_color_chooser_get_rgba(colorsel, &c);
		color->r = c.red * 65535 + 0.5;
		color->g = c.green * 65535 + 0.5;
		color->b = c.blue * 65535 + 0.5;
		color->a = c.alpha * 65535 + 0.5;
#else
		GdkColor c;
		gtk_color_selection_get_current_color(colorsel, &c);
		color->r = c.red >> 8;
		color->g = c.green >> 8;
		color->b = c.blue >> 8;
		color->a = gtk_color_selection_get_current_alpha(colorsel) >> 8;
#endif

		result = 1;
	}

	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();
	return result;
}
