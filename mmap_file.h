// REVIEWED
#pragma once
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <string>
#include "nocopy.h"
#include "utils.h"

struct mmap_file_t
{
	NOCOPY(mmap_file_t);
	mmap_file_t() = delete;
	mmap_file_t(const std::string &filename);
	bool valid() const;
	~mmap_file_t();

	int fd;
	void *addr;
	off_t len;
};


