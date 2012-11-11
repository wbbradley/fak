// REVIEWED
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "mmap_file.h"
#include "cmd_options.h"

const char *option_dir = "dir";
const char *option_verbose = "verbose";
const char *option_find = "find";
const char *option_replace = "replace";
const char *option_filenames = "filenames";

cmd_option_t cmd_options[] =
{
	{ option_dir, "--dir" /*opt*/, true /*mandatory*/, true /*has_data*/ },
	{ option_find, "--find" /*opt*/, true /*mandatory*/, true /*has_data*/ },
	// TODO make a mode that is just find
	{ option_replace, "--replace" /*opt*/, true /*mandatory*/, true /*has_data*/ },
	{ option_filenames, "-F" /*opt*/, false /*mandatory*/, false /*has_data*/ },
	{ option_verbose, "-v" /*opt*/, false /*mandatory*/, false /*has_data*/ },
};

void string_replace(
		const std::string &before,
	   	const std::string &after,
	   	const std::string &input_file,
	   	const std::string &output_file)
{
	mmap_file_t mmap_file(input_file);

	if (mmap_file.valid())
	{
		auto pch = (const char *)mmap_file.addr;
#ifdef EX_DEBUG
		for (int i = 0; i != mmap_file.len; ++i)
		{
			if (pch[i] != 0)
				fprintf(stdout, "%c", pch[i]);
		}
#endif
		std::ofstream ofs;
		ofs.open(output_file.c_str(), std::ios_base::binary);
		if (ofs.good())
		{
			const char * const pch_end = ((const char *)mmap_file.addr) + mmap_file.len;
			streamed_replace(pch, pch_end, before, after, ofs);
		}
		else
		{
			fprintf(stderr, "couldn't open %s\n", output_file.c_str());
			exit(-2);
		}
		ofs.close();
	}
	else
	{
		fprintf(stderr, "couldn't open %s\n", input_file.c_str());
	}
}

int main(int argc, char *argv[])
{
	cmd_options_t options;
	if (!get_options(argc, argv, cmd_options, countof(cmd_options), options))
		return EXIT_FAILURE;

	std::string dir;
	if (!get_option(options, option_dir, dir))
		return EXIT_FAILURE;

	std::string find;
	if (!get_option(options, option_find, find))
		return EXIT_FAILURE;

	std::string replace;
	if (!get_option(options, option_replace, replace))
		return EXIT_FAILURE;

	fprintf(stderr, "TEST: dir is %s, find is %s, replace is %s\n",
			dir.c_str(), find.c_str(), replace.c_str());

	// TODO loop over all files in dir building up a dictionary of filenames
	// TODO loop over all filenames in dictionary and run string_replace on the file
	// TODO if the filename contains the "before" string, and we're doing filename replacement, then do that, too
	// string_replace(before, after, input_file, output_file);

	return 0;
}

