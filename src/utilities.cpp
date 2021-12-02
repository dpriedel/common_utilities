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
#include <stdexcept>

#include <date/tz.h>
#include <string_view>

#include "utilities.h"

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
//         Name:  ConstructeBusinessDayRange
//  Description:  Generate a start/end pair of dates which included n business days 
//                ignoring holidays.
// =====================================================================================

std::pair<date::year_month_day, date::year_month_day> ConstructeBusinessDayRange(date::year_month_day start_from, int how_many_business_days, UpOrDown order)
{
    // we need to do some date arithmetic so we can use our basic 'GetTickerData' method. 

    auto days = date::sys_days(start_from);
    date::weekday business_day{days};

    std::chrono::days one_day{order == UpOrDown::e_Up ? 1 : -1}; 
    std::chrono::days two_days{order == UpOrDown::e_Up ? 2 : -2}; 

    // if we start on a weekend, move to the closest business day 
    // based on direction we will move in 

    if (order == UpOrDown::e_Up)
    {
        // move forward to the next business day 

        if (business_day == date::Saturday)
        {
            days += two_days;
            business_day += two_days;
        }
        else if (business_day == date::Sunday)
        {
            days += one_day;
            business_day += one_day;
        }
    }
    else
    {
        // move back to the previous business day 

        if (business_day == date::Saturday)
        {
            days += one_day;
            business_day += one_day;
        }
        else if (business_day == date::Sunday)
        {
            days += two_days;
            business_day += two_days;
        }
    }

    start_from = date::year_month_day{days};

    for (int i = --how_many_business_days; i > 0; --i)
    {
        business_day += one_day;
        days += one_day;

        if (order == UpOrDown::e_Up)
        {
            // move forward to the next business day 

            if (business_day == date::Saturday)
            {
                days += two_days;
                business_day += two_days;
            }
            else if (business_day == date::Sunday)
            {
                days += one_day;
                business_day += one_day;
            }
        }
        else
        {
            // move back to the previous business day 

            if (business_day == date::Saturday)
            {
                days += one_day;
                business_day += one_day;
            }
            else if (business_day == date::Sunday)
            {
                days += two_days;
                business_day += two_days;
            }
        }
    }
    date::year_month_day end_at = date::year_month_day{days};
    return {start_from, end_at};
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
    
    file_content += '\0';
    return file_content;
}		/* -----  end of function LoadDataFileForUse  ----- */

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
