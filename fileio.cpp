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
#include "fileio.h"
#include "gameio.h"

#include <iostream>
#include <fstream>
using namespace std;

#define F_ENDLINE '\n'

extern long _ftype,_fcreator;

Aoutfile::Aoutfile()
{
	file = new ofstream;
}

Aoutfile::~Aoutfile()
{
	delete file;
}

Areport::Areport()
{
	file = new ofstream;
}

Areport::~Areport()
{
	delete file;
}

Arules::Arules()
{
	file = new ofstream;
}

Arules::~Arules()
{
	delete file;
}

void Aoutfile::Open(const AString &s)
{
	while(!(file->rdbuf()->is_open())) {
		AString *name = getfilename(s);
		file->open(name->Str(), ios::out|ios::ate);
		delete name;
		// Handle a broke ios::ate implementation on some boxes
		file->seekp(0, ios::end);
		if ((int)file->tellp()!= 0) file->close();
	}
}

int Aoutfile::OpenByName(const AString &s)
{
	AString temp = s;
	file->open(temp.Str(), ios::out|ios::ate);
	if (!file->rdbuf()->is_open()) return -1;
	// Handle a broke ios::ate implementation on some boxes
	file->seekp(0, ios::end);
	if ((int)file->tellp() != 0) {
		file->close();
		return -1;
	}
	return 0;
}

void Aoutfile::Close()
{
	file->close();
}

void Areport::Close()
{
	file->close();
}

void Arules::Close()
{
	file->close();
}

void skipwhite(istream *f)
{
	if (f->eof()) return;
	int ch = f->peek();
	while((ch == ' ') || (ch == '\n') || (ch == '\t') ||
			(ch == '\r') || (ch == '\0')) {
		f->get();
		if (f->eof()) return;
		ch = f->peek();
	}
}

void Aoutfile::PutInt(int x)
{
	*file << x;
	*file << F_ENDLINE;
}

void Aoutfile::PutStr(char const *s)
{
	*file << s << F_ENDLINE;
}

void Aoutfile::PutStr(const AString &s)
{
	*file << s << F_ENDLINE;
}

void Areport::Open(const AString &s)
{
	while(!(file->rdbuf()->is_open())) {
		AString *name = getfilename(s);
		file->open(name->Str(),ios::out|ios::ate);
		delete name;
		// Handle a broke ios::ate implementation on some boxes
		file->seekp(0, ios::end);
		if ((int)file->tellp()!=0) file->close();
	}
	tabs = 0;
}

int Areport::OpenByName(const AString &s)
{
	AString temp = s;
	file->open(temp.Str(), ios::out|ios::ate);
	if (!file->rdbuf()->is_open()) return -1;
	// Handle a broke ios::ate implementation on some boxes
	file->seekp(0, ios::end);
	if ((int)file->tellp() != 0) {
		file->close();
		return -1;
	}
	tabs = 0;
	return 0;
}

void Areport::AddTab()
{
	tabs++;
}

void Areport::DropTab()
{
	if (tabs > 0) tabs--;
}

void Areport::ClearTab()
{
	tabs = 0;
}

void Areport::PutStr(const AString &s,int comment)
{
	AString temp;
	for (int i=0; i<tabs; i++) temp += "  ";
	temp += s;
	AString *temp2 = temp.Trunc(70);
	if (comment) *file << ";";
	*file << temp << F_ENDLINE;
	while (temp2) {
		temp = "  ";
		for (int i=0; i<tabs; i++) temp += "  ";
		temp += *temp2;
		delete temp2;
		temp2 = temp.Trunc(70);
		if (comment) *file << ";";
		*file << temp << F_ENDLINE;
	}
}

void Areport::PutNoFormat(const AString &s)
{
	*file << s << F_ENDLINE;
}

void Areport::EndLine()
{
	*file << F_ENDLINE;
}

void Arules::Open(const AString &s)
{
	while(!(file->rdbuf()->is_open())) {
		AString *name = getfilename(s);
		file->open(name->Str(),ios::out|ios::ate);
		delete name;
		// Handle a broke ios::ate implementation on some boxes
		file->seekp(0, ios::end);
		if ((int)file->tellp()!=0) file->close();
	}
	tabs = 0;
	wraptab = 0;
}

int Arules::OpenByName(const AString &s)
{
	AString temp = s;
	file->open(temp.Str(), ios::out|ios::trunc);
	if (!file->rdbuf()->is_open()) return -1;
	// Handle a broke ios::ate implementation on some boxes
	file->seekp(0, ios::end);
	if ((int)file->tellp() != 0) {
		file->close();
		return -1;
	}
	tabs = 0;
	wraptab = 0;
	return 0;
}

void Arules::AddTab()
{
	tabs++;
}

void Arules::DropTab()
{
	if (tabs > 0) tabs--;
}

void Arules::ClearTab()
{
	tabs = 0;
}

void Arules::AddWrapTab()
{
	wraptab++;
}

void Arules::DropWrapTab()
{
	if (wraptab > 0) wraptab--;
}

void Arules::ClearWrapTab()
{
	wraptab = 0;
}

void Arules::PutStr(const AString &s)
{
	AString temp;
	for (int i=0; i<tabs; i++) temp += "  ";
	temp += s;
	AString *temp2 = temp.Trunc(78, 70);
	*file << temp << F_ENDLINE;
	while (temp2) {
		temp = "";
		for (int i=0; i<tabs; i++) temp += "  ";
		temp += *temp2;
		delete temp2;
		temp2 = temp.Trunc(78, 70);
		*file << temp << F_ENDLINE;
	}
}

void Arules::WrapStr(const AString &s)
{
	AString temp;
	for (int i=0; i<wraptab; i++) temp += "  ";
	temp += s;
	AString *temp2 = temp.Trunc(70);
	*file << temp << F_ENDLINE;
	while (temp2) {
		temp = "  ";
		for (int i=0; i<wraptab; i++) temp += "  ";
		temp += *temp2;
		delete temp2;
		temp2 = temp.Trunc(70);
		*file << temp << F_ENDLINE;
	}
}

void Arules::PutNoFormat(const AString &s)
{
	*file << s << F_ENDLINE;
}

void Arules::EndLine()
{
	*file << F_ENDLINE;
}

void Arules::Enclose(int flag, const AString &tag)
{
	if (flag) {
		PutStr(AString("<") + tag + ">");
		AddTab();
	} else {
		DropTab();
		PutStr(AString("</")+ tag + ">");
	}
}

void Arules::TagText(const AString &tag, const AString &text)
{
	Enclose(1, tag);
	PutStr(text);
	Enclose(0, tag);
}

// LLS - converted HTML tags to lowercase
void Arules::ClassTagText(const AString &tag, const AString &cls,
		const AString &text)
{
	AString temp = tag;
	temp +=  " class=\"";
	temp += cls;
	temp += "\"";
	Enclose(1, temp);
	PutStr(text);
	Enclose(0, tag);
}

// LLS - converted HTML tags to lowercase
void Arules::Paragraph(const AString &text)
{
	Enclose(1, "p");
	PutStr(text);
	Enclose(0, "p");
}

// LLS - converted HTML tags to lowercase
void Arules::CommandExample(const AString &header, const AString &examp)
{
	Paragraph(header);
	Paragraph("");
	Enclose(1, "pre");
	PutNoFormat(examp);
	Enclose(0, "pre");
}

// LLS - converted HTML tags to lowercase
AString Arules::Link(const AString &href, const AString &text)
{
	return (AString("<a href=\"")+href+"\">"+text+"</a>");
}

// LLS - converted HTML tags to lowercase
void Arules::LinkRef(const AString &name)
{
	PutStr(AString("<a name=\"")+name+"\"></a>");
}
