#include "osdialog.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>


char *osdialog_strndup(const char *s, size_t n) {
	char *d = OSDIALOG_MALLOC(n + 1);
	memcpy(d, s, n);
	d[n] = '\0';
	return d;
}

osdialog_filters *osdialog_filters_parse(const char *str) {
	osdialog_filters *filters_head = OSDIALOG_MALLOC(sizeof(osdialog_filters));
	filters_head->next = NULL;

	osdialog_filters *filters = filters_head;
	osdialog_filter_patterns *patterns = NULL;

	const char *text = str;
	while (1) {
		switch (*str) {
			case ':': {
				filters->name = osdialog_strndup(text, str - text);
				filters->patterns = OSDIALOG_MALLOC(sizeof(osdialog_filter_patterns));
				patterns = filters->patterns;
				patterns->next = NULL;
				text = str + 1;
			} break;
			case ',': {
				assert(patterns);
				patterns->pattern = osdialog_strndup(text, str - text);
				patterns->next = OSDIALOG_MALLOC(sizeof(osdialog_filter_patterns));
				patterns = patterns->next;
				patterns->next = NULL;
				text = str + 1;
			} break;
			case ';': {
				assert(patterns);
				patterns->pattern = osdialog_strndup(text, str - text);
				filters->next = OSDIALOG_MALLOC(sizeof(osdialog_filters));
				filters = filters->next;
				filters->next = NULL;
				patterns = NULL;
				text = str + 1;
			} break;
			case '\0': {
				assert(patterns);
				patterns->pattern = osdialog_strndup(text, str - text);
			} break;
			default: break;
		}
		if (!*str)
			break;
		str++;
	}

	return filters_head;
}

static void patterns_free(osdialog_filter_patterns *patterns) {
	if (!patterns)
		return;
	OSDIALOG_FREE(patterns->pattern);
	osdialog_filter_patterns *next = patterns->next;
	OSDIALOG_FREE(patterns);
	patterns_free(next);
}

void osdialog_filters_free(osdialog_filters *filters) {
	if (!filters)
		return;
	OSDIALOG_FREE(filters->name);
	patterns_free(filters->patterns);
	osdialog_filters *next = filters->next;
	OSDIALOG_FREE(filters);
	osdialog_filters_free(next);
}
