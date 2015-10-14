#ifndef __IZUMO_MISC_STRING_H__
#define __IZUMO_MISC_STRING_H__

#include <stdlib.h>
#include <string.h>

#define IZM_STR_P(str) (int)((str).len), (str).data

typedef struct {
	char	*data;
	size_t	len;
} izm_string;

static inline void
izm_string_init(izm_string *str)
{
	str->len = 0;
	str->data = NULL;
}

static inline void
izm_string_set(izm_string *str, char *data, size_t len)
{
	str->data = data;
	str->len = len;
}

static inline void
izm_string_from_cstr(izm_string *str, char *cstr)
{
	str->data = cstr;
	str->len = strlen(cstr);
}

static inline int
izm_strcmp(izm_string a, izm_string b)
{
	return strncmp(a.data, b.data,
		       a.len < b.len ? a.len : b.len);
}

static inline int
izm_strequ(izm_string a, izm_string b)
{
	return izm_strcmp(a, b) == 0;
}

static inline izm_string
izm_cstrcpy(izm_string a, char *cstr, size_t len)
{
	len = a.len > len ? a.len : len;
	strncpy(cstr, a.data, len);
	return a;
}

static inline int
izm_cstrcmp(izm_string a, const char *cstr)
{
	size_t len = strlen(cstr);
	len = a.len > len ? len : a.len;
	return strncmp(a.data, cstr, len);
}

static inline int
izm_cstrcasecmp(izm_string a, const char *cstr)
{
	size_t len = strlen(cstr);
	len = a.len > len ? len : a.len;
	return strncasecmp(a.data, cstr, len);
}

#endif	/* __IZUMO_MISC_STRING_H__ */
