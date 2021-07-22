#pragma once
#include <string>
#include <iostream>

std::string timeformat(int n, int bit);

class Date
{
public:
	Date();
	Date(std::string);//"2021-06-01"  "2021/06/01"
	Date(int year, int month, int day);
	std::string toStr();
public:
	int m_year;
	int m_month;
	int m_day;
};

class Time
{
public:
	Time();
	Time(std::string);
	Time(int sec);
	Time operator-(Time& time1);
	Time operator+(Time& time1);
	bool operator<(Time& time1);
	bool operator>(Time& time1);
	std::string toString();
	int toSec();
	Time toTime(int sec);
public:
	int hour=0;
	int minute=0;
	int second=0;
};

class XTime
{
public:
	XTime(std::string time);
	XTime(const XTime&);

	std::string GetError();
	Time GetTime();
	Date GetDate();
	Time CalculateAddtime(Time  time1, Time time2);
	Time CalculateSubtime(Time time1, Time time2);
	std::string TimeToString(Time time);

private:
	bool init(std::string time);
	bool checktime(std::string time);
	bool IsValidYear(int year);
	bool IsValidMonth(int month);
	bool IsValidDay(int day);
	bool IsValidHour(int hour);
	bool IsValidMinute(int minute);
	bool IsValidSecond(int second);

	std::string timeformat(int n, int bit);
private:
	Date m_Date;
	Time m_Time;

	std::string m_error;
};
/*
2020/12/12 19:12:01
2020-12-12 19:12:01
*/
