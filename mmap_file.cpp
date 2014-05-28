// REVIEWED
#include "mmap_file.h"
#include <assert.h>
#include <unistd.h>

mmap_file_t::mmap_file_t(const std::string &filename)
{
	addr = MAP_FAILED;
	errno = 0;
	len = 0;
	fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0)
	{
		check_errno("mmap_file_t file open error");
	}
	else
	{
		len = lseek(fd, 0, SEEK_END);
		if (len > 0)
		{
			addr = mmap(nullptr, len, PROT_READ, MAP_SHARED, fd, 0 /*offset*/);
			if (addr == MAP_FAILED)
			{
				::close(fd);
				len = 0;
				fd = -1;
			}
		}
		else
		{
			check_errno("mmap_file_t lseek error");
			len = 0;
		}
	}
}

bool mmap_file_t::valid() const
{
	return (addr != MAP_FAILED) && (len > 0);
}

void mmap_file_t::close()
{
	if (addr != MAP_FAILED)
	{
		assert(addr != nullptr);
		if (munmap(addr, len) < 0)
		{
			check_errno("mmap_file_t munmap error");
		}
	}
	if (fd >= 0)
	{
		if (::close(fd) < 0)
		{
			check_errno("mmap_file_t file close error");
		}
	}
	addr = MAP_FAILED;
	fd = -1;
}

mmap_file_t::~mmap_file_t()
{
	close();
}

