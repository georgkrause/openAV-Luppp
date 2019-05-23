struct OUTPUT {
	int warnLevel;
	const char *name;
	const char *file;
	const char *func;
	int line;
	const char *format;

	void
	reset();
};