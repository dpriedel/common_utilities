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

#include <stdexcept>

#include <date/tz.h>
#include <string_view>

#include "utilities.h"


// ===  FUNCTION  ======================================================================
//         Name:  LocalDateTimeAsString
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
