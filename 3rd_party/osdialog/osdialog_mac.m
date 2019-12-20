#include "osdialog.h"
#include <AppKit/AppKit.h>
#include <Availability.h>


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message) {
	@autoreleasepool {

	NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];

	NSAlert *alert = [[NSAlert alloc] init];

	switch (level) {
		default:
#ifdef __MAC_10_12
		case OSDIALOG_INFO: [alert setAlertStyle:NSAlertStyleInformational]; break;
		case OSDIALOG_WARNING: [alert setAlertStyle:NSAlertStyleWarning]; break;
		case OSDIALOG_ERROR: [alert setAlertStyle:NSAlertStyleCritical]; break;
#else
		case OSDIALOG_INFO: [alert setAlertStyle:NSInformationalAlertStyle]; break;
		case OSDIALOG_WARNING: [alert setAlertStyle:NSWarningAlertStyle]; break;
		case OSDIALOG_ERROR: [alert setAlertStyle:NSCriticalAlertStyle]; break;
#endif
	}

	switch (buttons) {
		default:
		case OSDIALOG_OK:
			[alert addButtonWithTitle:@"OK"];
			break;
		case OSDIALOG_OK_CANCEL:
			[alert addButtonWithTitle:@"OK"];
			[alert addButtonWithTitle:@"Cancel"];
			break;
		case OSDIALOG_YES_NO:
			[alert addButtonWithTitle:@"Yes"];
			[alert addButtonWithTitle:@"No"];
			break;
	}

	NSString *messageString = [NSString stringWithUTF8String:message];
	[alert setMessageText:messageString];
	// Non-bold text
	// [alert setInformativeText:messageString];

	NSInteger button = [alert runModal];

	[keyWindow makeKeyAndOrderFront:nil];

	return (button == NSAlertFirstButtonReturn);
	} // @autoreleasepool
}


char *osdialog_prompt(osdialog_message_level level, const char *message, const char *text) {
	@autoreleasepool {

	NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];

	NSAlert *alert = [[NSAlert alloc] init];

	switch (level) {
		default:
#ifdef __MAC_10_12
		case OSDIALOG_INFO: [alert setAlertStyle:NSAlertStyleInformational]; break;
		case OSDIALOG_WARNING: [alert setAlertStyle:NSAlertStyleWarning]; break;
		case OSDIALOG_ERROR: [alert setAlertStyle:NSAlertStyleCritical]; break;
#else
		case OSDIALOG_INFO: [alert setAlertStyle:NSInformationalAlertStyle]; break;
		case OSDIALOG_WARNING: [alert setAlertStyle:NSWarningAlertStyle]; break;
		case OSDIALOG_ERROR: [alert setAlertStyle:NSCriticalAlertStyle]; break;
#endif
	}

	[alert addButtonWithTitle:@"OK"];
	[alert addButtonWithTitle:@"Cancel"];

	NSString *messageString = [NSString stringWithUTF8String:message];
	[alert setMessageText:messageString];

	NSTextField *input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
	[alert setAccessoryView:input];

	if (text) {
		NSString *path_str = [NSString stringWithUTF8String:text];
		[input setStringValue:path_str];
	}

	NSInteger button = [alert runModal];

	char *result = NULL;
	if (button == NSAlertFirstButtonReturn) {
		[input validateEditing];
		NSString *result_str = [input stringValue];
		result = osdialog_strndup([result_str UTF8String], [result_str length]);
	}

	[keyWindow makeKeyAndOrderFront:nil];

	return result;
	} // @autoreleasepool
}


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, osdialog_filters *filters) {
	@autoreleasepool {

	NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];

	NSSavePanel *panel;
	// NSOpenPanel is a subclass of NSSavePanel. Not defined for OSDIALOG_SAVE.
	NSOpenPanel *open_panel;

	if (action == OSDIALOG_OPEN || action == OSDIALOG_OPEN_DIR) {
		panel = open_panel = [NSOpenPanel openPanel];
	}
	else {
		panel = [NSSavePanel savePanel];
	}

	// Bring dialog to front
	// https://stackoverflow.com/a/2402069
	// Thanks Dave!
	[panel setLevel:CGShieldingWindowLevel()];

	if (filters) {
		NSMutableArray *fileTypes = [[NSMutableArray alloc] init];

		for (; filters; filters = filters->next) {
			for (osdialog_filter_patterns *patterns = filters->patterns; patterns; patterns = patterns->next) {
				NSString *fileType = [NSString stringWithUTF8String:patterns->pattern];
				[fileTypes addObject:fileType];
			}
		}

		[panel setAllowedFileTypes:fileTypes];
	}

	if (action == OSDIALOG_OPEN || action == OSDIALOG_OPEN_DIR) {
		[open_panel setAllowsMultipleSelection:NO];
	}
	if (action == OSDIALOG_OPEN) {
		[open_panel setCanChooseDirectories:NO];
		[open_panel setCanChooseFiles:YES];
	}
	if (action == OSDIALOG_OPEN_DIR) {
		[open_panel setCanCreateDirectories:YES];
		[open_panel setCanChooseDirectories:YES];
		[open_panel setCanChooseFiles:NO];
	}

	if (path) {
		NSString *path_str = [NSString stringWithUTF8String:path];
		NSURL *path_url = [NSURL fileURLWithPath:path_str];
		[panel setDirectoryURL:path_url];
	}

	if (filename) {
		NSString *filenameString = [NSString stringWithUTF8String:filename];
		[panel setNameFieldStringValue:filenameString];
	}

	char *result = NULL;

	NSModalResponse response = [panel runModal];
#ifdef __MAC_10_9
	#define OK NSModalResponseOK
#else
	#define OK NSOKButton
#endif
	if (response == OK) {
		NSURL *result_url = [panel URL];
		NSString *result_str = [result_url path];
		result = osdialog_strndup([result_str UTF8String], [result_str length]);
	}

	[keyWindow makeKeyAndOrderFront:nil];

	return result;
	} // @autoreleasepool
}


int osdialog_color_picker(osdialog_color *color, int opacity) {
	assert(0);

	@autoreleasepool {

	// TODO I have no idea what I'm doing here
	NSColorPanel *panel = [NSColorPanel sharedColorPanel];
	// [panel setDelegate:self];
	[panel isVisible];

	// if (opacity)
	// 	[panel setShowAlpha:YES];
	// else
	// 	[panel setShowAlpha:NO];

	// [panel makeKeyAndOrderFront:self];

	return 0;
	} // @autoreleasepool
}
