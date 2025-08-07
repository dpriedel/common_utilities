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

#endif /* US_HOLIDAYS_H_ */
// include/us_holidays.h
#ifndef US_HOLIDAYS_H
#define US_HOLIDAYS_H

#include <chrono>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// Type definition for a single US Market Holiday
using US_MarketHoliday = std::pair<const std::string, const std::chrono::year_month_day>;
// Type definition for a list of US Market Holidays
using US_MarketHolidays = std::vector<US_MarketHoliday>;

// These small tag structs are used as "rules" for specific holiday calculations
// when a simple chrono type like month_day or month_weekday isn't sufficient.
struct NewYearsDayRuleTag
{
};
struct EasterRuleTag
{
};
struct JuneteenthRuleTag
{
};

// --- Individual Holiday Rule Structs ---

struct NewYearsHoliday
{
    std::string holiday_name_;
    NewYearsDayRuleTag holiday_rule_;

    NewYearsHoliday(std::string name, NewYearsDayRuleTag rule_tag);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct MLKDayHoliday
{
    std::string holiday_name_;
    std::chrono::month_weekday holiday_rule_;

    MLKDayHoliday(std::string name, std::chrono::month_weekday rule);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct WashingtonBdayHoliday
{
    std::string holiday_name_;
    std::chrono::month_weekday holiday_rule_;

    WashingtonBdayHoliday(std::string name, std::chrono::month_weekday rule);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct GoodFridayHoliday
{
    std::string holiday_name_;
    EasterRuleTag holiday_rule_;

    GoodFridayHoliday(std::string name, EasterRuleTag rule_tag);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct MemorialDayHoliday
{
    std::string holiday_name_;
    std::chrono::month_weekday_last holiday_rule_;

    MemorialDayHoliday(std::string name, std::chrono::month_weekday_last rule);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct JuneteenthHoliday
{
    std::string holiday_name_;
    JuneteenthRuleTag holiday_rule_;

    JuneteenthHoliday(std::string name, JuneteenthRuleTag rule_tag);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct IndependenceDayHoliday
{
    std::string holiday_name_;
    std::chrono::month_day holiday_rule_;

    IndependenceDayHoliday(std::string name, std::chrono::month_day rule);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct LaborDayHoliday
{
    std::string holiday_name_;
    std::chrono::month_weekday holiday_rule_;

    LaborDayHoliday(std::string name, std::chrono::month_weekday rule);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct ThanksgivingHoliday
{
    std::string holiday_name_;
    std::chrono::month_weekday holiday_rule_;

    ThanksgivingHoliday(std::string name, std::chrono::month_weekday rule);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct ChristmasHoliday
{
    std::string holiday_name_;
    std::chrono::month_day holiday_rule_;

    ChristmasHoliday(std::string name, std::chrono::month_day rule);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

struct CartersDayHoliday
{
    std::string holiday_name_;
    std::chrono::year_month_day holiday_rule_;

    CartersDayHoliday(std::string name, std::chrono::year_month_day rule);
    std::optional<US_MarketHoliday> operator()(std::chrono::year which_year) const;
};

// Type alias for the variant of all holiday rule structs
using HolidayRuleVariant = std::variant<NewYearsHoliday, MLKDayHoliday, WashingtonBdayHoliday, GoodFridayHoliday,
                                        MemorialDayHoliday, JuneteenthHoliday, IndependenceDayHoliday, LaborDayHoliday,
                                        ThanksgivingHoliday, ChristmasHoliday, CartersDayHoliday>;

// Function to generate a list of US market holidays for the given year
US_MarketHolidays MakeHolidayList(std::chrono::year which_year);

#endif // US_HOLIDAYS_H
