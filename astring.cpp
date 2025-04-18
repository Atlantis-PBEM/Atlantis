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

AString *AString::gettoken()
{
	char buf[1024];
	int place = 0;
	int place2 = 0;
	while (place < len && (str[place] == ' ' || str[place] == '\t'))
		place++;
	if (place >= len) return 0;
	if (str[place] == ';') return 0;

	if (str[place] == '"') {
		place++;
		while (place < len && str[place] != '"') {
			buf[place2++] = str[place++];
		}
		if (place != len) {
			/* Get rid of extra " */
			place++;
		} else {
			/* Unmatched "" return 0 */
			delete[] str;
			str = new char[1];
			len = 0;
			str[0] = '\0';
			return 0;
		}
	} else {
		while (place<len &&
				(str[place]!=' ' && str[place]!='\t' && str[place]!=';')) {
			buf[place2++] = str[place++];
		}
	}
	buf[place2] = '\0';
	if (place == len || str[place] == ';') {
		delete[] str;
		str = new char[1];
		len = 0;
		str[0] = '\0';
		return new AString(buf);
	}
	char * buf2 = new char[len-place2+1];
	int newlen = 0;
	place2 = 0;
	while (place < len) {
		buf2[place2++] = str[place++];
		newlen++;
	}
	buf2[place2] = '\0';
	len = newlen;
	delete[] str;
	str = buf2;
	return new AString(buf);
}

AString *AString::StripWhite()
{
	int place = 0;
	while (place < len && (str[place] == ' ' || str[place] == '\t')) {
		place++;
	}
	if (place >= len) {
		return 0;
	}
	return( new AString( &str[ place ] ));
}

bool AString::getat()
{
	int place = 0;
	while (place < len && (str[place] == ' ' || str[place] == '\t'))
		place++;
	if (place >= len) return 0;
	if (str[place] == '@') {
		str[place] = ' ';
		return true;
	}
	return false;
}

char islegal(char c)
{
	if ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') ||
			c=='!' || c==',' || c=='.' || c==' ' ||
			c=='{' || c=='}' || c=='@' || c=='#' || c=='$' || c=='%' ||
			c=='^' || c=='&' || c=='*' || c=='-' || c=='_' || c=='+' ||
			c=='=' || c==':' || c=='<' || c=='>' || c=='?' ||
			c=='/' || c=='~' || c=='\'' || c== '\\' || c=='`')
		return 1;
	return 0;
}

AString *AString::getlegal()
{
	char * temp = new char[len+1];
	char * temp2 = temp;
	int j = 0;
	for (int i=0; i<len; i++) {
		if (islegal(str[i])) {
			*temp2 = str[i];
			if (str[i] != ' ') j=1;
			temp2++;
		}
	}

	if (!j) {
		delete[] temp;
		return 0;
	}

	*temp2 = '\0';
	AString * retval = new AString(temp);
	delete[] temp;
	return retval;
}

AString *AString::stripnumber()
{
	char *temp = new char[len+1];
	int i = len  - 1;

	// walk back until we find either a [ or a a ( which will be the unit/faction or object identifier
	while (i > 0 && (str[i] != '[' && str[i] != '(')) {
		i--;
	}
	// walk back one more step to get rid of the space;
	if (i > 0) i--;
	// copy the string up to that point
	strncpy(temp, str, i);
	temp[i] = '\0';
	AString * retval = new AString(temp);
	delete[] temp;
	return retval;
}

int AString::CheckPrefix(const AString &s)
{
	if (Len() < s.len) return 0;

	AString x = *this;
	x.str[s.len] = '\0';
	x.len = s.len;

	return AString(x) == s;
}

AString *AString::Trunc(int val, int back)
{
	int l=Len();
	if (l <= val) return 0;
	for (int i = 0; i < val; i++) {
		if (str[i] == '\n' || str[i] == '\r') {
			str[i] = '\0';
			return new AString(&(str[i+1]));
		}
	}
	for (int i=val; i>(val-back); i--) {
		if (str[i] == ' ') {
			str[i] = '\0';
			return new AString(&(str[i+1]));
		}
	}
	AString * temp = new AString(&(str[val]));
	str[val] = '\0';
	return temp;
}

int AString::value()
{
	int place = 0;
	int ret = 0;
	while ((str[place] >= '0') && (str[place] <= '9')) {
		ret *= 10;
		// Fix bug where int could be overflowed.
		if (ret < 0) return 0;
		ret += (str[place++] - '0');
	}
	return ret;
}

int AString::strict_value() //this cannot handle negative numbers!
{
	int l=Len();
	for (int i=0; i<l; i++) {
		if (!((str[i] >= '0') && (str[i] <= '9'))) {
			return -1;
		}
	}

	int place = 0;
	int ret = 0;
	while ((str[place] >= '0') && (str[place] <= '9')) {
		ret *= 10;
		// Fix bug where int could be overflowed.
		if (ret < 0) return 0;
		ret += (str[place++] - '0');
	}
	return ret;
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

const std::string plural(int count, const std::string &one, const std::string &many) {
	return count != 1 ? many : one;
}

const std::string join(const std::string &delimeter, const std::vector<std::string>& items) {
	std::ostringstream buffer;

	bool next = false;
	for (auto &item : items) {
		if (next) {
			buffer << delimeter;
		}

		buffer << item;
		next = true;
	}

	const std::string result = buffer.str();
	return result;
}

const std::string join(const std::string &delimeter, const std::string &lastDelimeter, const std::vector<std::string>& items) {
	std::ostringstream buffer;

	std::size_t size = items.size();
	std::size_t i = 0;

	for (auto &item : items) {
		if (i > 0) {
			buffer << (i == size - 1 ? lastDelimeter : delimeter);
		}

		buffer << item;
		i++;
	}

	const std::string result = buffer.str();
	return result;
}

const bool endsWith(const std::string &str, const std::string &search) {
    if (str.length() < search.length()) {
        return false;
    }

	return std::equal(search.rbegin(), search.rend(), str.rbegin());
}

const bool startsWith(const std::string &str, const std::string &search) {
    if (search.length() > str.length()) {
        return false;
    }

	return std::equal(search.begin(), search.end(), str.begin());
}

const std::string capitalize(const std::string &str) {
	if (str.empty()) {
		return str;
	}

	std::string s = str;
	s[0] = toupper(str[0]);

	return s;
}
