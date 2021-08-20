/* th_libc.c provides hooks for stdlib functions. */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

extern int port;
void fatalep(char *s);

int
th_strncmp(const char *str1, const char *str2, size_t n) {
	return strncmp(str1, str2, n);
}

char*
th_strncpy(char *dest, const char *src, size_t n) {
	return strncpy(dest, src, n);
}

size_t
th_strnlen(const char *str, size_t maxlen) {
	size_t strnlen(const char *__string, size_t __maxlen);
	return strnlen(str, maxlen);
}

char*
th_strcat(char *dest, const char *src) {
	return strcat(dest, src);
}

char*
th_strtok(char *str1, const char *sep) {
	return strtok(str1, sep);
}

int
th_atoi(const char *str) {
	return atoi(str);
}

void*
th_memset(void *b, int c, size_t len) {
	return memset(b, c, len);
}

void*
th_memcpy(void *dst, const void *src, size_t n) {
	return memcpy(dst, src, n);
}

int
th_vprintf(const char *format, va_list ap) {
	int vdprintf(int __fd, const char *__restrict __fmt, __gnuc_va_list __arg);
	vdprintf(2, format, ap);
	return vdprintf(port, format, ap);
}

void
th_printf(const char *p_fmt, ...) {
	va_list args;
	va_start(args, p_fmt);
	(void)th_vprintf(p_fmt, args); /* ignore return */
	va_end(args);
}

char
th_getchar() {
	static char buf[256];
	static size_t idx = 0, n = 0;

	if(idx < n)
		return buf[idx++];

	if((n = read(port, buf, sizeof(buf))) < 0) {
		fatalep("th_getchar");
	}
	idx = 0;
	return buf[idx++];
}
