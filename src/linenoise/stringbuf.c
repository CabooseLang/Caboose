#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef STRINGBUF_H
#include "stringbuf.h"
#endif
#ifdef USE_UTF8
#include "utf8.h"
#endif

#define SB_INCREMENT 200

stringbuf *sb_alloc(void) {
	stringbuf *sb = (stringbuf *)malloc(sizeof(*sb));
	sb->remaining = 0;
	sb->last = 0;
#ifdef USE_UTF8
	sb->chars = 0;
#endif
	sb->data = NULL;
	return (sb);
}

void sb_free(stringbuf *sb) {
	if (sb)
		free(sb->data);
	free(sb);
}

void sb_realloc(stringbuf *sb, int newlen) {
	sb->data = (char *)realloc(sb->data, newlen);
	sb->remaining = newlen - sb->last;
}

void sb_append(stringbuf *sb, const char *str) {
	sb_append_len(sb, str, strlen(str));
}

void sb_append_len(stringbuf *sb, const char *str, int len) {
	if (sb->remaining < len + 1)
		sb_realloc(sb, sb->last + len + 1 + SB_INCREMENT);
	memcpy(sb->data + sb->last, str, len);
	sb->data[sb->last + len] = 0;
	sb->last += len;
	sb->remaining -= len;
#ifdef USE_UTF8
	sb->chars += utf8_strlen(str, len);
#endif
}

char *sb_to_string(stringbuf *sb) {
	if (sb->data == NULL)
		return strdup("");
	else {
		char *pt = sb->data;
		free(sb);
		return pt;
	}
}

static void sb_insert_space(stringbuf *sb, int pos, int len) {
	assert(pos <= sb->last);
	if (sb->remaining < len)
		sb_realloc(sb, sb->last + len + SB_INCREMENT);
	memmove(sb->data + pos + len, sb->data + pos, sb->last - pos);
	sb->last += len;
	sb->remaining -= len;
	sb->data[sb->last] = 0;
}

static void sb_delete_space(stringbuf *sb, int pos, int len) {
	assert(pos < sb->last);
	assert(pos + len <= sb->last);
#ifdef USE_UTF8
	sb->chars -= utf8_strlen(sb->data + pos, len);
#endif
	memmove(sb->data + pos, sb->data + pos + len, sb->last - pos - len);
	sb->last -= len;
	sb->remaining += len;
	sb->data[sb->last] = 0;
}

void sb_insert(stringbuf *sb, int index, const char *str) {
	if (index >= sb->last)
		sb_append(sb, str);
	else {
		int len = strlen(str);

		sb_insert_space(sb, index, len);
		memcpy(sb->data + index, str, len);
#ifdef USE_UTF8
		sb->chars += utf8_strlen(str, len);
#endif
	}
}

void sb_delete(stringbuf *sb, int index, int len) {
	if (index < sb->last) {
		char *pos = sb->data + index;
		if (len < 0)
			len = sb->last;

		sb_delete_space(sb, pos - sb->data, len);
	}
}

void sb_clear(stringbuf *sb) {
	if (sb->data) {
		sb->data[0] = 0;
		sb->last = 0;
#ifdef USE_UTF8
		sb->chars = 0;
#endif
	}
}