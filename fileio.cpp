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
#include <iostream.h>
#include <fstream.h>

#define F_ENDLINE '\n'

extern long _ftype,_fcreator;

static char buf[1024];

Aoutfile::Aoutfile() {
	file = new ofstream;
}

Aoutfile::~Aoutfile() {
	delete file;
}

Ainfile::Ainfile() {
	file = new ifstream;
}

Ainfile::~Ainfile() {
	delete file;
}

Aorders::Aorders() {
	file = new ifstream;
}

Aorders::~Aorders() {
	delete file;
}

Areport::Areport() {
	file = new ofstream;
}

Areport::~Areport() {
	delete file;
}

void Aoutfile::Open(const AString & s) {
  while (! (file->rdbuf()->is_open())) {
    AString * name = getfilename(s);
    file->open(name->Str(),ios::noreplace | ios::out);
    delete name;
  }
}

int Aoutfile::OpenByName(const AString & s) {
  AString temp = s;
  file->open(temp.Str(),ios::noreplace | ios::out);
  if (!file->rdbuf()->is_open()) return -1;
  return 0;
}

void Ainfile::Open(const AString & s) {
  while (! (file->rdbuf()->is_open())) {
    AString * name = getfilename(s);
    file->open(name->Str(),ios::in | ios::nocreate);
    delete name;
  }
}

int Ainfile::OpenByName(const AString & s) {
  AString temp = s;
  file->open(temp.Str(),ios::in | ios::nocreate);
  if (! (file->rdbuf()->is_open())) return -1;
  return 0;
}

void Aoutfile::Close() {
	file->close();
}

void Ainfile::Close() {
	file->close();
}

void Aorders::Close() {
	file->close();
}

void Areport::Close() {
	file->close();
}

void skipwhite(ifstream * f) {
  if (f->eof()) return;
  int ch = f->peek();
  while(((ch == ' ') || (ch == '\n') || (ch == '\t')
	 || (ch == '\r') || (ch == '\0'))) {
    f->get();
    if (f->eof()) return;
    ch = f->peek();
  }
}

AString * Ainfile::GetStr() {
  skipwhite(file);
  if (file->peek() == -1 || file->eof()) return 0;
  file->getline(buf,1023,F_ENDLINE);
  AString * s = new AString((char *) &(buf[0]));
  return s;
}

AString * Ainfile::GetStrNoSkip() {
  if (file->peek() == -1 || file->eof()) return 0;
  file->getline(buf,1023,F_ENDLINE);
  AString * s = new AString((char *) &(buf[0]));
  return s;
}

int Ainfile::GetInt() {
  int x;
  *file >> x;
  return x;
}

void Aoutfile::PutInt(int x) {
  *file << x;
  *file << F_ENDLINE;
}

void Aoutfile::PutStr(char * s) {
  *file << s << F_ENDLINE;
}

void Aoutfile::PutStr(const AString & s) {
  *file << s << F_ENDLINE;
}

void Aorders::Open(const AString & s)
{
    while (! (file->rdbuf()->is_open()))
    {
        AString * name = getfilename(s);
        file->open(name->Str(),ios::in | ios::nocreate);
        delete name;
    }
}

int Aorders::OpenByName(const AString & s)
{
    AString temp = s;
    file->open(temp.Str(),ios::in | ios::nocreate);
    if (! (file->rdbuf()->is_open())) return -1;
    return 0;
}

AString * Aorders::GetLine() {
  skipwhite(file);
  if (file->eof()) return 0;
  if (file->peek() == -1) return 0;
  file->getline(buf,1023,F_ENDLINE);
  AString * s = new AString((char *) &(buf[0]));
  return s;
}

void Areport::Open(const AString & s)
{
    while (! (file->rdbuf()->is_open()))
    {
        AString * name = getfilename(s);
        file->open(name->Str(),ios::noreplace | ios::out);
        delete name;
    }
    tabs = 0;
}

int Areport::OpenByName(const AString & s)
{
    AString temp = s;
    file->open(temp.Str(),ios::noreplace | ios::out);
    tabs = 0;
    if (!file->rdbuf()->is_open()) return -1;
    return 0;
}

void Areport::AddTab() {
  tabs++;
}

void Areport::DropTab() {
  if (tabs > 0) tabs--;
}

void Areport::ClearTab() {
  tabs = 0;
}

void Areport::PutStr(const AString & s,int comment)
{
    AString temp;
    for (int i=0; i<tabs; i++) temp += "  ";
    temp += s;
    AString * temp2 = temp.Trunc(70);
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

void Areport::PutNoFormat(const AString & s) {
  *file << s << F_ENDLINE;
}

void Areport::EndLine() {
  *file << F_ENDLINE;
}




