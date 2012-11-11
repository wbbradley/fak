#include <assert.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include "utils.h"
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <iomanip>
#include "logger_decls.h"

#define case_error(error) case error: error_string = #error; break

bool check_errno(const char *tag)
{
	int err = errno;
	if (err == 0)
		return true;
	const char *error_string = "unknown";
	switch (err)
	{
		case_error(EACCES);
		case_error(EAFNOSUPPORT);
		case_error(EISCONN);
		case_error(EMFILE);
		case_error(ENFILE);
		case_error(ENOBUFS);
		case_error(ENOMEM);
		case_error(EPROTO);
		case_error(EHOSTDOWN);
		case_error(EHOSTUNREACH);
		case_error(ENETUNREACH);
		case_error(EPROTONOSUPPORT);
		case_error(EPROTOTYPE);
		case_error(EDQUOT);
		case_error(EAGAIN);
		case_error(EBADF);
		case_error(ECONNRESET);
		case_error(EFAULT);
		case_error(EINTR);
		case_error(EINVAL);
		case_error(ENETDOWN);
		case_error(ENOTCONN);
		case_error(ENOTSOCK);
		case_error(EOPNOTSUPP);
		case_error(ETIMEDOUT);
		case_error(EMSGSIZE);
		case_error(ECONNREFUSED);
	};
	if (err == -1)
		err = errno;

	log(log_info, "check_errno : %s %s %s\n", tag, error_string, strerror(err));

	return false;
}

void streamed_replace(
		const char *pch,
	   	const char * const pch_end,
	   	const std::string &before,
	   	const std::string &after,
	   	std::ostream &ofs)
{
	// TODO get rid of this poor man's ascii -> unicode to enable non-latin code pages, etc...
	static_assert(sizeof(int16_t) == 2, "doh!");
	std::basic_string<int16_t> wbefore, wafter;
	std::basic_string<int16_t> wbefore_be, wafter_be;
	std::copy(before.begin(), before.end(), std::back_inserter(wbefore));
	std::copy(after.begin(), after.end(), std::back_inserter(wafter));
	wbefore_be = wbefore;
	wafter_be = wafter;
	for (auto &wch : wbefore_be)
		std::swap(*(char *)&wch, *(((char *)&wch) + 1));
	for (auto &wch : wafter_be)
		std::swap(*(char *)&wch, *(((char *)&wch) + 1));

	std::vector<const char *> nexts;
	assert(((char *)wbefore.c_str())[0] == 0 || ((char *)wbefore.c_str())[1] == 0);
	while (pch < pch_end)
	{
		const char *wch_next = (const char *)memmem(pch, pch_end - pch, &wbefore[0], wbefore.size() * sizeof(wbefore[0]));
		const char *wch_next_be = (const char *)memmem(pch, pch_end - pch, &wbefore_be[0], wbefore_be.size() * sizeof(wbefore[0]));
		const char *pch_next = (const char *)memmem(pch, pch_end - pch, before.c_str(), before.size());

		nexts.resize(0);

		if (wch_next != nullptr)
			nexts.push_back(wch_next);
		if (wch_next_be != nullptr)
			nexts.push_back(wch_next_be);
		if (pch_next != nullptr)
			nexts.push_back(pch_next);

		std::sort(nexts.begin(), nexts.end());
		if (nexts.size() != 0)
		{
			if (nexts[0] == wch_next)
			{
				ofs.write(pch, wch_next - pch);
				ofs.write((const char *)wafter.c_str(), wafter.size() * sizeof(wafter[0]));
				pch = wch_next + (wbefore.size() * sizeof(wbefore[0]));
			}
			if (nexts[0] == wch_next_be)
			{
				ofs.write(pch, wch_next_be - pch);
				ofs.write((const char *)wafter_be.c_str(), wafter_be.size() * sizeof(wafter_be[0]));
				pch = wch_next_be + (wbefore_be.size() * sizeof(wbefore_be[0]));
			}
			else if (nexts[0] == pch_next)
			{
				ofs.write(pch, pch_next - pch);
				ofs.write(after.c_str(), after.size());
				pch = pch_next + before.size();
			}
		}
		else
		{
			ofs.write(pch, pch_end - pch);
			pch = pch_end;
		}
	}
}
double get_current_time()
{
	timeval tv;
	gettimeofday(&tv, NULL);
	double time_now = tv.tv_sec + (double(tv.tv_usec) / 1000000.0);
	return time_now;
}

