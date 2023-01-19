#include "cmd_options.h"
#include "disk.h"
#include "mmap_file.h"
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>

const char *option_dir = "dir";
const char *option_verbose = "verbose";
const char *option_find = "find";
const char *option_replace = "replace";
const char *option_filenames = "filenames";
const char *option_no_recurse = "no-recursion";
const char *option_no_color = "no-color";

cmd_option_t cmd_options[] = {
    {option_find, "-f" /*opt*/, true /*mandatory*/, true /*has_data*/},
    {option_dir, "-d" /*opt*/, false /*mandatory*/, true /*has_data*/},
    {option_replace, "-r" /*opt*/, false /*mandatory*/, true /*has_data*/},
    {option_no_recurse, "--no-recurse" /*opt*/, false /*mandatory*/,
     false /*has_data*/},
    {option_filenames, "-F" /*opt*/, false /*mandatory*/, false /*has_data*/},
    {option_no_color, "--no-color" /*opt*/, false /*mandatory*/,
     false /*has_data*/},
    {option_verbose, "-v" /*opt*/, false /*mandatory*/, false /*has_data*/},
};

bool string_replace(const std::string &before, const std::string &after,
                    const std::string &filename, bool do_replace,
                    bool use_color) {
  std::stringstream ss(std::ios_base::out | std::ios_base::binary);
  mmap_file_t mmap_file(filename);

  if (mmap_file.valid()) {
    auto pch = (const char *)mmap_file.addr;
    auto len = mmap_file.len;

    if ((*pch == -1) && (len > 1) && (*(pch + 1) == -2)) {
      /* skip the BOM */
      pch += 2;
      len -= 2;

      // TODO only search with Unicode search
    } else if ((*pch == -2) && (len > 1) && (*(pch + 1) == -1)) {
      /* skip the BOM */
      pch += 2;
      len -= 2;

      // TODO only search with Unicode search
    }

#ifdef EX_DEBUG
    for (int i = 0; i != len; ++i) {
      if (pch[i] != 0)
        fprintf(stdout, "%c", pch[i]);
    }
#endif
    const char *const pch_end = (pch + len);
    if (streamed_replace(filename, pch, pch_end, before, after, ss,
                         true /*print_matches*/, use_color /*pretty_print*/,
                         do_replace) &&
        do_replace) {
      std::ofstream ofs;
      std::string temp_file("/var/tmp/fak.tmp");
      ofs.open(temp_file.c_str(), std::ios_base::binary);
      if (ofs.good()) {
        ofs.write(ss.str().c_str(), ss.str().size());
      } else {
        debug_ex(dlog(log_error, "couldn't open %s\n", temp_file.c_str()));
      }
      ofs.close();
      mmap_file.close();

      /* we've written out the file to the string stream */
      if (rename(temp_file.c_str(), filename.c_str()) == 0) {
        return true;
      } else {
        check_errno("string_replace file rename");
      }
    } else {
      /* no match found */
    }
  } else {
    debug_ex(dlog(log_error, "couldn't open %s\n", filename.c_str()));
  }

  return false;
}

const std::string TAGS = "tags";
const std::string BUILD = "build";
const std::string ENV = "env";
const std::string LIB_SUFFIX = ".o";
const std::string SHARED_LIB_SUFFIX = ".so";
const std::string PYC_SUFFIX = ".pyc";

bool valid_file_to_mess_with(const std::string &file_path) {
  std::string leaf_name(leaf_from_file_path(file_path));
  if (leaf_name.size() != 0 && leaf_name[0] == '.') {
    return false;
  }
  if (leaf_name == TAGS || leaf_name == BUILD || leaf_name == ENV) {
    return false;
  }
  if (ends_with(leaf_name, LIB_SUFFIX) || ends_with(leaf_name, PYC_SUFFIX) ||
      ends_with(leaf_name, SHARED_LIB_SUFFIX)) {
    return false;
  }
  return true;
}

int main(int argc, char *argv[]) {
  cmd_options_t options;
  if (!get_options(argc, argv, cmd_options, countof(cmd_options), options)) {
    return EXIT_FAILURE;
  }

  if (get_option_exists(options, option_verbose))
    log_enable(log_error | log_warning | log_info);
  else
    log_enable(log_error);

  bool use_color = isatty(STDOUT_FILENO);
  if (get_option_exists(options, option_no_color))
    use_color = false;

  bool recurse_subdirectories = true;
  if (get_option_exists(options, option_no_recurse))
    recurse_subdirectories = false;

  std::string dir;
  if (!get_option(options, option_dir, dir))
    dir = ".";

  std::string find;
  if (!get_option(options, option_find, find)) {
    return EXIT_FAILURE;
  }

  std::string replace;
  get_option(options, option_replace, replace);
  bool do_replace = (replace.size() != 0);

  dlog(log_info, "dir is %s, find is %s, replace is %s\n", dir.c_str(),
       find.c_str(), replace.c_str());

  std::vector<std::string> filenames;

  for_each_file(dir, [&filenames, recurse_subdirectories](
                         const std::string &name,
                         const for_each_file_stat_t &file_stat,
                         for_each_control_t &control) {
    if (valid_file_to_mess_with(name)) {
      control.recurse = recurse_subdirectories;
      debug_ex(dlog(log_info, "found file %s\n", name.c_str()));
      if (file_stat.regular_file()) {
        filenames.push_back(name);
      }
    }
  });

  for (auto &filename : filenames) {
    if (string_replace(find, replace, filename, do_replace, use_color)) {
      dlog(log_info, "made replacements in %s\n", filename.c_str());
    }
  }

  return EXIT_SUCCESS;
}
