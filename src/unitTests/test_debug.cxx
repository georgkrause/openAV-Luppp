#include "test_debug.hxx"

OUTPUT output;

void
luppp_debug(int warnLevel, const char *name, const char *file, const char *func,
	int line, const char *format, ...)
{
	output.warnLevel = warnLevel;
	output.name = name;
	output.file = file;
	output.func = func;
	output.line = line;
	output.format = format;
}

void
OUTPUT::reset()
{
	warnLevel = 0;
	name = "";
	file = "";
	func = "";
	line = 0;
	format = "";
}