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
#include <map>
#include <string>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <vector>

#include <boost/assert.hpp>

#include <date/date.h>
#include <date/chrono_io.h>
#include <date/tz.h>

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <json/json.h>

#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>

namespace fs = std::filesystem;

#include "DDecQuad.h"

// keep our database related parms together

// for streamed data, we want to be able to show a graphic of
// price history along with the P&F graphic for each ticker that
// we are monitoring.

struct streamed_prices
{
    std::vector<int64_t> timestamp_ = {};
    std::vector<double> price_ = {};
    std::vector<int32_t> signal_type_ = {};
};

using PF_StreamedPrices = std::map<std::string, streamed_prices>;

struct StockDataRecord
{
	std::string date_;
	std::string symbol_;
	DprDecimal::DDecQuad open_;
	DprDecimal::DDecQuad high_;
	DprDecimal::DDecQuad low_;
	DprDecimal::DDecQuad close_;
};

struct DateCloseRecord
{
	date::utc_clock::time_point date_;
	DprDecimal::DDecQuad close_;
};

struct MultiSymbolDateCloseRecord
{
    std::string symbol_;
	date::utc_clock::time_point date_;
    DprDecimal::DDecQuad close_;
};

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
        return v.data();
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
	for (size_t it = 0; it < string_data.size(); ++it)
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

// a (hopefully) efficient way to read an entire file into a string.  Does a binary read. 

std::string LoadDataFileForUse (const fs::path& file_name);

enum class UpOrDown { e_Down, e_Up };

enum class UseAdjusted { e_Yes, e_No };

// some basic time utilities 


// some to/from date parsing functions

std::string UTCTimePointToLocalTZHMSString(date::utc_clock::time_point a_time_point);

date::utc_clock::time_point StringToUTCTimePoint(std::string_view input_format, std::string_view the_date);

date::year_month_day StringToDateYMD(std::string_view input_format, std::string_view the_date);

// some time values  for accessing streaming market data.

using US_MarketTime = date::zoned_seconds;

US_MarketTime GetUS_MarketOpenTime(const date::year_month_day& a_day);
US_MarketTime GetUS_MarketCloseTime(const date::year_month_day& a_day);

enum class US_MarketStatus { e_NotOpenYet, e_ClosedForDay, e_OpenForTrading, e_NonTradingDay};

US_MarketStatus GetUS_MarketStatus(std::string_view local_time_zone_name, date::local_seconds a_time);

// some more date related functions related to our point and figure project 
//
// Generate a list of US market holidays for the given year 
// NOTE: List will contain day holidays are observed by market, not necessarily
// the actual date of the holiday

using US_MarketHoliday = std::pair<const std::string, const date::year_month_day>;
using US_MarketHolidays = std::vector<US_MarketHoliday>;

US_MarketHolidays MakeHolidayList(date::year which_year);

// see if the US Stock market is open on the given day.

bool IsUS_MarketOpen(const date::year_month_day& a_day);

// this function will generate a list of dates which spans the specified number of business days, optionally
// taking holidays into account.  the starting date is included.
// This is useful for generating test data.

std::vector<date::year_month_day> ConstructeBusinessDayList(date::year_month_day start_from, int how_many_business_days, UpOrDown order, const US_MarketHolidays* holidays=nullptr);

// this function will generate a pair of dates which spans the specified number of business days, optionally
// taking holidays into account.  the starting date is included.

std::pair<date::year_month_day, date::year_month_day> ConstructeBusinessDayRange(date::year_month_day start_from, int how_many_business_days, UpOrDown order, const US_MarketHolidays* holidays=nullptr);

    // bridge between Tiingo price history data and DB price history data

std::vector<StockDataRecord> ConvertJSONPriceHistory(const std::string& symbol, const Json::Value& the_data, uint32_t how_many_days, UseAdjusted use_adjusted);

// help us out for testing

// custom fmtlib formatter for filesytem paths

template <> struct fmt::formatter<std::filesystem::path>: formatter<std::string> {
  // parse is inherited from formatter<string>.
  auto format(const std::filesystem::path& p, fmt::format_context& ctx) {
    std::string f_name;
	fmt::format_to(std::back_inserter(f_name), "{}", p.string());
    return formatter<std::string>::format(f_name, ctx);
  }
};

// custom fmtlib formatter for date year_month_day
// need this till GCC12 when chrono as new features ??

template <> struct fmt::formatter<date::year_month_day>: formatter<std::string> {
  // parse is inherited from formatter<string>.
  auto format(const date::year_month_day& d, fmt::format_context& ctx) {
	std::string s_date;
	fmt::format_to(std::back_inserter(s_date), "{}", date::format("%F", d));
    return formatter<std::string>::format(s_date, ctx);
  }
};

// custom fmtlib formatter for UTC timepoint

template <> struct fmt::formatter<date::utc_time<date::utc_clock::duration>>: fmt::formatter<std::chrono::time_point<std::chrono::system_clock>> {
  auto format(const date::utc_time<date::utc_clock::duration>& val, fmt::format_context& ctx) {
	  const auto xxx = date::clock_cast<std::chrono::system_clock>(val);
    return formatter<std::tm, char>::format(fmt::gmtime(xxx), ctx); }

};

// custom formatter for PriceDataRecord 

template <> struct fmt::formatter<StockDataRecord>: formatter<std::string> {
  // parse is inherited from formatter<string>.
  auto format(const StockDataRecord& pdr, fmt::format_context& ctx) {
	std::string record;
	fmt::format_to(std::back_inserter(record), "{}, {}, {}, {}, {}, {}",
			pdr.date_,
			pdr.symbol_,
			pdr.open_,
			pdr.high_,
			pdr.low_,
			pdr.close_);
    return formatter<std::string>::format(record, ctx);
  }
};

// custom fmtlib formatter for US market status.

template <> struct fmt::formatter<US_MarketStatus>: formatter<std::string> {
  // parse is inherited from formatter<string>.
  auto format(US_MarketStatus status, fmt::format_context& ctx) {
    std::string s;
    switch(status)
    {
        using enum US_MarketStatus;
        case e_NotOpenYet:
			fmt::format_to(std::back_inserter(s), "{}", "US markets not open yet");
            break;

        case e_ClosedForDay:
			fmt::format_to(std::back_inserter(s), "{}", "US markets closed for the day");
            break;

        case e_NonTradingDay:
			fmt::format_to(std::back_inserter(s), "{}", "Non-trading day");
            break;

        case e_OpenForTrading:
			fmt::format_to(std::back_inserter(s), "{}", "US markets are open for trading");
            break;
    };
    return formatter<std::string>::format(s, ctx);
  }
};

inline std::ostream& operator<<(std::ostream& os, US_MarketStatus status)
{
    fmt::format_to(std::ostream_iterator<char>{os}, "{}", status);

	return os;
}

#endif   // ----- #ifndef _UTILITIES_INC_  ----- 

