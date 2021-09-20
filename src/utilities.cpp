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

#include "utilities.h"


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
