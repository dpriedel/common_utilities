/* =====================================================================================
 *
 * Filename:  us_holidays.cpp
 *
 * Description:  Implementation file for computing various US holiday dates.
 *
 * Version:  1.0
 * Created:  2025-08-07 12:15:55
 * Revision:  none
 * Compiler:  gcc / g++
 *
 * Author:  David P. Riedel <driedel@cox.net>
 * Copyright (c) 2025, David P. Riedel
 *
 * =====================================================================================
 */

extern "C"
{
#include "calfaq.h"
}

#include "us_holidays.h"

// --- Individual Holiday Rule Structs Implementations ---

std::optional<US_MarketHoliday> NewYearsHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::sys_days newyears = which_year / holiday_rule_.month() / holiday_rule_.day();
    std::chrono::year_month_weekday newyearsday{newyears};
    const std::chrono::weekday which_day = newyearsday.weekday();
    if (which_day == std::chrono::Sunday)
    {
        newyears += std::chrono::days{1};
    }
    if (which_day != std::chrono::Saturday)
    { // If Saturday, no observed holiday.
        return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{newyears}};
    }
    return std::nullopt;
}

std::optional<US_MarketHoliday> MLKDayHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::year_month_weekday mlk = {which_year, holiday_rule_.month(), holiday_rule_.weekday_indexed()};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{mlk}};
}

std::optional<US_MarketHoliday> WashingtonBdayHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::year_month_weekday wbd = {which_year, holiday_rule_.month(), holiday_rule_.weekday_indexed()};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{wbd}};
}

std::optional<US_MarketHoliday> GoodFridayHoliday::operator()(std::chrono::year which_year) const
{
    int month = 0;
    int day = 0;
    easter(GREGORIAN, which_year.operator int(), &month, &day);
    std::chrono::year_month_day easter_sunday{which_year, std::chrono::month(month), std::chrono::day(day)};
    std::chrono::sys_days good_friday_sys_days = std::chrono::sys_days{easter_sunday} - std::chrono::days{2};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{good_friday_sys_days}};
}

std::optional<US_MarketHoliday> MemorialDayHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::year_month_weekday_last md = {which_year, holiday_rule_.month(), holiday_rule_.weekday_last()};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{md}};
}

std::optional<US_MarketHoliday> JuneteenthHoliday::operator()(std::chrono::year which_year) const
{
    if (which_year >= 2022y)
    { // first use of this holiday is 2022
        std::chrono::sys_days hday = which_year / holiday_rule_.month() / holiday_rule_.day();
        std::chrono::year_month_weekday hwday{hday};
        const std::chrono::weekday which_day = hwday.weekday();
        if (which_day == std::chrono::Sunday)
        {
            hday += std::chrono::days{1};
        }
        else if (which_day == std::chrono::Saturday)
        {
            hday -= std::chrono::days{1};
        }
        return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{hday}};
    }
    return std::nullopt;
}

std::optional<US_MarketHoliday> IndependenceDayHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::sys_days hday = which_year / holiday_rule_.month() / holiday_rule_.day();
    std::chrono::year_month_weekday hwday{hday};
    const std::chrono::weekday which_day = hwday.weekday();
    if (which_day == std::chrono::Sunday)
    {
        hday += std::chrono::days{1};
    }
    else if (which_day == std::chrono::Saturday)
    {
        hday -= std::chrono::days{1};
    }
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{hday}};
}

std::optional<US_MarketHoliday> LaborDayHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::year_month_weekday ld = {which_year, holiday_rule_.month(), holiday_rule_.weekday_indexed()};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{ld}};
}

std::optional<US_MarketHoliday> ThanksgivingHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::year_month_weekday tg = {which_year, holiday_rule_.month(), holiday_rule_.weekday_indexed()};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{tg}};
}

std::optional<US_MarketHoliday> ChristmasHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::sys_days hday = which_year / holiday_rule_.month() / holiday_rule_.day();
    std::chrono::year_month_weekday hwday{hday};
    const std::chrono::weekday which_day = hwday.weekday();
    if (which_day == std::chrono::Sunday)
    {
        hday += std::chrono::days{1};
    }
    else if (which_day == std::chrono::Saturday)
    {
        hday -= std::chrono::days{1};
    }
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{hday}};
}

std::optional<US_MarketHoliday> CartersDayHoliday::operator()(std::chrono::year which_year) const
{
    if (which_year == holiday_rule_.year())
    {
        return US_MarketHoliday{holiday_name_, holiday_rule_};
    }
    return std::nullopt;
}

// ===  FUNCTION  ======================================================================
//         Name:  MakeHolidayList
//  Description:
// =====================================================================================
US_MarketHolidays MakeHolidayList(std::chrono::year which_year)
{
    // Static list of holiday rule instances
    static const std::vector<HolidayRuleVariant> holiday_rules = {
        NewYearsHoliday{},     MLKDayHoliday{},     WashingtonBdayHoliday{},  GoodFridayHoliday{},
        MemorialDayHoliday{},  JuneteenthHoliday{}, IndependenceDayHoliday{}, LaborDayHoliday{},
        ThanksgivingHoliday{}, ChristmasHoliday{},  CartersDayHoliday{}};

    US_MarketHolidays h_days;

    for (const auto &x : holiday_rules)
    {
        std::visit(
            [&](const auto &rule_struct) {
                if (auto holiday = rule_struct(which_year); holiday)
                {
                    h_days.emplace_back(*holiday);
                }
            },
            x);
    }

    return h_days;
}
