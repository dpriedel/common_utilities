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

#ifndef _UTILITIES_INC_
#define _UTILITIES_INC_

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <format>
#include <iterator>
#include <locale>
#include <map>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <boost/assert.hpp>

#include <json/json.h>

#include <decimal.hh>

namespace fs = std::filesystem;

// mpdecimal does not include functions for working with floating point types
// so provided minimal interface through character representatons as intermediaries.
//
decimal::Decimal dbl2dec(double number);

double dec2dbl(const decimal::Decimal& dec);

// convenience function to construct a Decimal from a string view

decimal::Decimal sv2dec(std::string_view text);

// keep our database related parms together

// for streamed data, we want to be able to show a graphic of
// price history along with the P&F graphic for each ticker that
// we are monitoring.

struct StreamedPrices
{
    std::vector<int64_t> timestamp_seconds_;
    std::vector<double> price_;
    std::vector<int32_t> signal_type_;
};

using PF_StreamedPrices = std::map<std::string, StreamedPrices>;

// we keep track of overall movement for streamed data --
// the open and most recent price for each symbol.

struct StreamedSummary
{
    double opening_price_{0.};
    double latest_price_{0.};
    int32_t curent_signal_type_{0};
};

using PF_StreamedSummary = std::map<std::string, StreamedSummary>;

struct StockDataRecord
{
    std::string date_;
    std::string symbol_;
    decimal::Decimal open_;
    decimal::Decimal high_;
    decimal::Decimal low_;
    decimal::Decimal close_;
};

struct TopOfBookOpenAndLastClose
{
    std::string symbol_;
    std::chrono::utc_clock::time_point time_stamp_nsecs_;
    decimal::Decimal open_;
    decimal::Decimal last_;
    decimal::Decimal previous_close_;
};

struct DateCloseRecord
{
    std::chrono::utc_clock::time_point date_;
    decimal::Decimal close_;
};

struct MultiSymbolDateCloseRecord
{
    std::string symbol_;
    std::chrono::utc_clock::time_point date_;
    decimal::Decimal close_;
};

// This ctype facet does NOT classify spaces and tabs as whitespace
// from cppreference example

struct line_only_whitespace : std::ctype<char>
{
    static const mask* make_table()
    {
        // make a copy of the "C" locale table
        static std::vector<mask> v(classic_table(), classic_table() + table_size);
        v['\t'] &= ~space;  // tab will not be classified as whitespace
        v[' '] &= ~space;   // space will not be classified as whitespace
        return v.data();
    }
    explicit line_only_whitespace(std::size_t refs = 0) : ctype(make_table(), false, refs) {}
};

//  let's do a little 'template normal' programming again

// function to split a string on a delimiter and return a vector of items.
// use concepts to restrict to strings and string_views.

template <typename T>
inline std::vector<T> split_string(std::string_view string_data, std::string_view delim)
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

template <typename T>
inline auto rng_split_string(std::string_view string_data, std::string_view delim)
    requires std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>
{
    // namespace rng = std::ranges;
    namespace vws = std::ranges::views;

    auto splitter = vws::split(string_data, delim) |
                    vws::transform([](auto&& item_rng) { return T(item_rng.begin(), item_rng.end()); });

    return splitter;
}

// a (hopefully) efficient way to read an entire file into a string.  Does a binary read.

std::string LoadDataFileForUse(const fs::path& file_name);

// common code to read in some JSON data and parse it out.

Json::Value ReadAndParsePF_ChartJSONFile(const fs::path& symbol_file_name);

enum class UpOrDown : int32_t
{
    e_Down,
    e_Up
};

enum class UseAdjusted : int32_t
{
    e_Yes,
    e_No
};

// some basic time utilities

// some to/from date parsing functions

std::string UTCTimePointToLocalTZHMSString(std::chrono::utc_clock::time_point a_time_point);

std::chrono::utc_time<std::chrono::nanoseconds> StringToUTCTimePoint(std::string_view input_format,
                                                                     std::string_view the_date);

std::chrono::year_month_day StringToDateYMD(std::string_view input_format, std::string_view the_date);

// some time values  for accessing streaming market data.

using US_MarketTime = std::chrono::zoned_seconds;

US_MarketTime GetUS_MarketOpenTime(const std::chrono::year_month_day& a_day);
US_MarketTime GetUS_MarketCloseTime(const std::chrono::year_month_day& a_day);

enum class US_MarketStatus : int32_t
{
    e_NotOpenYet,
    e_ClosedForDay,
    e_OpenForTrading,
    e_NonTradingDay
};

US_MarketStatus GetUS_MarketStatus(std::string_view local_time_zone_name, std::chrono::local_seconds a_time);

// some more date related functions related to our point and figure project
//
// Generate a list of US market holidays for the given year
// NOTE: List will contain day holidays are observed by market, not necessarily
// the actual date of the holiday

using US_MarketHoliday = std::pair<const std::string, const std::chrono::year_month_day>;
using US_MarketHolidays = std::vector<US_MarketHoliday>;

US_MarketHolidays MakeHolidayList(std::chrono::year which_year);

// see if the US Stock market is open on the given day.

bool IsUS_MarketOpen(const std::chrono::year_month_day& a_day);

// this function will generate a list of dates which spans the specified number of business days, optionally
// taking holidays into account.  the starting date is included.
// This is useful for generating test data.

std::vector<std::chrono::year_month_day> ConstructeBusinessDayList(std::chrono::year_month_day start_from,
                                                                   size_t how_many_business_days, UpOrDown order,
                                                                   const US_MarketHolidays* holidays = nullptr);

// this function will generate a pair of dates which spans the specified number of business days, optionally
// taking holidays into account.  the starting date is included.

std::pair<std::chrono::year_month_day, std::chrono::year_month_day> ConstructeBusinessDayRange(
    std::chrono::year_month_day start_from, int how_many_business_days, UpOrDown order,
    const US_MarketHolidays* holidays = nullptr);

// bridge between Tiingo price history data and DB price history data

std::vector<StockDataRecord> ConvertJSONPriceHistory(const std::string& symbol, const Json::Value& the_data,
                                                     uint32_t how_many_days, UseAdjusted use_adjusted);

// help us out for testing

template <>
struct std::formatter<decimal::Decimal> : std::formatter<std::string>
{
    // parse is inherited from formatter<string>.
    auto format(const decimal::Decimal& dec, std::format_context& ctx) const
    {
        std::string d;
        std::format_to(std::back_inserter(d), "{}", dec.format("f"));
        return formatter<std::string>::format(d, ctx);
    }
};

// custom fmtlib formatter for filesytem paths

template <>
struct std::formatter<std::filesystem::path> : std::formatter<std::string>
{
    // parse is inherited from formatter<string>.
    auto format(const std::filesystem::path& p, std::format_context& ctx) const
    {
        std::string f_name;
        std::format_to(std::back_inserter(f_name), "{}", p.string());
        return formatter<std::string>::format(f_name, ctx);
    }
};

// custom formatter for PriceDataRecord

template <>
struct std::formatter<StockDataRecord> : std::formatter<std::string>
{
    // parse is inherited from formatter<string>.
    auto format(const StockDataRecord& pdr, std::format_context& ctx) const
    {
        std::string record;
        std::format_to(std::back_inserter(record), "{}, {}, {}, {}, {}, {}", pdr.date_, pdr.symbol_, pdr.open_,
                       pdr.high_, pdr.low_, pdr.close_);
        return formatter<std::string>::format(record, ctx);
    }
};

// custom fmtlib formatter for US market status.

template <>
struct std::formatter<US_MarketStatus> : std::formatter<std::string>
{
    // parse is inherited from formatter<string>.
    auto format(US_MarketStatus status, std::format_context& ctx) const
    {
        std::string s;
        switch (status)
        {
            using enum US_MarketStatus;
            case e_NotOpenYet:
                std::format_to(std::back_inserter(s), "{}", "US markets not open yet");
                break;

            case e_ClosedForDay:
                std::format_to(std::back_inserter(s), "{}", "US markets closed for the day");
                break;

            case e_NonTradingDay:
                std::format_to(std::back_inserter(s), "{}", "Non-trading day");
                break;

            case e_OpenForTrading:
                std::format_to(std::back_inserter(s), "{}", "US markets are open for trading");
                break;
        };
        return formatter<std::string>::format(s, ctx);
    }
};

inline std::ostream& operator<<(std::ostream& os, US_MarketStatus status)
{
    std::format_to(std::ostream_iterator<char>{os}, "{}", status);

    return os;
}

#endif  // ----- #ifndef _UTILITIES_INC_  -----
