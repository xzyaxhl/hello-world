#include "XTime.h"
#include <string>
#include <cmath>

using namespace std;

std::string timeformat(int n, int bit)
{
	string stemp = to_string(n);
	int bit1 = stemp.size();
	for (bit1; bit1 < bit; bit1++)
	{
		stemp = "0" + stemp;
	}
	return stemp;
}

Date::Date()
{

}

Date::Date(std::string str)
{

}

Date::Date(int year, int month, int day):m_year(year),m_month(month),m_day(day)
{
	
}

std::string Date::toStr()
{
	return to_string(m_year)+to_string(m_month)+to_string(m_day);

}
Time::Time()
{

}

Time::Time(std::string time)
{
	int n1 = time.find(':');
	if (n1 != 1 && n1 != 2)
		return;
	hour = stoi(time.substr(0, n1));
	int n2 = time.find(':', n1 + 1);
	if (n2 != n1 + 2 && n2 != n1 + 3)
		return;
	minute = stoi(time.substr(n1 + 1, n2 - n1 - 1));
	second = stoi(time.substr(n2 + 1, time.size() - n2));
}

Time Time::operator-(Time& time)
{
	Time time1;
	Time time2;
	bool flag = true;//this>time1
	if (*this < time)
	{
		flag = 0;
		time2 = time;
		time1 = *this;
	}
	else
	{
		time2 = *this;
		time1 = time;
	}
	int sec = time2.toSec() - time1.toSec();
	Time time3(sec);
	return time3;
}

Time Time::operator+(Time& time1)
{
	Time time;
	int carry = 0;
	time.second = this->second + time1.second;
	if (time.second >59)
	{
		carry = 1;
		time.second = time.second- 60;
	}
	time.minute = this->minute + time1.minute + carry;
	carry = 0;
	if (time.minute >59)
	{
		carry = 1;
		time.minute = time.minute-60;
	}
	time.hour = this->hour + time1.hour + carry;
	return time;
}

bool Time::operator<(Time& time1)
{
	if (this->hour < time1.hour ||
		(this->hour == time1.hour && this->minute < time1.minute) ||
		(this->hour == time1.hour && this->minute == time1.minute && this->second < time1.second)
		)
		return true;
	return false;
}
bool Time::operator>(Time& time1)
{
	if (this->hour > time1.hour ||
		(this->hour == time1.hour && this->minute > time1.minute) ||
		(this->hour == time1.hour && this->minute == time1.minute && this->second > time1.second)
		)
		return true;
	return false;
}

std::string Time::toString()
{
	return timeformat(hour, 2) + ":" + timeformat(minute, 2) + ":" + timeformat(second, 2);
}

int Time::toSec()
{
	int sec = 0;
	if (hour < 0)
	{
		sec = hour * 3600 - minute * 60 - second;
	}
	else
	{
		sec = hour * 3600 + minute * 60 + second;
	}
	return sec;
}

Time::Time(int sec)
{
	bool flag = true;//正数
	if (sec < 0)
	{
		flag = false;
		sec = 0 - sec;
	}
	hour = sec / 3600;
	minute = (sec % 3600) / 60;
	second = (sec % 3600) % 60;
	if (!flag)
		hour = 0 - hour;
}












XTime::XTime(std::string time)
{
	if (!init(time))
		cout << "输入格式有误!" << endl;
}

XTime::XTime(const XTime&)
{

}

bool XTime::init(std::string time)
{
	return checktime(time);
}

bool XTime::checktime(std::string time)
{

	cout << time << time.size() << endl;
	if (time.size() > 19)
		return false;
	int n = 0;
	string timehead;
	string timetail;
	if ((n = time.find(' ', 0)) != std::string::npos)
	{
		timehead = time.substr(0, n);
		timetail = time.substr(n + 1);
		cout << timehead << "___" << timetail << endl;
		if (time.find('/') != std::string::npos)
		{
			int n1 = timehead.find('/');
			if (n1 != 4)
				return false;
			m_Date.m_year = stoi(timehead.substr(0, n1));
			int n2 = timehead.find('/', n1 + 1);
			if (n2 == n1 + 1 || n2 >= n1 + 4 || n2 == -1)
				return false;
			m_Date.m_month = stoi(timehead.substr(n1 + 1, n2 - n1 - 1));
			m_Date.m_day = stoi(timehead.substr(n2 + 1, timehead.size() - n2));
			//cout << m_year << "_" << m_month << "_" << m_day << endl;	
		}
		else
		{
			int n1 = timehead.find('-');
			if (n1 != 4)
				return false;
			m_Date.m_year = stoi(timehead.substr(0, n1));
			int n2 = timehead.find('-', n1 + 1);
			if (n2 == n1 + 1 || n2 >= n1 + 4 || n2 == -1)
				return false;
			m_Date.m_month = stoi(timehead.substr(n1 + 1, n2 - n1 - 1));
			m_Date.m_day = stoi(timehead.substr(n2 + 1, timehead.size() - n2));
			//cout << m_year << "_" << m_month << "_" << m_day << endl;
		}
		if (!IsValidYear(m_Date.m_year) || !IsValidMonth(m_Date.m_month) || !IsValidDay(m_Date.m_day))
			return false;
	}
	else {
		timetail = time;
	}
	//
	int n1 = timetail.find(':');
	if (n1 != 1 && n1 != 2)
		return false;
	m_Time.hour = stoi(timetail.substr(0, n1));
	int n2 = timetail.find(':', n1 + 1);
	if (n2 != n1 + 2 && n2 != n1 + 3)
		return false;
	m_Time.minute = stoi(timetail.substr(n1 + 1, n2 - n1 - 1));
	m_Time.second = stoi(timetail.substr(n2 + 1, timetail.size() - n2));
	//cout << m_hour << "_" << m_minute << "_" << m_second << endl;
	if (!IsValidHour(m_Time.hour) || !IsValidMinute(m_Time.minute) || !IsValidSecond(m_Time.second))
		return false;
	return true;
}

bool XTime::IsValidYear(int year)
{
	if (year < 1 || year>9999)
	{
		m_error = "year is error! year: " + to_string(year);
		return false;
	}
	return true;
}
bool XTime::IsValidMonth(int month)
{
	if (month < 1 || month>12)
	{
		m_error = "month is error! month: " + to_string(month);
		return false;
	}
	return true;
}
bool XTime::IsValidDay(int day)
{
	if (day < 0 || day>31)
	{
		m_error = "day is error! day: " + to_string(day);
		return false;
	}
	if (day >= 29)
	{
		if (day == 31)
		{
			switch (m_Date.m_month)
			{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				return true;
			default:
				m_error = "day and month is error! expect day is 30: day:" + to_string(day) + " month: " + to_string(m_Date.m_month);
				return false;
			}
		}
		else if (day = 30)
		{
			if (m_Date.m_month == 2)
			{
				m_error = "day and month is error! expect day is 28 or 29 : day:" + to_string(day) + " month: " + to_string(m_Date.m_month);
				return false;
			}
		}
		else
		{
			if (m_Date.m_month == 2)
			{
				if (m_Date.m_year % 4 != 0)
				{
					m_error = "day and month is error! expect day is 28: day:" + to_string(day) + " month: " + to_string(m_Date.m_month) + " year: " + to_string(m_Date.m_year);
					return false;
				}
			}
		}
	}
	return true;
}
bool XTime::IsValidHour(int hour)
{
	if (hour < 0 || hour>24)
	{
		m_error = "hour is error! hour: " + to_string(hour);
		return false;
	}
	return true;
}
bool XTime::IsValidMinute(int minute)
{
	if (minute < 0 || minute>59)
	{
		m_error = "minute is error! minute: " + to_string(minute);
		return false;
	}
	return true;
}
bool XTime::IsValidSecond(int second)
{
	if (second < 0 || second>59)
	{
		m_error = "second is error! second: " + to_string(second);
		return false;
	}
	return true;
}

std::string XTime::GetError()
{
	return m_error;
}

Time XTime::CalculateAddtime(Time time1, Time time2)
{
	Time temptime;
	int carry = 0;
	temptime.second = time1.second + time2.second;
	if (temptime.second > 59)
	{
		temptime.second -= 60;
		carry = 1;
	}
	temptime.minute = time1.minute + time2.minute + carry;
	if (temptime.minute > 59)
	{
		temptime.minute -= 60;
		carry = 1;
	}

	temptime.hour = time1.hour + time2.hour + carry;
	return temptime;
}

Time XTime::CalculateSubtime(Time time1, Time time2)
{
	Time temptime;
	int carry = 0;
	temptime.second = time1.second - time2.second;
	if (temptime.second < 0)
	{
		temptime.second = abs(temptime.second);
		carry = -1;
	}
	temptime.minute = time1.minute - time2.minute;
	if (temptime.minute < 0)
	{
		temptime.minute = abs(temptime.minute) + carry;
		carry = -1;
	}

	temptime.hour = time1.hour - time2.hour;
	if (temptime.hour < 0)
	{
		temptime.hour = abs(temptime.hour) + carry;
	}
	return temptime;
}

std::string XTime::TimeToString(Time time)
{
	std::string temp = timeformat(time.hour, 2) + ":" + timeformat(time.minute, 2) + ":" + timeformat(time.second, 2);
	return temp;
}

std::string XTime::timeformat(int n, int bit)
{
	string stemp = to_string(n);
	int bit1 = stemp.size();
	for (bit1; bit1 < bit; bit1++)
	{
		stemp = "0" + stemp;
	}
	return stemp;
}

Time XTime::GetTime()
{
	Time time;
	time.hour = m_Time.hour;
	time.minute = m_Time.minute;
	time.second = m_Time.second;
	return time;
}



Date XTime::GetDate()
{
	Date date;
	date.m_year = m_Date.m_year;
	date.m_month = m_Date.m_month;
	date.m_day = m_Date.m_day;
	return date;
}