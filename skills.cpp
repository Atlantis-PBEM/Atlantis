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
#include "skills.h"
#include "items.h"
#include "gamedata.h"

RangeType *FindRange(char *range)
{
    if (range == NULL) return NULL;
    for (int i = 0; i < NUMRANGES; i++) {
	        if (RangeDefs[i].key == NULL) continue;
	        if (AString(range) == RangeDefs[i].key)
	            return &RangeDefs[i];
	    }
    return NULL;
}

SpecialType *FindSpecial(char *key)
{
    if (key == NULL) return NULL;
    for (int i = 0; i < NUMSPECIALS; i++) {
	        if (SpecialDefs[i].key == NULL) continue;
	        if (AString(key) == SpecialDefs[i].key)
	            return &SpecialDefs[i];
	    }
    return NULL;
}

EffectType *FindEffect(char *effect)
{
    if (effect == NULL) return NULL;
    for (int i = 0; i < NUMEFFECTS; i++) {
	        if (EffectDefs[i].name == NULL) continue;
	        if (AString(effect) == EffectDefs[i].name)
	            return &EffectDefs[i];
	    }
    return NULL;
}

AttribModType *FindAttrib(char *attrib)
{
    if (attrib == NULL) return NULL;
    for (int i = 0; i < NUMATTRIBMODS; i++) {
	        if (AttribDefs[i].key == NULL) continue;
	        if (AString(attrib) == AttribDefs[i].key)
	            return &AttribDefs[i];
	    }
    return NULL;
}

int LookupSkill(AString *token)
{
	for (int i=0; i<NSKILLS; i++) {
		if (*token == SkillDefs[i].abbr) return i;
	}
	return -1;
}

int ParseSkill(AString *token)
{
	int r = -1;
	for (int i=0; i<NSKILLS; i++) {
		if ((*token == SkillDefs[i].name) || (*token == SkillDefs[i].abbr)) {
			r = i;
			break;
		}
	}
	if (r != -1) {
		if(SkillDefs[r].flags & SkillType::DISABLED) r = -1;
	}
	return r;
}

AString SkillStrs(int i)
{
	AString temp = AString(SkillDefs[i].name) + " [" +
		SkillDefs[i].abbr + "]";
	return temp;
}

int SkillCost(int skill)
{
	return SkillDefs[skill].cost;
}

int SkillMax(int skill, int race)
{
	ManType *mt = FindRace(ItemDefs[race].abr);

	if (mt == NULL) return 0;

	if(!Globals->MAGE_NONLEADERS) {
		if(SkillDefs[skill].flags & SkillType::MAGIC) {
			if(race != I_LEADERS) return(0);
		}
	}

	for(unsigned int c=0; c < sizeof(mt->skills)/sizeof(mt->skills[0]); c++) {
		if(mt->skills[c] == skill)
			return mt->speciallevel;
	}
	return mt->defaultlevel;
}

int GetLevelByDays(int dayspermen)
{
	int z = 30;
	int i = 0;
	while (dayspermen >= z) {
		i++;
		dayspermen -= z;
		z += 30;
	}
	return i;
}

int GetDaysByLevel(int level)
{
	int days = 0;

	for (;level>0; level--) {
		days += level * 30;
	}

	return days;
}

ShowSkill::ShowSkill(int s, int l)
{
	skill = s;
	level = l;
}

void Skill::Readin(Ainfile *f)
{
	AString *temp, *token;

	temp = f->GetStr();
	token = temp->gettoken();
	type = LookupSkill(token);
	delete token;

	token = temp->gettoken();
	days = token->value();
	delete token;
	delete temp;
}

void Skill::Writeout(Aoutfile *f)
{
	AString temp;

	if (type != -1) {
		temp = AString(SkillDefs[type].abbr) + " " + days;
	} else
		temp = AString("NO_SKILL 0");
	f->PutStr(temp);
}

Skill *Skill::Split(int total, int leave)
{
	Skill *temp = new Skill;
	temp->type = type;
	temp->days = (days * leave) / total;
	days = days - temp->days;
	return temp;
}

int SkillList::GetDays(int skill)
{
	forlist(this) {
		Skill *s = (Skill *) elem;
		if (s->type == skill) {
			return s->days;
		}
	}
	return 0;
}

void SkillList::SetDays(int skill, int days)
{
	forlist(this) {
		Skill *s = (Skill *) elem;
		if (s->type == skill) {
			if (days == 0) {
				Remove(s);
				delete s;
				return;
			} else {
				s->days = days;
				return;
			}
		}
	}
	if (days == 0) return;
	Skill *s = new Skill;
	s->type = skill;
	s->days = days;
	Add(s);
}

SkillList *SkillList::Split(int total, int leave)
{
	SkillList *ret = new SkillList;
	forlist (this) {
		Skill *s = (Skill *) elem;
		Skill *n = s->Split(total, leave);
		if (s->days == 0) {
			Remove(s);
			delete s;
		}
		ret->Add(n);
	}
	return ret;
}

void SkillList::Combine(SkillList *b)
{
	forlist(b) {
		Skill *s = (Skill *) elem;
		SetDays(s->type, GetDays(s->type) + s->days);
	}
}

AString SkillList::Report(int nummen)
{
	AString temp;
	if (!Num()) {
		temp += "none";
		return temp;
	}
	int i = 0;
	forlist (this) {
		Skill *s = (Skill *) elem;
		if (i) {
			temp += ", ";
		} else {
			i=1;
		}
		temp += SkillStrs(s->type);
		temp += AString(" ") + GetLevelByDays(s->days/nummen) +
			AString(" (") + AString(s->days/nummen) + AString(")");
	}
	return temp;
}

void SkillList::Readin(Ainfile *f)
{
	int n = f->GetInt();
	for (int i=0; i<n; i++) {
		Skill *s = new Skill;
		s->Readin(f);
		if (s->days == 0) delete s;
		else Add(s);
	}
}

void SkillList::Writeout(Aoutfile *f)
{
	f->PutInt(Num());
	forlist(this) ((Skill *) elem)->Writeout(f);
}
