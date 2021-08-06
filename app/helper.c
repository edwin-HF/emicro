char * replace(char * src, const char search, const char replace) {
	char *head = src;
	while (*src != '\0') {
		if (*src == search)
			*src = replace;
		src++;
	}
	return head;
}