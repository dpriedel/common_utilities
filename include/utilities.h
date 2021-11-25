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
#include <filesystem>
#include <locale>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <boost/assert.hpp>

#include <date/date.h>
#include <fmt/format.h>

#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>

namespace fs = std::filesystem;

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

// here's a ranges based version of the split code above.
// this advantage of using this version is that it is lazy -- 
// no requirement to split the whole input up front.

template<typename T>
inline auto rng_split_string(std::string_view string_data, char delim)
    requires std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>
{
    auto splitter = string_data
        | ranges::views::split(delim)
        | ranges::views::transform([](auto &&item_rng)
            {
                return T(&*item_rng.begin(), ranges::distance(item_rng));
            });

    return splitter;
}

// custom fmtlib formatter for filesytem paths

template <> struct fmt::formatter<std::filesystem::path>: formatter<std::string> {
  // parse is inherited from formatter<string_view>.
  template <typename FormatContext>
  auto format(const std::filesystem::path& p, FormatContext& ctx) {
    std::string f_name = p.string();
    return formatter<std::string>::format(f_name, ctx);
  }
};

// custom fmtlib formatter for date year_month_day

template <> struct fmt::formatter<date::year_month_day>: formatter<std::string> {
  // parse is inherited from formatter<string_view>.
  template <typename FormatContext>
  auto format(date::year_month_day d, FormatContext& ctx) {
    std::string s_date = date::format("%Y-%m-%d", d);
    return formatter<std::string>::format(s_date, ctx);
  }
};

enum class UpOrDown { e_Down, e_Up };

enum class UseAdjusted { e_Yes, e_No };

// some to/from date parsing functions

std::string TimePointToYMDString(std::chrono::system_clock::time_point a_time_point);

std::chrono::system_clock::time_point StringToTimePoint(std::string_view input_format, std::string_view the_date);

date::year_month_day StringToDateYMD(std::string_view input_format, std::string_view the_date);

// utility to convert a date::year_month_day to a string
// using Howard Hinnant's date library
// this function includes the time zone

std::string LocalDateTimeAsString(std::chrono::system_clock::time_point a_date_time);

std::string DateTimeAsString(std::chrono::system_clock::time_point a_date_time);

// this function will generate a pair of dates which spans the specified number of business days (ignoring holidays). 
// the starting date is included.

std::pair<date::year_month_day, date::year_month_day> ConstructeBusinessDayRange(date::year_month_day start_from, int how_many_business_days, UpOrDown order);

std::string LoadDataFileForUse (const fs::path& file_name);

#endif   // ----- #ifndef _UTILITIES_INC_  ----- 

