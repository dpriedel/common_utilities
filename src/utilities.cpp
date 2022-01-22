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


#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <variant>
#include <vector>

#include <date/date.h>
#include <date/tz.h>

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/find.hpp>

//#include <range/v3/range/conversion.hpp>
//#include <range/v3/view/transform.hpp>

#include "utilities.h"
extern "C"
{
    #include "calfaq.h"
}

using namespace date::literals;


// ===  FUNCTION  ======================================================================
//         Name:  TimePointToYMDString
//  Description:  
// =====================================================================================
std::string TimePointToYMDString (std::chrono::system_clock::time_point a_time_point)
{
    auto x1 = date::year_month_day{floor<std::chrono::days>(a_time_point)};
    std::string result = fmt::format("{}", x1);
    return result;
}		// -----  end of function TimePointToYMDString  -----

// ===  FUNCTION  ======================================================================
//         Name:  TimePointToHMSString
//  Description:  
// =====================================================================================

std::string TimePointToHMSString(std::chrono::system_clock::time_point a_time_point)
{
    auto t = date::make_zoned(date::current_zone(), a_time_point);
    std::string result = date::format("%I:%M:%S", t);
    return result;
}		// -----  end of function TimePointToHMSString  -----

// ===  FUNCTION  ======================================================================
//         Name:  StringToTimePoint
//  Description:  
// =====================================================================================

std::chrono::system_clock::time_point StringToTimePoint(std::string_view input_format, std::string_view the_date)
{
    std::istringstream in{the_date.data()};
    std::chrono::system_clock::time_point tp;
    date::from_stream(in, input_format.data(), tp);
    BOOST_ASSERT_MSG(! in.fail() && ! in.bad(), fmt::format("Unable to parse given date: {}", the_date).c_str());
    return tp;
}		// -----  end of method StringToDateYMD  ----- 

// ===  FUNCTION  ======================================================================
//         Name:  StringToDateYMD
//  Description:  
// =====================================================================================

date::year_month_day StringToDateYMD(std::string_view input_format, std::string_view the_date)
{
    auto timept = StringToTimePoint(input_format, the_date);
    date::year_month_day result = floor<date::days>(timept);
    BOOST_ASSERT_MSG(result.ok(), fmt::format("Invalid date: {}", the_date).c_str());
    return result;
}		// -----  end of method StringToDateYMD  ----- 

// ===  FUNCTION  ======================================================================
//         Name:  LocalDateTimeAsString
//  Description:  
// =====================================================================================

std::string LocalDateTimeAsString(std::chrono::system_clock::time_point a_date_time)
{
    auto t = date::make_zoned(date::current_zone(), a_date_time);
    std::string ts = date::format("%a, %b %d, %Y at %I:%M:%S %p %Z", t);
    return ts;
}		// -----  end of function LocalDateTimeAsString  -----

// ===  FUNCTION  ======================================================================
//         Name:  DateTimeAsString
//  Description:  
// =====================================================================================

std::string DateTimeAsString(std::chrono::system_clock::time_point a_date_time)
{
    std::string ts = date::format("%m-%d-%Y : %H:%M:%S", a_date_time);
    return ts;
}		// -----  end of function DateTimeAsString  -----

// ===  FUNCTION  ======================================================================
//         Name:  ConstructeBusinessDayList
//  Description:  Generate a start/end pair of dates which included n business days 
//                skipping holidays.
// =====================================================================================

std::vector<date::year_month_day> ConstructeBusinessDayList(date::year_month_day start_from, int how_many_business_days, UpOrDown order, const US_MarketHolidays* holidays)
{
    // we need to do some date arithmetic so we can use our basic 'GetTickerData' method. 

    auto days = date::sys_days(start_from);

    const std::chrono::days day_increment{order == UpOrDown::e_Up ? 1 : -1}; 

    std::vector<date::year_month_day> business_days;

    auto IsHoliday = [holidays](const date::year_month_day& a_day)
        {
            if (holidays == nullptr) return false;
            return ranges::find(*holidays, a_day, [](const auto& e) { return e.second; }) != holidays->end();
        };

    while (business_days.size() < how_many_business_days)
    {
        auto b_day = date::weekday{days};
        while (b_day == date::Saturday || b_day == date::Sunday || IsHoliday(days))
        {
            days += day_increment;
            b_day = date::weekday{days};
        }
        business_days.push_back(days);
        days += day_increment;
    }

//    ranges::for_each(business_days, [](const auto& e) { std::cout << e << '\n'; });
//    std::cout << "how many: " << business_days.size() << '\n';
    return business_days;
}		// -----  end of function ConstructeBusinessDayList  -----

// ===  FUNCTION  ======================================================================
//         Name:  ConstructeBusinessDayRange
//  Description:  Generate a start/end pair of dates which included n business days 
//                skipping holidays.
// =====================================================================================

std::pair<date::year_month_day, date::year_month_day> ConstructeBusinessDayRange(date::year_month_day start_from, int how_many_business_days, UpOrDown order, const US_MarketHolidays* holidays)
{
    // we need to do some date arithmetic so we can use our basic 'GetTickerData' method. 

    auto days = date::sys_days(start_from);

    const std::chrono::days day_increment{order == UpOrDown::e_Up ? 1 : -1}; 

    auto business_days = ConstructeBusinessDayList(start_from, how_many_business_days, order, holidays);

    return {business_days.front(), business_days.back()};
}		// -----  end of function ConstructeBusinessDayRange  -----

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  LoadDataFileForUse
 *  Description:  
 * =====================================================================================
 */
std::string LoadDataFileForUse (const fs::path& file_name)
{
    std::string file_content;     // make room for trailing null
    file_content.reserve(fs::file_size(file_name) + 1);
    std::ifstream input_file{file_name, std::ios_base::in | std::ios_base::binary};
    BOOST_ASSERT_MSG(input_file.is_open(), fmt::format("Can't open data file: {}.", file_name).c_str());
//    input_file.read(&file_content[0], file_content.size());
	file_content.assign(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>());
    input_file.close();
    
    return file_content;
}		/* -----  end of function LoadDataFileForUse  ----- */

// helper type for the visitor function used below (from cppreference)
//
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };


// ===  FUNCTION  ======================================================================
//         Name:  MakeHolidayList
//  Description:  
// =====================================================================================
US_MarketHolidays MakeHolidayList (date::year which_year)
{
    // save us some typing,,,

    using namespace std::literals::chrono_literals;
    using namespace date::literals;

    using md = date::month_day;
    using mwd = date::month_weekday;
    using mwdl = date::month_weekday_last;

    using ymd = date::year_month_day;
    using ymwd = date::year_month_weekday;
    using ymwdl = date::year_month_weekday_last;

    struct NewYearsDayRule {};     // New Years day does not fall back to previous year.
    struct EasterRule {};       // use this to trigger computation needed to find Good Friday.
    struct JuneteenthRule {};       // use this to trigger computation needed to find Good Friday.

    // here are the rules for constructing each holiday. 

    using HolidayRule = std::pair<const std::string, std::variant<const md, const mwd, const mwdl, NewYearsDayRule, EasterRule, JuneteenthRule>>;
    using HolidayRuleList = std::vector<HolidayRule>;

    // with gcc 12, the below will become 'constexpr'

    static const HolidayRule NewYears = std::make_pair("New Years", NewYearsDayRule{});
    static const HolidayRule MLKDay = std::make_pair("Martin Luther King Day", mwd{date::January, date::weekday_indexed{date::Monday, 3}});
    static const HolidayRule WashingtonBday = std::make_pair("Presidents Day", mwd{date::February, date::weekday_indexed{date::Monday, 3}});
    static const HolidayRule GoodFriday = std::make_pair("Good Frday", EasterRule{});
    static const HolidayRule MemorialDay = std::make_pair("Memorial Day", mwdl{date::May, date::Monday[last]});
    static const HolidayRule Juneteenth = std::make_pair("Juneteenth", JuneteenthRule{});
    static const HolidayRule IndependenceDay = std::make_pair("Independence Day", md{date::July, 4_d});
    static const HolidayRule LaborDay = std::make_pair("Labor Day", mwd{date::September, date::weekday_indexed{date::Monday, 1}});
    static const HolidayRule Thanksgiving = std::make_pair("Thanksgiving Day", mwd{date::November, date::weekday_indexed{date::Thursday, 4}});
    static const HolidayRule Christmas = std::make_pair("Christmas Day", md{date::December, 25_d});

    static const HolidayRuleList holiday_rules = {NewYears, MLKDay, WashingtonBday, GoodFriday, MemorialDay, Juneteenth,
        IndependenceDay, LaborDay, Thanksgiving, Christmas};

//    static const std::vector<ymd> GoodFridays = {{2022_y, date::April, 15_d}, {2023_y, date::April, 7_d}, {2024_y, date::March, 29_d},
//        {2025_y, date::April, 18_d}, {2026_y, date::April, 3_d}, {2027_y, date::March, 26_d}, {2028_y, date::April, 14_d},
//        {2029_y, date::March, 30_d}, {2030_y, date::April, 19_d}};

    US_MarketHolidays h_days;

    // NOTE: for Good Friday, first calculate Easter then back up 2 days.

    for(const auto& x : holiday_rules)
    {
        const auto& h_name = x.first;
        const auto& h_rule = x.second;

            std::visit(overloaded 
            {
                [which_year, &h_days, &h_name](const md& h_rule)
                    {
                        // these holidays can be any day of the week so adjust observed day 
                        // for Saturdays and Sundays 

                        date::sys_days hday = ymd{which_year, h_rule.month(), h_rule.day()};
                        ymwd hwday{hday};
                        const date::weekday which_day = hwday.weekday();
                        if (which_day == date::Sunday)
                        {
                            hday += date::days{1};
                        }
                        else if (which_day == date::Saturday)
                        {
                            hday -= date::days{1};
                        }
                        h_days.emplace_back(US_MarketHoliday{h_name, ymd{hday}});
                    },
                [which_year, &h_days, &h_name](const mwd& h_rule){ ymwd x = {which_year, h_rule.month(), h_rule.weekday_indexed()}; h_days.emplace_back(US_MarketHoliday{h_name, ymd{x}}); },

                [which_year, &h_days, &h_name](const mwdl& h_rule){ ymwdl x = {which_year, h_rule.month(), h_rule.weekday_last()}; h_days.emplace_back(US_MarketHoliday{h_name, ymd{x}}); },
                
                [which_year, &h_days, &h_name](const NewYearsDayRule& h_rule)
                    { 
                        // If New Years falls on a Saturday, then no holiday observed.
                        // If on a Sunday, then Monday
                        date::sys_days newyears = ymd{which_year, date::month{1}, date::day{1}};
                        ymwd newyearsday{newyears};
                        const date::weekday which_day = newyearsday.weekday();
                        if (which_day == date::Sunday)
                        {
                            newyears += date::days{1};
                        }
                        if (which_day != date::Saturday)
                        {
                            h_days.emplace_back(US_MarketHoliday{h_name, ymd{newyears}});
                        }
                    },
                [which_year, &h_days, &h_name](const EasterRule& h_rule)
                    { 
                        int month, day;
                        easter(GREGORIAN, which_year.operator int(), &month, &day); 
                        ymd easter_sunday{which_year, date::month(month), date::day(day)};
                        date::sys_days good_friday = date::sys_days(easter_sunday) - date::days{2};
                        h_days.emplace_back(US_MarketHoliday{h_name, ymd{good_friday}});
                    },
                [which_year, &h_days, &h_name](const JuneteenthRule& h_rule)
                    { 
                        // first use of this holiday is 2022
                        if (which_year >= 2022_y)
                        {
                            date::sys_days hday = ymd{which_year, date::month(6), date::day(19)};
                            ymwd hwday{hday};
                            const date::weekday which_day = hwday.weekday();
                            if (which_day == date::Sunday)
                            {
                                hday += date::days{1};
                            }
                            else if (which_day == date::Saturday)
                            {
                                hday -= date::days{1};
                            }
                            h_days.emplace_back(US_MarketHoliday{h_name, ymd{hday}});
                        }
                    }
            }, h_rule);
    }

    // last thing, add in Good Friday.
    return h_days;
}		// -----  end of function MakeHolidayList  -----

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

    void assertion_failed_msg (char const* expr, char const* msg, char const* function, char const* file, long line)
    {
        throw std::invalid_argument(fmt::format("\n*** Assertion failed *** test: {} in function: {} from file: {} at line: {} \nassertion msg: {}",
                    expr, function, file, line,  msg));
    }		/* -----  end of function assertion_failed_mgs  ----- */

    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  assertion_failed
     *  Description:  
     * =====================================================================================
     */
    void assertion_failed (char const* expr, char const* function, char const* file, long line )
    {
        throw std::invalid_argument(fmt::format("\n*** Assertion failed *** test: {} in function: {} from file: {} at line: {}",
                    expr, function, file, line));
    }		/* -----  end of function assertion_failed  ----- */
} /* end namespace boost */
