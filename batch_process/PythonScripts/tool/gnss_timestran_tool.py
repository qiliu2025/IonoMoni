# =================================================================
# Responsible for GNSS data processing time format conversion and calculation
# Copyright (C) The GNSS Center, Wuhan University & Chinese Academy of Surveying and mapping
# =================================================================
from tool.gnss_print_tool import PrintGDD
import datetime


# 2022-04-30 : datetime to GNSS TIME and output
#              by Chang Chuntao  -> Version : 1.12
def datetime2GnssTime(specTime):
    """
    2022-04-30 : datetime to GNSS TIME and output
                 by Chang Chuntao  -> Version : 1.12
    """
    YearMonthDay = [specTime.year, specTime.month, specTime.day]
    gpsWeekdDelTime = specTime - datetime.datetime(year=1980, month=1, day=6)
    gpsWeek = gpsWeekdDelTime.days // 7
    gpsWeekD = gpsWeekdDelTime.days - gpsWeek * 7
    GPSWeekDay = [gpsWeek, gpsWeekD]

    delTime = specTime - datetime.datetime(year=specTime.year, month=1, day=1)
    doy = delTime.days + 1
    YearDoy = [int(specTime.year), doy]

    mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)  # Simplified starting date of Julian Day
    mjd = (specTime - mjdT0).days
    sod = specTime.hour * 3600.0 + specTime.minute * 60.0 + specTime.second + specTime.microsecond / 1000000.0
    MjdSod = [int(mjd), sod]

    return YearMonthDay, GPSWeekDay, YearDoy, MjdSod


def gnssTime2datetime(gnssTime, gnssTimeType):
    """
    2022-04-30 : Convert GNSS TIME to datetime
             by Chang Chuntao  -> Version : 1.12
    """
    gnssTime = gnssTime.split()
    if gnssTimeType == "YearMonthDay":
        dateTime = datetime.datetime(int(gnssTime[0]), int(gnssTime[1]), int(gnssTime[2]), 0, 0, 0, 0)
    elif gnssTimeType == "YearDoy":
        day1Time = datetime.datetime(int(gnssTime[0]), 1, 1, 0, 0, 0, 0)
        dateTime = day1Time + datetime.timedelta(days=int(gnssTime[1]) - 1)
    elif gnssTimeType == "GPSWeekDay":
        wd1Time = datetime.datetime(year=1980, month=1, day=6)
        dateTime = wd1Time + datetime.timedelta(weeks=int(gnssTime[0])) + datetime.timedelta(days=int(gnssTime[1]))
    elif gnssTimeType == "MjdSod":
        mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)
        dateTime = mjdT0 + datetime.timedelta(days=int(gnssTime[0])) + datetime.timedelta(seconds=float(gnssTime[1]))
    else:
        PrintGDD("This format is not currently supported!", "fail")
        dateTime = datetime.datetime.utcnow()  # Gets the current utc time
        PrintGDD("The current system UTC time is" + str(dateTime)[:-7], "normal")
        datetime2GnssTime(dateTime)

    return dateTime


def printTime(specTime, GPSWeekDay, YearDoy, MjdSod):
    """
    2022-05-25 : Output time
             by Chang Chuntao  -> Version : 1.13
    """
    PrintGDD("Year / Month / Day ".ljust(22) + ": " + str(specTime.year).ljust(5) + " " +
             str(specTime.month).zfill(2) + " " + str(specTime.day).zfill(2), "nothing")
    PrintGDD("Year / Doy ".ljust(22) + ": " + str(YearDoy[0]).ljust(5) + " " + str(YearDoy[1]).zfill(3), "nothing")
    PrintGDD("GPSWeek / DayofWeek ".ljust(22) + ": " + str(GPSWeekDay[0]).ljust(5) + " " + str(GPSWeekDay[1]),
             "nothing")
    PrintGDD("MJD / Sod ".ljust(22) + ": " + str(MjdSod[0]).ljust(5) + " " + str(MjdSod[1]), "nothing")
    
def datetime2gnssTime(specTime, gnssTimeType):
    '''
    2022-04-30 : datetime to GNSS TIME  by Chang Chuntao
    '''
    import datetime
    import sys
    if gnssTimeType == "YMD":
        outTime = [specTime.year, specTime.month, specTime.day]
    elif gnssTimeType == "YDOY":
        delTime = specTime - datetime.datetime(year=specTime.year, month=1, day=1)
        doy = delTime.days + 1
        outTime = [int(specTime.year), doy]
    elif gnssTimeType == "GPSWEEKD":
        gpsWeekdDelTime = specTime - datetime.datetime(year=1980, month=1, day=6)
        gpsWeek = gpsWeekdDelTime.days // 7
        gpsWeekD = gpsWeekdDelTime.days - gpsWeek * 7
        outTime = [gpsWeek, gpsWeekD]
    elif gnssTimeType == "MJDSOD":
        mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)  # Simplified starting date of Julian Day
        mjd = (specTime - mjdT0).days
        sod = specTime.hour * 3600.0 + specTime.minute * 60.0 + specTime.second + specTime.microsecond / 1000000.0
        outTime = [int(mjd), sod]
    elif gnssTimeType == 'YmdHMS':
        outTime = [specTime.year, specTime.month, specTime.day, specTime.hour, specTime.minute,
                   specTime.second + specTime.microsecond / 1000000.0]
    else:
        sys.exit()
    return outTime

class GNSS_Time:
    '''
    2022.06.05 : module of gnss time        by Chang Chuntao
    '''
    def __init__(self, datetime):
        #  year, month, day, hour, minutes, second, doy, gpsweek, dow, sow, gpsweekd, mjd, sod
        self.datetime = datetime
        [year, doy] = datetime2gnssTime(datetime, 'YDOY')
        [year, month, day, hour, minutes, second] = datetime2gnssTime(datetime, 'YmdHMS')
        [mjd, sod] = datetime2gnssTime(datetime, 'MJDSOD')
        [gpsweek, gpsdow] = datetime2gnssTime(datetime, 'GPSWEEKD')
        gpssow = gpsdow * 7 * 86400 + sod
        gpsweekd = gpsweek * 10 + gpsdow
        self.year = year
        self.month = month
        self.day = day
        self.hour = hour
        self.minutes = minutes
        self.second = second
        self.doy = doy
        self.gpsweek = gpsweek
        self.gpsweekd = gpsweekd
        self.dow = gpsdow
        self.mjd = mjd
        self.sod = sod
        self.sow = gpssow

def datetime2allgnssTime(datetime):
    '''
    2022.06.05 : datetime to all GNSS TIME      by Chang Chuntao
    '''
    gnssTime = GNSS_Time(datetime)
    return gnssTime

def gnssTimesTran():
    """
    2022-04-30 : GNSS_Timestran guide
             by Chang Chuntao  -> Version : 1.12
    """
    nowdatetime = datetime.datetime.utcnow()  # Gets the current utc time
    PrintGDD("The current system UTC time is" + str(nowdatetime)[:-7], "normal")
    [YearMonthDaynow, GPSWeekDaynow, YearDoynow, MjdSodnow] = datetime2GnssTime(nowdatetime)
    printTime(nowdatetime, GPSWeekDaynow, YearDoynow, MjdSodnow)
    print("")
    PrintGDD("1. Year Month Day  2. Year Doy  3. GPSWeek DayofWeek  4. MJD SOD", "input")
    PrintGDD("Please enter the time format number you want to convert (eg. 2)", "input")
    inputTime = input("     ")
    while True:
        if inputTime.isdigit() and 0 < int(inputTime) < 5:
            inputTime = int(inputTime)
            if inputTime == 1:
                PrintGDD("Please enter Year Month Day (eg. 2022 04 29)", "input")
                YearMonthDay = input("     ")
                while True:
                    if len(YearMonthDay.split(" ")) == 3:
                        break
                    else:
                        PrintGDD("Please enter the correct Year Month Day (eg. 2022 04 30)", "input")
                        YearMonthDay = input("     ")
                specTime = gnssTime2datetime(YearMonthDay, "YearMonthDay")
                break
            elif inputTime == 2:
                PrintGDD("Please enter Year Doy (eg. 2022 119)", "input")
                YearDoy = input("     ")
                while True:
                    if len(YearDoy.split(" ")) == 2:
                        break
                    else:
                        PrintGDD("Please enter the correct Year Doy (eg. 2022 119)", "input")
                        YearDoy = input("     ")
                specTime = gnssTime2datetime(YearDoy, "YearDoy")
                break
            elif inputTime == 3:
                PrintGDD("Please enter GPSWeek DayofWeek (eg. 2207 5)", "input")
                GPSWeekDay = input("     ")
                while True:
                    if len(GPSWeekDay.split(" ")) == 2:
                        break
                    else:
                        PrintGDD("Please enter the correct GPSWeek DayofWeek (eg. 2207 5)", "input")
                        GPSWeekDay = input("     ")
                specTime = gnssTime2datetime(GPSWeekDay, "GPSWeekDay")
                break
            elif inputTime == 4:
                PrintGDD("Please enter MJD SOD (eg. 59698 69656.17121)", "input")
                MjdSod = input("     ")
                while True:
                    if len(MjdSod.split(" ")) == 2:
                        break
                    else:
                        PrintGDD("Please enter the correct MJD SOD (eg. 59698 69656.17121)", "input")
                        MjdSod = input("     ")
                specTime = gnssTime2datetime(MjdSod, "MjdSod")
                break
        else:
            PrintGDD("Please enter the correct number (eg. 2)", "input")
            inputTime = input("     ")
    [YearMonthDay, GPSWeekDay, YearDoy, MjdSod] = datetime2GnssTime(specTime)
    printTime(specTime, GPSWeekDay, YearDoy, MjdSod)
    print("")
    return 0


###########################datetime->other############################
def datetime2ymd(specTime):
    """
    2022.09.30 : datetime -> mjd
             by ChangChuntao -> Version : 1.00
    """
    return specTime.year, specTime.month, specTime.day

def ReplaceTimeWildcard(string, spectime):
    """
    2022-03-27 :    * Replace time wild cards in string with specific date time
                    by Jiang Kecai -> Version : 1.00
    2023-03-17 :    + YYYY_F / YYYY_B / MONTH_F / MONTH_B / DAY_F/ DAY_B
                    by Chang Chuntao  -> Version : 2.08
    """
    import datetime
    newstr = str(string)
    spectime_B = spectime + datetime.timedelta(days=-1)
    spectime_B2 = spectime + datetime.timedelta(days=-2)
    spectime_F = spectime + datetime.timedelta(days=1)
    spectime_F2 = spectime + datetime.timedelta(days=2)
    spectime_F3 = spectime + datetime.timedelta(days=3)
    # replace four digit GPS week
    if newstr.find('<GPSW>') >= 0:
        deltime = spectime - datetime.datetime(year=1980, month=1, day=6)
        gpsweek = deltime.days // 7
        newstr = newstr.replace('<GPSW>', '%04d' % gpsweek)
    # replace four digit GPS week and one digit week day
    if newstr.find('<GPSWD>') >= 0:
        deltime = spectime - datetime.datetime(year=1980, month=1, day=6)
        gpsweek = deltime.days // 7
        gpswday = deltime.days - gpsweek * 7
        newstr = newstr.replace('<GPSWD>', '%04d%1d' % (gpsweek, gpswday))
    if newstr.find('<GPSWD_F>') >= 0:
        deltime = spectime_F - datetime.datetime(year=1980, month=1, day=6)
        gpsweek = deltime.days // 7
        gpswday = deltime.days - gpsweek * 7
        newstr = newstr.replace('<GPSWD_F>', '%04d%1d' % (gpsweek, gpswday))
    # replace four digit year
    if newstr.find('<YEAR>') >= 0:
        newstr = newstr.replace('<YEAR>', '%04d' % spectime.year)
    if newstr.find('<YYYY>') >= 0:
        newstr = newstr.replace('<YYYY>', '%04d' % spectime.year)
    if newstr.find('<YY>') >= 0:
        newstr = newstr.replace('<YY>', '%02d' % (spectime.year % 100))
    # replace three digit day number in year
    if newstr.find('<DOY>') >= 0:
        deltime = spectime - datetime.datetime(year=spectime.year, month=1, day=1)
        newstr = newstr.replace('<DOY>', '%03d' % (deltime.days + 1))
    if newstr.find('<DOY_B>') >= 0:
        deltime = spectime_B - datetime.datetime(year=spectime_B.year, month=1, day=1)
        newstr = newstr.replace('<DOY_B>', '%03d' % (deltime.days + 1))
    if newstr.find('<DOY_F>') >= 0:
        deltime = spectime_F - datetime.datetime(year=spectime_F.year, month=1, day=1)
        newstr = newstr.replace('<DOY_F>', '%03d' % (deltime.days + 1))
    if newstr.find('<DOY_F2>') >= 0:
        deltime = spectime_F2 - datetime.datetime(year=spectime_F2.year, month=1, day=1)
        newstr = newstr.replace('<DOY_F2>', '%03d' % (deltime.days + 1))        
    if newstr.find('<DOY_F3>') >= 0:
        deltime = spectime_F3 - datetime.datetime(year=spectime_F3.year, month=1, day=1)
        newstr = newstr.replace('<DOY_F3>', '%03d' % (deltime.days + 1))        
    if newstr.find('<DDD>') >= 0:
        newstr = newstr.replace('<DDD>', spectime.strftime('%j'))
    # replace two_digit month in year
    if newstr.find('<MONTH>') >= 0:
        newstr = newstr.replace('<MONTH>', '%02d' % spectime.month)
    # replace two digit day in month
    if newstr.find('<DAY>') >= 0:
        newstr = newstr.replace('<DAY>', '%02d' % spectime.day)

    # replace four digit year
    if newstr.find('<YYYY_F>') >= 0:
        newstr = newstr.replace('<YYYY_F>', '%04d' % spectime_F.year)
    # replace two digit hour in day
    if newstr.find('<MONTH_F>') >= 0:
        newstr = newstr.replace('<MONTH_F>', '%02d' % spectime_F.month)
    if newstr.find('<DAY_F>') >= 0:
        newstr = newstr.replace('<DAY_F>', '%02d' % spectime_F.day)
    if newstr.find('<YY_F>') >= 0:
        newstr = newstr.replace('<YY_F>', '%02d' % (spectime_F.year % 100))

    # replace four digit year
    if newstr.find('<YYYY_B>') >= 0:
        newstr = newstr.replace('<YYYY_B>', '%04d' % spectime_B.year)
    if newstr.find('<MONTH_B>') >= 0:
        newstr = newstr.replace('<MONTH_B>', '%02d' % spectime_B.month)
    # replace two digit day in month
    if newstr.find('<DAY_B>') >= 0:
        newstr = newstr.replace('<DAY_B>', '%02d' % spectime_B.day)
    if newstr.find('<YY_B>') >= 0:
        newstr = newstr.replace('<YY_B>', '%02d' % (spectime_B.year % 100))

    # replace two digit hour in day
    if newstr.find('<HOUR>') >= 0:
        newstr = newstr.replace('<HOUR>', '%02d' % spectime.hour)
    # replace two digit minute in hour
    if newstr.find('<MINUTE>') >= 0:
        newstr = newstr.replace('<MINUTE>', '%02d' % spectime.minute)
    # replace two digit second in minute
    if newstr.find('<SECOND>') >= 0:
        newstr = newstr.replace('<SECOND>', '%02d' % spectime.second)
    if newstr.find('<MM>') >= 0:
        newstr = newstr.replace('<MM>', '%02d' % spectime.month)
    if newstr.find('<MMM>') >= 0:
        month_num = spectime.month
        if month_num == 1:
            month_str = "JAN"
        elif month_num == 2:
            month_str = "FEB"
        elif month_num == 3:
            month_str = "MAR"
        elif month_num == 4:
            month_str = "APR"
        elif month_num == 5:
            month_str = "MAY"
        elif month_num == 6:
            month_str = "JUN"
        elif month_num == 7:
            month_str = "JUL"
        elif month_num == 8:
            month_str = "AUG"
        elif month_num == 9:
            month_str = "SEP"
        elif month_num == 10:
            month_str = "OCT"
        elif month_num == 11:
            month_str = "NOV"
        elif month_num == 12:
            month_str = "DEC"
        else:
            month_str = "JAN"
        newstr = newstr.replace('<MMM>', month_str)
    if newstr.find('<WEEK0DOY>') >= 0:
        find_gpsweek, wd = datetime2gpswd(spectime)
        year, week_0_doy = gpswd2doy(find_gpsweek, 0)
        newstr = newstr.replace('<WEEK0DOY>', '%03d' % week_0_doy)

    # return new string
    return newstr

def datetime2doy(specTime):
    """
    2022.09.30 : datetime -> ydoy
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    doy = (specTime - datetime.datetime(year=specTime.year, month=1, day=1)).days + 1
    return specTime.year, doy


def datetime2mjd(specTime):
    """
    2022.09.30 : datetime -> mjd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)
    mjd = (specTime - mjdT0).days
    sod = specTime.hour * 3600.0 + specTime.minute * 60.0 + specTime.second + specTime.microsecond / 1000000.0
    return mjd, sod


def datetime2gpswd(specTime):
    """
    2022.09.30 : datetime -> gpswd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    gpsWeekdDelTime = specTime - datetime.datetime(year=1980, month=1, day=6)
    gpsWeek = gpsWeekdDelTime.days // 7
    gpsWeekD = gpsWeekdDelTime.days - gpsWeek * 7
    return gpsWeek, gpsWeekD


###########################datetime->other############################


##############################ymd->other##############################
def ymd2datetime(year, month, day):
    """
    2022.09.30 : ymd -> datetime
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    specTime = datetime.datetime(year, month, day)
    return specTime


def ymd2doy(year, month, day):
    """
    2022.09.30 : ymd -> doy
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    specTime = datetime.datetime(year, month, day)
    doy = (specTime - datetime.datetime(year=specTime.year, month=1, day=1)).days + 1
    return specTime.year, doy


def ymd2mjd(year, month, day):
    """
    2022.09.30 : ymd -> mjd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    specTime = datetime.datetime(year, month, day)
    mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)
    mjd = (specTime - mjdT0).days
    sod = specTime.hour * 3600.0 + specTime.minute * 60.0 + specTime.second + specTime.microsecond / 1000000.0
    return mjd, sod


def ymd2gpswd(year, month, day):
    """
    2022.09.30 : ymd -> gpswd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    specTime = datetime.datetime(year, month, day)
    gpsWeekdDelTime = specTime - datetime.datetime(year=1980, month=1, day=6)
    gpsWeek = gpsWeekdDelTime.days // 7
    gpsWeekD = gpsWeekdDelTime.days - gpsWeek * 7
    return gpsWeek, gpsWeekD


##############################ymd->other##############################

##############################doy->other##############################
def doy2datetime(year, doy):
    """
    2022.09.30 : year doy -> datetime
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    day1Time = datetime.datetime(year, 1, 1)
    specTime = day1Time + datetime.timedelta(days=int(doy) - 1)
    return specTime


def doy2ymd(year, doy):
    """
    2022.09.30 : year doy -> ymd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    day1Time = datetime.datetime(year, 1, 1)
    specTime = day1Time + datetime.timedelta(days=int(doy) - 1)
    return specTime.year, specTime.month, specTime.day


def doy2gpswd(year, doy):
    """
    2022.09.30 : year doy -> gpswd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    day1Time = datetime.datetime(year, 1, 1)
    specTime = day1Time + datetime.timedelta(days=int(doy) - 1)
    gpsWeekdDelTime = specTime - datetime.datetime(year=1980, month=1, day=6)
    gpsWeek = gpsWeekdDelTime.days // 7
    gpsWeekD = gpsWeekdDelTime.days - gpsWeek * 7
    return gpsWeek, gpsWeekD


def doy2mjd(year, doy):
    """
    2022.09.30 : year doy -> mjd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    day1Time = datetime.datetime(year, 1, 1)
    specTime = day1Time + datetime.timedelta(days=int(doy) - 1)
    mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)
    mjd = (specTime - mjdT0).days
    sod = specTime.hour * 3600.0 + specTime.minute * 60.0 + specTime.second + specTime.microsecond / 1000000.0
    return mjd, sod


##############################doy->other##############################

##############################mjd->other##############################
def mjd2datetime(mjd, sod=0.0):
    """
    2022.09.30 : mjd -> datetime
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)
    specTime = mjdT0 + datetime.timedelta(days=int(mjd)) + datetime.timedelta(seconds=float(sod))
    return specTime


def mjd2ymd(mjd, sod=0.0):
    """
    2022.09.30 : mjd -> ymd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)
    specTime = mjdT0 + datetime.timedelta(days=int(mjd)) + datetime.timedelta(seconds=float(sod))
    return specTime.year, specTime.month, specTime.day


def mjd2doy(mjd, sod=0.0):
    """
    2022.09.30 : mjd -> doy
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)
    specTime = mjdT0 + datetime.timedelta(days=int(mjd)) + datetime.timedelta(seconds=float(sod))
    doy = (specTime - datetime.datetime(year=specTime.year, month=1, day=1)).days + 1
    return specTime.year, doy


def mjd2gpswd(mjd, sod=0.0):
    """
    2022.09.30 : mjd -> gpswd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)
    specTime = mjdT0 + datetime.timedelta(days=int(mjd)) + datetime.timedelta(seconds=float(sod))
    gpsWeekdDelTime = specTime - datetime.datetime(year=1980, month=1, day=6)
    gpsWeek = gpsWeekdDelTime.days // 7
    gpsWeekD = gpsWeekdDelTime.days - gpsWeek * 7
    return gpsWeek, gpsWeekD


def sod2hms(sod):
    """
    2022.09.30 : Convert integer second in one day to hour minute second
             by ChangChuntao -> Version : 1.00
    """
    sec = sod
    hour = sec // 3600
    sec -= hour * 3600
    if (sec < 0):
        hour -= 1
        sec += 3600
    minu = sec // 60
    sec -= minu * 60
    return hour, minu, sec


##############################mjd->other##############################


#############################gpswd->other#############################
def gpswd2datetime(gpsWeek, gpsWeekD):
    """
    2022.09.30 : gpswd -> datetime
             by ChangChuntao -> Version : 1.00
    """
    import datetime

    wd1Time = datetime.datetime(year=1980, month=1, day=6)
    specTime = wd1Time + datetime.timedelta(weeks=int(gpsWeek)) + datetime.timedelta(days=int(gpsWeekD))
    return specTime


def gpswd2ymd(gpsWeek, gpsWeekD):
    """
    2022.09.30 : gpswd -> ymd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    wd1Time = datetime.datetime(year=1980, month=1, day=6)
    specTime = wd1Time + datetime.timedelta(weeks=int(gpsWeek)) + datetime.timedelta(days=int(gpsWeekD))
    return specTime.year, specTime.month, specTime.day


def gpswd2doy(gpsWeek, gpsWeekD):
    """
    2022.09.30 : gpswd -> year doy
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    wd1Time = datetime.datetime(year=1980, month=1, day=6)
    specTime = wd1Time + datetime.timedelta(weeks=int(gpsWeek)) + datetime.timedelta(days=int(gpsWeekD))
    doy = (specTime - datetime.datetime(year=specTime.year, month=1, day=1)).days + 1
    return specTime.year, doy


def gpswd2mjd(gpsWeek, gpsWeekD):
    """
    2022.09.30 : gpswd -> mjd
             by ChangChuntao -> Version : 1.00
    """
    import datetime
    wd1Time = datetime.datetime(year=1980, month=1, day=6)
    specTime = wd1Time + datetime.timedelta(weeks=int(gpsWeek)) + datetime.timedelta(days=int(gpsWeekD))
    mjdT0 = datetime.datetime(1858, 11, 17, 0, 0, 0, 0)
    mjd = (specTime - mjdT0).days
    sod = specTime.hour * 3600.0 + specTime.minute * 60.0 + specTime.second + specTime.microsecond / 1000000.0
    return mjd, sod
#############################gpswd->other#############################
