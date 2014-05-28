// REVIEWED
#pragma once

#ifndef MACOS
#include <tr1/unordered_set>
#include <tr1/unordered_map>
using std::tr1::unordered_set;
using std::tr1::unordered_map;
#else
#include <unordered_set>
#include <unordered_map>
using std::unordered_set;
using std::unordered_map;
#endif

