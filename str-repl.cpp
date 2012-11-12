// REVIEWED
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "mmap_file.h"
#include "cmd_options.h"
#include "disk.h"

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

bool string_replace(
		const std::string &before,
		const std::string &after,
		const std::string &filename)
{
	std::stringstream ss(std::ios_base::out | std::ios_base::binary);
	mmap_file_t mmap_file(filename);

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
		const char * const pch_end = ((const char *)mmap_file.addr) + mmap_file.len;
		if (streamed_replace(pch, pch_end, before, after, ss))
		{
			std::ofstream ofs;
			std::string temp_file("/var/tmp/fak.tmp");
			ofs.open(temp_file.c_str(), std::ios_base::binary);
			if (ofs.good())
			{
				ofs.write(ss.str().c_str(), ss.str().size());
			}
			else
			{
				dlog(log_error, "couldn't open %s\n", temp_file.c_str());
			}
			ofs.close();
			mmap_file.close();

			/* we've written out the file to the string stream */
			if (rename(temp_file.c_str(), filename.c_str()) == 0)
			{
				return true;
			}
			else
			{
				check_errno("string_replace file rename");
			}
		}
		else
		{
			/* no match found */
		}
	}
	else
	{
		dlog(log_error, "couldn't open %s\n", filename.c_str());
	}

	return false;
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

	dlog(log_info, "TEST: dir is %s, find is %s, replace is %s\n",
			dir.c_str(), find.c_str(), replace.c_str());

	std::vector<std::string> filenames;

	for_each_file(dir,
		[&filenames](const std::string &name, const for_each_file_stat_t &file_stat, for_each_control_t &control) {
			control.recurse = true;
			debug_ex(dlog(log_info, "found file %s\n", name.c_str()));
			if (file_stat.regular_file())
				filenames.push_back(name);
		});

	for (auto &filename : filenames)
	{
		if (string_replace(find, replace, filename))
		{
			dlog(log_info, "made replacements in %s\n", filename.c_str());
		}
	}

	// TODO loop over all filenames in dictionary and run string_replace on the file

	return 0;
}

