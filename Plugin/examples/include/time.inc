/* Time specific functions
*
* by the AMX Mod X Development Team
*
* This file is provided as is (no warranties).
*/

#if defined _time_included
  #endinput
#endif
#define _time_included

/* Time unit types for get_time_length() */
enum 
{
    timeunit_seconds = 0,
    timeunit_minutes,
    timeunit_hours,
    timeunit_days,
    timeunit_weeks,
};

// seconds are in each time unit
#define SECONDS_IN_MINUTE 60
#define SECONDS_IN_HOUR   3600
#define SECONDS_IN_DAY    86400
#define SECONDS_IN_WEEK   604800

/* Stock by Brad */
stock get_time_length(id, unitCnt, type, output[], outputLen)
{
// IMPORTANT: 	You must add register_dictionary("time.txt") in plugin_init()

// id:          The player whose language the length should be translated to (or 0 for server language).
// unitCnt:     The number of time units you want translated into verbose text.
// type:        The type of unit (i.e. seconds, minutes, hours, days, weeks) that you are passing in.
// output:      The variable you want the verbose text to be placed in.
// outputLen:	The length of the output variable.

    if (unitCnt > 0)
    {
        // determine the number of each time unit there are
        new weekCnt = 0, dayCnt = 0, hourCnt = 0, minuteCnt = 0, secondCnt = 0;

        switch (type)
        {
            case timeunit_seconds: secondCnt = unitCnt;
            case timeunit_minutes: secondCnt = unitCnt * SECONDS_IN_MINUTE;
            case timeunit_hours:   secondCnt = unitCnt * SECONDS_IN_HOUR;
            case timeunit_days:    secondCnt = unitCnt * SECONDS_IN_DAY;
            case timeunit_weeks:   secondCnt = unitCnt * SECONDS_IN_WEEK;
        }

        weekCnt = secondCnt / SECONDS_IN_WEEK;
        secondCnt -= (weekCnt * SECONDS_IN_WEEK);

        dayCnt = secondCnt / SECONDS_IN_DAY;
        secondCnt -= (dayCnt * SECONDS_IN_DAY);

        hourCnt = secondCnt / SECONDS_IN_HOUR;
        secondCnt -= (hourCnt * SECONDS_IN_HOUR);

        minuteCnt = secondCnt / SECONDS_IN_MINUTE;
        secondCnt -= (minuteCnt * SECONDS_IN_MINUTE);

        // translate the unit counts into verbose text
        new maxElementIdx = -1;
        new timeElement[5][33];

        if (weekCnt > 0)
            formatex(timeElement[++maxElementIdx], 32, "%i %s", weekCnt, (weekCnt == 1) ? _T("week", id) : _T("weeks", id));
        if (dayCnt > 0)
            formatex(timeElement[++maxElementIdx], 32, "%i %s", dayCnt, (dayCnt == 1) ? _T("day", id) : _T("days", id));
        if (hourCnt > 0)
            formatex(timeElement[++maxElementIdx], 32, "%i %s", hourCnt, (hourCnt == 1) ? _T("hour", id) : _T("hours", id));
        if (minuteCnt > 0)
            formatex(timeElement[++maxElementIdx], 32, "%i %s", minuteCnt, (minuteCnt == 1) ? _T("minute", id) : _T("minutes", id));
        if (secondCnt > 0)
            formatex(timeElement[++maxElementIdx], 32, "%i %s", secondCnt, (secondCnt == 1) ? _T("second", id) : _T("seconds", id));

        switch(maxElementIdx)
        {
            case 0: formatex(output, outputLen, "%s", timeElement[0]);
            case 1: formatex(output, outputLen, "%s %s %s", timeElement[0], _T("and", id), timeElement[1]);
            case 2: formatex(output, outputLen, "%s, %s %s %s", timeElement[0], timeElement[1], _T("and", id), timeElement[2]);
            case 3: formatex(output, outputLen, "%s, %s, %s %s %s", timeElement[0], timeElement[1], timeElement[2], _T("and", id), timeElement[3]);
            case 4: formatex(output, outputLen, "%s, %s, %s, %s %s %s", timeElement[0], timeElement[1], timeElement[2], timeElement[3], _T("and", id), timeElement[4]);
        }
    }
}
