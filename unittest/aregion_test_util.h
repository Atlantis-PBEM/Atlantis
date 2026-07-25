// Shared helpers for the aregion_*_test.cpp suites.
//
// These are header-only (inline) so several test translation units can include them without
// a link clash. They stay minimal on purpose -- fixture builders that vary per suite live in
// that suite's own anonymous namespace.
#ifndef AREGION_TEST_UTIL_H
#define AREGION_TEST_UTIL_H

#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include "aregion.h"
#include "fileio.h"

namespace aregion_test {

	// Run a report-writing call against an Areport backed by a temp file, then return the
	// file's contents. This is the technique for unit-testing any Write*/report method:
	// capture the emitted text and assert on it.
	inline std::string capture(std::function<void(Areport *)> emit)
	{
		const char *scratch = "aregion_capture.tmp";
		std::remove(scratch); // Areport::OpenByName refuses a non-empty file
		Areport rep;
		rep.OpenByName(scratch);
		emit(&rep);
		rep.Close();

		std::ifstream in(scratch);
		std::stringstream ss;
		ss << in.rdbuf();
		in.close();
		std::remove(scratch);
		return ss.str();
	}

	// Capture everything a call writes to std::cout (Awrite goes there). Used for the
	// statistics methods, which report via Awrite and have no return value.
	inline std::string captureCout(std::function<void()> fn)
	{
		std::stringstream buffer;
		std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());
		fn();
		std::cout.rdbuf(old);
		return buffer.str();
	}

	inline bool contains(const std::string &haystack, const std::string &needle)
	{
		return haystack.find(needle) != std::string::npos;
	}

}

#endif
