// =====================================================================================
//
//       Filename:  utilities.cpp
//
//    Description:  various bits of usefule code
//
//        Version:  1.0
//        Created:  09/20/2021 01:30:48 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  David P. Riedel (), driedel@cox.net
//        License:  GNU General Public License -v3
//
// =====================================================================================

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <variant>
#include <vector>

#include <date/date.h>  // for from_stream
// #include <date/chrono_io.h>
#include <date/tz.h>

using namespace std::chrono_literals;

namespace rng = std::ranges;
namespace vws = std::ranges::views;

#include "utilities.h"
extern "C"
{
#include "calfaq.h"
}

decimal::Decimal dbl2dec(double number)
{
    std::array<char, 30> buf{};
    if (auto [p, ec] = std::to_chars(buf.data(), buf.data() + buf.size() - 1, number, std::chars_format::fixed);
        ec == std::errc())
    {
        // string is NOT NULL terminated
        *p = '\0';
    }
    else
    {
        throw std::runtime_error(
            std::format("Problem converting double to decimal: {}\n", std::make_error_code(ec).message()));
    }

    return decimal::Decimal{buf.data()};
}

double dec2dbl(const decimal::Decimal& dec)
{
    // I don't see a better way to do this.

    std::string temp = dec.format("f");
    double result = {};
    if (auto [p, ec] = std::from_chars(temp.data(), temp.data() + temp.size(), result); ec != std::errc())
    {
        throw std::runtime_error(
            std::format("Problem converting decimal to double: {}\n", std::make_error_code(ec).message()));
    }
    return result;
}

// convenience function to construct a Decimal from a string view

decimal::Decimal sv2dec(std::string_view text)
{
    constexpr size_t kBufLen = 30;
    BOOST_ASSERT_MSG(text.size() < kBufLen,
                     std::format("String value: {} is too long to convert to Decimal.", text).c_str());
    std::array<char, kBufLen> buf{};
    auto result = std::ranges::copy(text, buf.begin());
    *result.out = '\0';

    return decimal::Decimal{buf.data()};
}

// ===  FUNCTION  ======================================================================
//         Name:  GetUS_MarketOpen
//  Description:
// =====================================================================================
US_MarketTime GetUS_MarketOpenTime(const std::chrono::year_month_day& a_day)
{
    return std::chrono::zoned_seconds("America/New_York", std::chrono::local_days{a_day} + 9h + 30min + 0s);
}  // -----  end of function GetUS_MarketOpen  -----

// ===  FUNCTION  ======================================================================
//         Name:  GetUS_MarketClose
//  Description:
// =====================================================================================
US_MarketTime GetUS_MarketCloseTime(const std::chrono::year_month_day& a_day)
{
    return std::chrono::zoned_seconds{"America/New_York", std::chrono::local_days{a_day} + 16h + 0min + 0s};
}  // -----  end of function GetUS_MarketClose  -----

// ===  FUNCTION  ======================================================================
//         Name:  GetUS_MarketStatus
//  Description:
// =====================================================================================

US_MarketStatus GetUS_MarketStatus(std::string_view local_time_zone_name, std::chrono::local_seconds a_time)
{
    // we convert the local time to US time then check to see if it's a US market holiday.
    // If not, then we check to see if we are within trading hours.

    const auto users_local_time = std::chrono::zoned_seconds(local_time_zone_name, a_time);
    const auto time_in_US = std::chrono::zoned_seconds("America/New_York", users_local_time);

    //    std::cout << "current user's local time: " <<  users_local_time  << '\n';
    //    std::cout << "current user's time in US: " <<  time_in_US  << '\n';

    const std::chrono::year_month_day today_in_US{floor<std::chrono::days>(time_in_US.get_local_time())};

    bool is_market_open = IsUS_MarketOpen(today_in_US);
    if (!is_market_open)
    {
        return US_MarketStatus::e_NonTradingDay;
    }
    auto local_market_open = std::chrono::zoned_seconds(local_time_zone_name, GetUS_MarketOpenTime(today_in_US));
    auto local_market_close = std::chrono::zoned_seconds(local_time_zone_name, GetUS_MarketCloseTime(today_in_US));

    //    std::cout << "Local Market Open: " <<  local_market_open << " Local Market Close: " << local_market_close <<
    //    '\n';

    if (users_local_time.get_sys_time() < local_market_open.get_sys_time())
    {
        return US_MarketStatus::e_NotOpenYet;
    }
    if (users_local_time.get_sys_time() > local_market_close.get_sys_time())
    {
        return US_MarketStatus::e_ClosedForDay;
    }
    return US_MarketStatus::e_OpenForTrading;
}  // -----  end of function GetUS_MarketStatus  -----

std::string UTCTimePointToLocalTZHMSString(std::chrono::utc_clock::time_point a_time_point)
{
    auto t =
        date::zoned_time(std::chrono::current_zone(),
                         floor<std::chrono::seconds>(std::chrono::clock_cast<std::chrono::system_clock>(a_time_point)));
    std::string result = date::format("%H:%M:%S", t);
    return result;
}  // -----  end of function UTCTimePointToLocalTZHMSString  -----

// ===  FUNCTION  ======================================================================
//         Name:  StringToTimePoint
//  Description:
// =====================================================================================

std::chrono::utc_time<std::chrono::nanoseconds> StringToUTCTimePoint(std::string_view input_format,
                                                                     std::string_view the_date)
{
    std::istringstream in{the_date.data()};
    date::utc_clock::time_point tp;
    date::from_stream(in, input_format.data(), tp);
    BOOST_ASSERT_MSG(!in.fail() && !in.bad(), std::format("Unable to parse given date: {}", the_date).c_str());
    std::chrono::utc_time<std::chrono::nanoseconds> tp1{
        std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch())};
    return tp1;
}  // -----  end of method StringToDateYMD  -----

// ===  FUNCTION  ======================================================================
//         Name:  StringToDateYMD
//  Description:
// =====================================================================================

std::chrono::year_month_day StringToDateYMD(std::string_view input_format, std::string_view the_date)
{
    std::istringstream in{the_date.data()};
    date::year_month_day result{};
    date::from_stream(in, input_format.data(), result);
    BOOST_ASSERT_MSG(!in.fail() && !in.bad(), std::format("Unable to parse given date: {}", the_date).c_str());
    std::chrono::year_month_day result1(std::chrono::year{result.year().operator int()},
                                        std::chrono::month{result.month().operator unsigned()},
                                        std::chrono::day{result.day().operator unsigned()});
    return result1;
}  // -----  end of method StringToDateYMD  -----

// ===  FUNCTION  ======================================================================
//         Name:  IsUS_MarketOpen
//  Description:
// =====================================================================================
bool IsUS_MarketOpen(const std::chrono::year_month_day& a_day)
{
    // look for holidays and weekends
    // start with weekends

    std::chrono::weekday d1{std::chrono::sys_days{a_day}};
    if (d1 == std::chrono::Saturday || d1 == std::chrono::Sunday)
    {
        return false;
    }

    auto holidays = MakeHolidayList(a_day.year());
    return rng::find(holidays, a_day, [](const auto& e) { return e.second; }) == holidays.end();
}  // -----  end of function IsUS_MarketOpen  -----

// ===  FUNCTION  ======================================================================
//         Name:  ConstructeBusinessDayList
//  Description:  Generate a start/end pair of dates which included n business days
//                skipping holidays.
// =====================================================================================

std::vector<std::chrono::year_month_day> ConstructeBusinessDayList(std::chrono::year_month_day start_from,
                                                                   size_t how_many_business_days, UpOrDown order,
                                                                   const US_MarketHolidays* holidays)
{
    // we need to do some date arithmetic so we can use our basic 'GetTickerData' method.

    auto days = std::chrono::sys_days{start_from};

    const std::chrono::days day_increment{order == UpOrDown::e_Up ? 1 : -1};

    std::vector<std::chrono::year_month_day> business_days;

    auto IsHoliday = [holidays](const std::chrono::year_month_day& a_day)
    {
        if (holidays == nullptr)
        {
            return false;
        }
        return rng::find(*holidays, a_day, [](const auto& e) { return e.second; }) != holidays->end();
    };

    while (business_days.size() < how_many_business_days)
    {
        auto b_day = std::chrono::weekday{days};
        while (b_day == std::chrono::Saturday || b_day == std::chrono::Sunday || IsHoliday(days))
        {
            days += day_increment;
            b_day = std::chrono::weekday{days};
        }
        business_days.emplace_back(days);
        days += day_increment;
    }

    //    ranges::for_each(business_days, [](const auto& e) { std::cout << e << '\n'; });
    //    std::cout << "how many: " << business_days.size() << '\n';
    return business_days;
}  // -----  end of function ConstructeBusinessDayList  -----

// ===  FUNCTION  ======================================================================
//         Name:  ConstructeBusinessDayRange
//  Description:  Generate a start/end pair of dates which included n business days
//                skipping holidays.
// =====================================================================================

std::pair<std::chrono::year_month_day, std::chrono::year_month_day> ConstructeBusinessDayRange(
    std::chrono::year_month_day start_from, int how_many_business_days, UpOrDown order,
    const US_MarketHolidays* holidays)
{
    // we need to do some date arithmetic so we can use our basic 'GetTickerData' method.

    auto days = std::chrono::sys_days{start_from};

    const std::chrono::days day_increment{order == UpOrDown::e_Up ? 1 : -1};

    auto business_days = ConstructeBusinessDayList(start_from, how_many_business_days, order, holidays);

    return {business_days.front(), business_days.back()};
}  // -----  end of function ConstructeBusinessDayRange  -----

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  LoadDataFileForUse
 *  Description:
 * =====================================================================================
 */
std::string LoadDataFileForUse(const fs::path& file_name)
{
    std::string file_content;  // make room for trailing null
    file_content.reserve(fs::file_size(file_name) + 1);
    std::ifstream input_file{file_name, std::ios_base::in | std::ios_base::binary};
    BOOST_ASSERT_MSG(input_file.is_open(), std::format("Can't open data file: {}.", file_name).c_str());
    //    input_file.read(&file_content[0], file_content.size());
    file_content.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
    input_file.close();

    return file_content;
} /* -----  end of function LoadDataFileForUse  ----- */

Json::Value ReadAndParsePF_ChartJSONFile(const fs::path& file_name)
{
    BOOST_ASSERT_MSG(fs::exists(file_name), std::format("Unable to find JSON file: {}", file_name).c_str());

    const std::string file_content = LoadDataFileForUse(file_name);

    JSONCPP_STRING err;
    Json::Value JSON_data;

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(file_content.data(), file_content.data() + file_content.size(), &JSON_data, &err))
    {
        throw std::runtime_error(std::format("Problem parsing test data file: {}", err));
    }
    return JSON_data;
}  // -----  end of method ReadAndParseJSONFile  -----

// helper type for the visitor function used below (from cppreference)
//
template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

// ===  FUNCTION  ======================================================================
//         Name:  MakeHolidayList
//  Description:
// =====================================================================================
US_MarketHolidays MakeHolidayList(std::chrono::year which_year)
{
    // save us some typing,,,

    using namespace std::chrono_literals;

    using md = std::chrono::month_day;
    using mwd = std::chrono::month_weekday;
    using mwdl = std::chrono::month_weekday_last;

    using ymd = std::chrono::year_month_day;
    using ymwd = std::chrono::year_month_weekday;
    using ymwdl = std::chrono::year_month_weekday_last;

    struct NewYearsDayRule
    {
    };  // New Years day does not fall back to previous year.
    struct EasterRule
    {
    };  // use this to trigger computation needed to find Good Friday.
    struct JuneteenthRule
    {
    };  // use this to trigger computation needed to find Good Friday.

    // here are the rules for constructing each holiday.

    using HolidayRule =
        std::pair<const std::string,
                  std::variant<const md, const mwd, const mwdl, NewYearsDayRule, EasterRule, JuneteenthRule>>;
    using HolidayRuleList = std::vector<HolidayRule>;

    // with gcc 12, the below will become 'constexpr'

    static const HolidayRule NewYears = std::make_pair("New Years", NewYearsDayRule{});
    static const HolidayRule MLKDay = std::make_pair(
        "Martin Luther King Day", mwd{std::chrono::January, std::chrono::weekday_indexed{std::chrono::Monday, 3}});
    static const HolidayRule WashingtonBday = std::make_pair(
        "Presidents Day", mwd{std::chrono::February, std::chrono::weekday_indexed{std::chrono::Monday, 3}});
    static const HolidayRule GoodFriday = std::make_pair("Good Frday", EasterRule{});
    static const HolidayRule MemorialDay =
        std::make_pair("Memorial Day", mwdl{std::chrono::May, std::chrono::Monday[std::chrono::last]});
    static const HolidayRule Juneteenth = std::make_pair("Juneteenth", JuneteenthRule{});
    static const HolidayRule IndependenceDay = std::make_pair("Independence Day", md{std::chrono::July, 4d});
    static const HolidayRule LaborDay =
        std::make_pair("Labor Day", mwd{std::chrono::September, std::chrono::weekday_indexed{std::chrono::Monday, 1}});
    static const HolidayRule Thanksgiving = std::make_pair(
        "Thanksgiving Day", mwd{std::chrono::November, std::chrono::weekday_indexed{std::chrono::Thursday, 4}});
    static const HolidayRule Christmas = std::make_pair("Christmas Day", md{std::chrono::December, 25d});

    static const HolidayRuleList holiday_rules = {NewYears,     MLKDay,     WashingtonBday,  GoodFriday,
                                                  MemorialDay,  Juneteenth, IndependenceDay, LaborDay,
                                                  Thanksgiving, Christmas};

    //    static const std::vector<ymd> GoodFridays = {{2022_y, std::chrono::April, 15_d}, {2023_y, std::chrono::April,
    //    7_d}, {2024_y, std::chrono::March, 29_d},
    //        {2025_y, std::chrono::April, 18_d}, {2026_y, std::chrono::April, 3_d}, {2027_y, std::chrono::March, 26_d},
    //        {2028_y, std::chrono::April, 14_d}, {2029_y, std::chrono::March, 30_d}, {2030_y, std::chrono::April,
    //        19_d}};

    US_MarketHolidays h_days;

    // NOTE: for Good Friday, first calculate Easter then back up 2 days.

    for (const auto& x : holiday_rules)
    {
        const auto& h_name = x.first;
        const auto& h_rule = x.second;

        std::visit(overloaded{[which_year, &h_days, &h_name](const md& h_rule)
                              {
                                  // these holidays can be any day of the week so adjust observed day
                                  // for Saturdays and Sundays

                                  std::chrono::sys_days hday = ymd{which_year, h_rule.month(), h_rule.day()};
                                  ymwd hwday{hday};
                                  const std::chrono::weekday which_day = hwday.weekday();
                                  if (which_day == std::chrono::Sunday)
                                  {
                                      hday += std::chrono::days{1};
                                  }
                                  else if (which_day == std::chrono::Saturday)
                                  {
                                      hday -= std::chrono::days{1};
                                  }
                                  h_days.emplace_back(US_MarketHoliday{h_name, ymd{hday}});
                              },
                              [which_year, &h_days, &h_name](const mwd& h_rule)
                              {
                                  ymwd x = {which_year, h_rule.month(), h_rule.weekday_indexed()};
                                  h_days.emplace_back(US_MarketHoliday{h_name, ymd{x}});
                              },

                              [which_year, &h_days, &h_name](const mwdl& h_rule)
                              {
                                  ymwdl x = {which_year, h_rule.month(), h_rule.weekday_last()};
                                  h_days.emplace_back(US_MarketHoliday{h_name, ymd{x}});
                              },

                              [which_year, &h_days, &h_name](const NewYearsDayRule& h_rule)
                              {
                                  // If New Years falls on a Saturday, then no holiday observed.
                                  // If on a Sunday, then Monday
                                  std::chrono::sys_days newyears =
                                      ymd{which_year, std::chrono::month{1}, std::chrono::day{1}};
                                  ymwd newyearsday{newyears};
                                  const std::chrono::weekday which_day = newyearsday.weekday();
                                  if (which_day == std::chrono::Sunday)
                                  {
                                      newyears += std::chrono::days{1};
                                  }
                                  if (which_day != std::chrono::Saturday)
                                  {
                                      h_days.emplace_back(US_MarketHoliday{h_name, ymd{newyears}});
                                  }
                              },
                              [which_year, &h_days, &h_name](const EasterRule& h_rule)
                              {
                                  int month = 0;
                                  int day = 0;
                                  easter(GREGORIAN, which_year.operator int(), &month, &day);
                                  ymd easter_sunday{which_year, std::chrono::month(month), std::chrono::day(day)};
                                  std::chrono::sys_days good_friday =
                                      std::chrono::sys_days{easter_sunday} - std::chrono::days{2};
                                  h_days.emplace_back(US_MarketHoliday{h_name, ymd{good_friday}});
                              },
                              [which_year, &h_days, &h_name](const JuneteenthRule& h_rule)
                              {
                                  // first use of this holiday is 2022
                                  if (which_year >= 2022y)
                                  {
                                      std::chrono::sys_days hday =
                                          ymd{which_year, std::chrono::month(std::chrono::June), std::chrono::day(19)};
                                      ymwd hwday{hday};
                                      const std::chrono::weekday which_day = hwday.weekday();
                                      if (which_day == std::chrono::Sunday)
                                      {
                                          hday += std::chrono::days{1};
                                      }
                                      else if (which_day == std::chrono::Saturday)
                                      {
                                          hday -= std::chrono::days{1};
                                      }
                                      h_days.emplace_back(US_MarketHoliday{h_name, ymd{hday}});
                                  }
                              }},
                   h_rule);
    }

    // last thing, add in Good Friday.
    return h_days;
}  // -----  end of function MakeHolidayList  -----

// ===  FUNCTION  ======================================================================
//         Name:  ConvertJSONPriceHistory
//         Description:  Expects the input data is in descending order by date
// =====================================================================================

std::vector<StockDataRecord> ConvertJSONPriceHistory(const std::string& symbol, const Json::Value& the_data,
                                                     uint32_t how_many_days, UseAdjusted use_adjusted)
{
    if (!the_data.isArray() || the_data.empty())
    {
        return {};
    }
    std::vector<StockDataRecord> history;

    auto how_many = std::min(how_many_days, the_data.size());

    if (use_adjusted == UseAdjusted::e_Yes)
    {
        for (int32_t i = 0; i < how_many; ++i)
        {
            StockDataRecord record;
            record.date_ = the_data[i]["date"].asCString();
            record.symbol_ = symbol;
            record.open_ = decimal::Decimal{the_data[i]["adjOpen"].asCString()};
            record.high_ = decimal::Decimal{the_data[i]["adjHigh"].asCString()};
            record.low_ = decimal::Decimal{the_data[i]["adjLow"].asCString()};
            record.close_ = decimal::Decimal{the_data[i]["adjClose"].asCString()};
            history.push_back(std::move(record));
        }
    }
    else
    {
        for (int32_t i = 0; i < how_many; ++i)
        {
            StockDataRecord record;
            record.date_ = the_data[i]["date"].asCString();
            record.symbol_ = symbol;
            record.open_ = decimal::Decimal{the_data[i]["open"].asCString()};
            record.high_ = decimal::Decimal{the_data[i]["high"].asCString()};
            record.low_ = decimal::Decimal{the_data[i]["low"].asCString()};
            record.close_ = decimal::Decimal{the_data[i]["close"].asCString()};
            history.push_back(std::move(record));
        }
    }
    return history;
}  // -----  end of function ConvertJSONPriceHistory  -----

namespace boost
{
// these functions are declared in the library headers but left to the user to define.
// so here they are...
//
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  assertion_failed_mgs
 *  Description:
 *         defined in boost header but left to us to implement.
 * =====================================================================================
 */

void assertion_failed_msg(char const* expr, char const* msg, char const* function, char const* file, int64_t line)
{
    throw std::invalid_argument(
        std::format("\n*** Assertion failed *** test: {} in function: {} from file: {} at line: {} \nassertion msg: {}",
                    expr, function, file, line, msg));
} /* -----  end of function assertion_failed_mgs  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  assertion_failed
 *  Description:
 * =====================================================================================
 */
void assertion_failed(char const* expr, char const* function, char const* file, int64_t line)
{
    throw std::invalid_argument(std::format(
        "\n*** Assertion failed *** test: {} in function: {} from file: {} at line: {}", expr, function, file, line));
} /* -----  end of function assertion_failed  ----- */
} /* end namespace boost */
