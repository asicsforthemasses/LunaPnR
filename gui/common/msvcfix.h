#pragma once

#ifdef NO_SSIZE_T
#include <type_traits>
typedef std::make_signed<size_t>::type ssize_t;
#endif