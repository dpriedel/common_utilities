// =====================================================================================
//
//       Filename:  utilities.h
//
//    Description:  various bits of useful code from several projects
//
//        Version:  1.0
//        Created:  09/20/2021 01:10:32 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  David P. Riedel (), driedel@cox.net
//        License:  GNU General Public License -v3
//
// =====================================================================================

#ifndef  _UTILITIES_INC_
#define  _UTILITIES_INC_

#include <chrono>
#include <locale>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <boost/assert.hpp>

#include <date/date.h>
#include <fmt/format.h>

// This ctype facet does NOT classify spaces and tabs as whitespace
// from cppreference example

struct line_only_whitespace : std::ctype<char>
{
    static const mask* make_table()
    {
        // make a copy of the "C" locale table
        static std::vector<mask> v(classic_table(), classic_table() + table_size);
        v['\t'] &= ~space;      // tab will not be classified as whitespace
        v[' '] &= ~space;       // space will not be classified as whitespace
        return &v[0];
    }
    explicit line_only_whitespace(std::size_t refs = 0) : ctype(make_table(), false, refs) {}
};

//  let's do a little 'template normal' programming again

// function to split a string on a delimiter and return a vector of items.
// use concepts to restrict to strings and string_views.

template<typename T>
inline std::vector<T> split_string(std::string_view string_data, char delim)
    requires std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>
{
    std::vector<T> results;
	for (auto it = 0; it != T::npos; ++it)
	{
		auto pos = string_data.find(delim, it);
        if (pos != T::npos)
        {
    		results.emplace_back(string_data.substr(it, pos - it));
        }
        else
        {
    		results.emplace_back(string_data.substr(it));
            break;
        }
		it = pos;
	}
    return results;
}

std::chrono::system_clock::time_point StringToTimePoint(std::string_view input_format, std::string_view the_date);

date::year_month_day StringToDateYMD(std::string_view input_format, std::string_view the_date);

// this function includes the time zone

std::string LocalDateTimeAsString(std::chrono::system_clock::time_point a_date_time);

std::string DateTimeAsString(std::chrono::system_clock::time_point a_date_time);


#endif   // ----- #ifndef _UTILITIES_INC_  ----- 

