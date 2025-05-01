// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 1995-1999 Geoff Dunbar
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program, in the file license.txt. If not, write
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// See the Atlantis Project web page for details:
// http://www.prankster.com/project
//
// END A3HEADER
#include "astring.h"
#include <string.h>
#include <stdio.h>
#include <sstream>

AString::AString()
{
	len = 0;
	str = new char[1];
	str[0] = '\0';
}

AString::AString(char *s)
{
	len = 0;
	if (s) len = strlen(s);
	str = new char[len + 1];
	str[0] = '\0';
	if (s) strcpy(str,s);
}

AString::AString(const char *s)
{
	len = 0;
	if (s) len = strlen(s);
	str = new char[len + 1];
	str[0] = '\0';
	if (s) strcpy(str,s);
}

AString::AString(int l)
{
	char buf[16];
	snprintf(buf, 16, "%d", l);
	len = strlen(buf);
	str = new char[len+1];
	strcpy(str,buf);
}

AString::AString(unsigned int l)
{
	char buf[16];
	snprintf(buf, 16, "%u", l);
	len = strlen(buf);
	str = new char[len+1];
	strcpy(str,buf);
}

AString::AString(char c)
{
	len = 1;
	str = new char[2];
	str[0] = c;
	str[1] = '\0';
}

AString::AString(const std::string & s) {
	auto buffer = s.c_str();

	len = strlen(buffer);
	str = new char[len + 1];
	strcpy(str, buffer);
}

AString::~AString()
{
	if (str) delete[] str;
	str = NULL;
}

AString::AString(const AString &s)
{
	len = s.len;
	str = new char[len + 1];
	strcpy(str,s.str);
}

AString & AString::operator=(const AString &s)
{
	len = s.len;
	if (str) delete[] str;
	str = new char[len + 1];
	strcpy(str,s.str);
	return *this;
}

AString & AString::operator=(const char *c)
{
	len = 0;
	if (c) len = strlen(c);
	if (str) delete[] str;
	str = new char[len + 1];
	if (c) strcpy(str,c);
	return *this;
}

int AString::operator==(char *s) const
{
	return isEqual(s);
}

int AString::operator==(const char *s) const
{
	return isEqual(s);
}

int AString::operator==(const AString &s) const
{
	return isEqual(s.str);
}

int AString::isEqual(const char *temp2) const
{
	char *temp1 = str;

	// Handle comparisons with null
	if (temp1 && !temp2) return 0;
	if (temp2 && !temp1) return 0;
	if (!temp1 && !temp2) return 1;

	while ((*temp1) && (*temp2)) {
		char t1 = *temp1;
		if ((t1 >= 'A') && (t1 <= 'Z'))
			t1 = t1 - 'A' + 'a';
		if (t1 == '_') t1 = ' ';
		char t2 = *temp2;
		if ((t2 >= 'A') && (t2 <= 'Z'))
			t2 = t2 - 'A' + 'a';
		if (t2 == '_') t2 = ' ';
		if (t1 != t2) return 0;
		temp1++;
		temp2++;
	}
	if (*temp1==*temp2) return 1;
	return 0;
}

AString AString::operator+(const AString &s)
{
	char *temp = new char[len+s.len+1];
	int i;
	for (i=0; i<len; i++) {
		temp[i] = str[i];
	}
	for (int j=0; j<s.len+1; j++) {
		temp[i++] = s.str[j];
	}
	AString temp2 = AString(temp);
	delete[] temp;
	return temp2;
}

AString &AString::operator+=(const AString &s)
{
	char *temp = new char[len+s.len+1];
	int i;
	for (i=0; i<len; i++) {
		temp[i] = str[i];
	}
	for (int j=0; j<s.len+1; j++) {
		temp[i++] = s.str[j];
	}
	delete[] str;
	str = temp;
	len = len + s.len;
	return *this;
}


char *AString::Str()
{
	return str;
}


const char *AString::const_str() const
{
	return str;
}

int AString::Len()
{
	return len;
}


std::ostream& operator<<(std::ostream &os,const AString &s)
{
	os << s.str;
	return os;
}

std::istream& operator>>(std::istream &is,AString &s)
{
	// We expect to read a line at a time from the file, not a string at a time since we do tokenization internally.
	std::string buf;
	getline(is, buf);
	s.len = strlen(buf.c_str());
	s.str = new char[s.len + 1];
	strcpy(s.str,buf.c_str());
	return is;
}
