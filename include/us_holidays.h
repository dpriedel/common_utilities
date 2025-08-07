/* =====================================================================================
 *
 * Filename:  us_holidays.h
 *
 * Description:  Code for constructing various US holiday dates.
 *
 * Version:  1.0
 * Created:  2025-08-07 12:15:02
 * Revision:  none
 * Compiler:  gcc / g++
 *
 * Author:  David P. Riedel <driedel@cox.net>
 * Copyright (c) 2025, David P. Riedel
 *
 * =====================================================================================
 */

#ifndef US_HOLIDAYS_H_
#define US_HOLIDAYS_H_

#include <chrono>
#include <optional>
#include <string>
#include <variant>
#include <vector>

using namespace std::chrono_literals;
using namespace std::literals;

// Type definition for a single US Market Holiday
using US_MarketHoliday = std::pair<const std::string, const std::chrono::year_month_day>;
// Type definition for a list of US Market Holidays
using US_MarketHolidays = std::vector<US_MarketHoliday>;

// --- Individual Holiday Rule Structs ---

struct NewYearsHoliday
{
    static constexpr char holiday_name_[] = "New Years";
    static constexpr std::chrono::month_day holiday_rule_ = std::chrono::January / 1d;
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct MLKDayHoliday
{
    static constexpr char holiday_name_[] = "Martin Luther King Day";
    static constexpr std::chrono::month_weekday holiday_rule_ = std::chrono::January / std::chrono::Monday[3];
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct WashingtonBdayHoliday
{
    static constexpr char holiday_name_[] = "Presidents Day";
    static constexpr std::chrono::month_weekday holiday_rule_ = std::chrono::February / std::chrono::Monday[3];
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct GoodFridayHoliday
{
    static constexpr char holiday_name_[] = "Good Friday";
    static constexpr std::chrono::month_day holiday_rule_ = {};
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct MemorialDayHoliday
{
    static constexpr char holiday_name_[] = "Memorial Day";
    static constexpr std::chrono::month_weekday_last holiday_rule_ =
        std::chrono::May / std::chrono::Monday[std::chrono::last];
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct JuneteenthHoliday
{
    static constexpr char holiday_name_[] = "Juneteenth";
    static constexpr std::chrono::month_day holiday_rule_ = std::chrono::June / 19d;
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct IndependenceDayHoliday
{
    static constexpr char holiday_name_[] = "Independence Day";
    static constexpr std::chrono::month_day holiday_rule_ = std::chrono::July / 4d;
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct LaborDayHoliday
{
    static constexpr char holiday_name_[] = "Labor Day";
    static constexpr std::chrono::month_weekday holiday_rule_ = std::chrono::September / std::chrono::Monday[1];
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct ThanksgivingHoliday
{
    static constexpr char holiday_name_[] = "Thanksgiving Day";
    static constexpr std::chrono::month_weekday holiday_rule_ = std::chrono::November / std::chrono::Thursday[4];
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct ChristmasHoliday
{
    static constexpr char holiday_name_[] = "Christmas Day";
    static constexpr std::chrono::month_day holiday_rule_ = std::chrono::December / 25d;
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct CartersDayHoliday
{
    static constexpr char holiday_name_[] = "Carter Memorial";
    static constexpr std::chrono::year_month_day holiday_rule_ = 2025y / std::chrono::January / 9d;
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

// Type alias for the variant of all holiday rule structs
using HolidayRuleVariant = std::variant<NewYearsHoliday, MLKDayHoliday, WashingtonBdayHoliday, GoodFridayHoliday,
                                        MemorialDayHoliday, JuneteenthHoliday, IndependenceDayHoliday, LaborDayHoliday,
                                        ThanksgivingHoliday, ChristmasHoliday, CartersDayHoliday>;

// Function to generate a list of US market holidays for the given year
US_MarketHolidays MakeHolidayList(std::chrono::year which_year);

#endif /* US_HOLIDAYS_H_ */
