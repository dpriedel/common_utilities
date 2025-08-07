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

#include <algorithm>
#include <chrono> // Redundant in C++20 if using <chrono> in .h, but good practice for .cpp files.
#include <format>

using namespace std::chrono_literals;

#include "calfaq.h" // For the easter() function
#include "us_holidays.h"

// --- Individual Holiday Rule Structs Implementations ---

NewYearsHoliday::NewYearsHoliday(std::string name, NewYearsDayRuleTag rule_tag)
    : holiday_name_(std::move(name)), holiday_rule_(rule_tag)
{
}

std::optional<US_MarketHoliday> NewYearsHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::sys_days newyears = which_year / std::chrono::month(1) / std::chrono::day(1);
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

MLKDayHoliday::MLKDayHoliday(std::string name, std::chrono::month_weekday rule)
    : holiday_name_(std::move(name)), holiday_rule_(rule)
{
}

std::optional<US_MarketHoliday> MLKDayHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::year_month_weekday x = {which_year, holiday_rule_.month(), holiday_rule_.weekday_indexed()};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{x}};
}

WashingtonBdayHoliday::WashingtonBdayHoliday(std::string name, std::chrono::month_weekday rule)
    : holiday_name_(std::move(name)), holiday_rule_(rule)
{
}

std::optional<US_MarketHoliday> WashingtonBdayHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::year_month_weekday x = {which_year, holiday_rule_.month(), holiday_rule_.weekday_indexed()};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{x}};
}

GoodFridayHoliday::GoodFridayHoliday(std::string name, EasterRuleTag rule_tag)
    : holiday_name_(std::move(name)), holiday_rule_(rule_tag)
{
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

MemorialDayHoliday::MemorialDayHoliday(std::string name, std::chrono::month_weekday_last rule)
    : holiday_name_(std::move(name)), holiday_rule_(rule)
{
}

std::optional<US_MarketHoliday> MemorialDayHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::year_month_weekday_last x = {which_year, holiday_rule_.month(), holiday_rule_.weekday_last()};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{x}};
}

JuneteenthHoliday::JuneteenthHoliday(std::string name, JuneteenthRuleTag rule_tag)
    : holiday_name_(std::move(name)), holiday_rule_(rule_tag)
{
}

std::optional<US_MarketHoliday> JuneteenthHoliday::operator()(std::chrono::year which_year) const
{
    if (which_year >= 2022y)
    { // first use of this holiday is 2022
        std::chrono::sys_days hday = which_year / std::chrono::month(std::chrono::June) / std::chrono::day(19);
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

IndependenceDayHoliday::IndependenceDayHoliday(std::string name, std::chrono::month_day rule)
    : holiday_name_(std::move(name)), holiday_rule_(rule)
{
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

LaborDayHoliday::LaborDayHoliday(std::string name, std::chrono::month_weekday rule)
    : holiday_name_(std::move(name)), holiday_rule_(rule)
{
}

std::optional<US_MarketHoliday> LaborDayHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::year_month_weekday x = {which_year, holiday_rule_.month(), holiday_rule_.weekday_indexed()};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{x}};
}

ThanksgivingHoliday::ThanksgivingHoliday(std::string name, std::chrono::month_weekday rule)
    : holiday_name_(std::move(name)), holiday_rule_(rule)
{
}

std::optional<US_MarketHoliday> ThanksgivingHoliday::operator()(std::chrono::year which_year) const
{
    std::chrono::year_month_weekday x = {which_year, holiday_rule_.month(), holiday_rule_.weekday_indexed()};
    return US_MarketHoliday{holiday_name_, std::chrono::year_month_day{x}};
}

ChristmasHoliday::ChristmasHoliday(std::string name, std::chrono::month_day rule)
    : holiday_name_(std::move(name)), holiday_rule_(rule)
{
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

CartersDayHoliday::CartersDayHoliday(std::string name, std::chrono::year_month_day rule)
    : holiday_name_(std::move(name)), holiday_rule_(rule)
{
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
        NewYearsHoliday{"New Years", NewYearsDayRuleTag{}},
        MLKDayHoliday{"Martin Luther King Day", std::chrono::January / std::chrono::Monday[3]},
        WashingtonBdayHoliday{"Presidents Day", std::chrono::February / std::chrono::Monday[3]},
        GoodFridayHoliday{"Good Friday", EasterRuleTag{}},
        MemorialDayHoliday{"Memorial Day", std::chrono::May / std::chrono::Monday[std::chrono::last]},
        JuneteenthHoliday{"Juneteenth", JuneteenthRuleTag{}},
        IndependenceDayHoliday{"Independence Day", std::chrono::July / 4d},
        LaborDayHoliday{"Labor Day", std::chrono::September / std::chrono::Monday[1]},
        ThanksgivingHoliday{"Thanksgiving Day", std::chrono::November / std::chrono::Thursday[4]},
        ChristmasHoliday{"Christmas Day", std::chrono::December / 25d},
        CartersDayHoliday{"Carter Memorial", 2025y / std::chrono::January / 9d}};

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
