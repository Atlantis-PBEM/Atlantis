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

#include <stdlib.h>

#include "game.h"
#include "skills.h"
#include "items.h"
#include "gamedata.h"

RangeType *FindRange(char const *range)
{
	if (range == NULL) return NULL;
	for (int i = 0; i < NUMRANGES; i++) {
		if (RangeDefs[i].key == NULL) continue;
		if (AString(range) == RangeDefs[i].key)
			return &RangeDefs[i];
	}
	return NULL;
}

SpecialType *FindSpecial(char const *key)
{
	if (key == NULL) return NULL;
	for (int i = 0; i < NUMSPECIALS; i++) {
		if (SpecialDefs[i].key == NULL) continue;
		if (AString(key) == SpecialDefs[i].key)
			return &SpecialDefs[i];
	}
	return NULL;
}

EffectType *FindEffect(char const *effect)
{
	if (effect == NULL) return NULL;
	for (int i = 0; i < NUMEFFECTS; i++) {
		if (EffectDefs[i].name == NULL) continue;
		if (AString(effect) == EffectDefs[i].name)
			return &EffectDefs[i];
	}
	return NULL;
}

AttribModType *FindAttrib(char const *attrib)
{
	if (attrib == NULL) return NULL;
	for (int i = 0; i < NUMATTRIBMODS; i++) {
		if (AttribDefs[i].key == NULL) continue;
		if (AString(attrib) == AttribDefs[i].key)
			return &AttribDefs[i];
	}
	return NULL;
}

SkillType *FindSkill(char const *skname)
{
	if (skname == NULL) return NULL;
	for (int i = 0; i < NSKILLS; i++) {
		if (SkillDefs[i].abbr == NULL) continue;
		if (AString(skname) == SkillDefs[i].abbr)
			return &SkillDefs[i];
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
		if (SkillDefs[r].flags & SkillType::DISABLED) r = -1;
	}
	return r;
}

AString SkillStrs(SkillType *pS)
{
	AString temp = AString(pS->name) + " [" + pS->abbr + "]";
	return temp;
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

int SkillMax(char const *skill, int race)
{
	ManType *mt = FindRace(ItemDefs[race].abr);

	if (mt == NULL) return 0;

	SkillType *pS = FindSkill(skill);
	if (!Globals->MAGE_NONLEADERS) {
		if (pS && (pS->flags & SkillType::MAGIC)) {
			if (!(ItemDefs[race].type & IT_LEADER)) return(0);
		}
	}

	AString skname = pS->abbr;
	AString mani = "MANI";
	for (unsigned int c=0; c < sizeof(mt->skills)/sizeof(mt->skills[0]); c++) {
		if (skname == mt->skills[c])
			return mt->speciallevel;
		// Allow MANI to act as a placeholder for all magical skills
		if ((pS->flags & SkillType::MAGIC) && mani == mt->skills[c])
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

/* Returns the adjusted study rate,
 */
int StudyRateAdjustment(int days, int exp)
{
	int rate = 30;
	if (!Globals->REQUIRED_EXPERIENCE) return rate;
	int slope = 62;
	int inc = Globals->REQUIRED_EXPERIENCE * 10;
	long int cdays = inc;
	int prevd = 0;
	int diff = days - exp;
	if (diff <= 0) {
		rate += abs(diff) / 3;
	} else 	{
		int level = 0;
		long int ctr = 0;
		while((((cdays + ctr) / slope + prevd) <= diff)
			&& (rate > 0)) {
			rate -= 1;
			if (rate <= 5) {
				prevd += cdays / slope;
				ctr += cdays;
				cdays = 0;
				slope = (slope * 2)/3;
			}
			cdays += inc;
			int clevel = GetLevelByDays(cdays/slope);
			if ((clevel > level)	&& (rate > 5)) {
				level = clevel;
				switch(level) {
					case 1: slope = 80;
						prevd += cdays /slope;
						ctr += cdays;
						cdays = 0;
						break;
					case 2: slope = 125;
						prevd += cdays / slope;
						ctr += cdays;
						cdays = 0;
						break;
				}
			}
		}
	}
	return rate;
}

void Skill::Readin(std::istream &f)
{
	AString temp, *token;

	f >> std::ws >> temp;
	token = temp.gettoken();
	type = LookupSkill(token);
	delete token;

	token = temp.gettoken();
	days = token->value();
	delete token;

	exp = 0;
	if (Globals->REQUIRED_EXPERIENCE) {
		token = temp.gettoken();
		exp = token->value();
		delete token;
	}
}

void Skill::Writeout(std::ostream& f)
{
	if (type != -1) {
		f << SkillDefs[type].abbr << " " << days;
		if (Globals->REQUIRED_EXPERIENCE) {
			f << " " << exp;
		}
	} else {
		f << "NO_SKILL 0";
		if (Globals->REQUIRED_EXPERIENCE) {
			f << " 0";
		}
	}
	f << '\n';
}

Skill *Skill::Split(int total, int leave)
{
	Skill *temp = new Skill;
	temp->type = type;
	temp->days = (days * leave) / total;
	days = days - temp->days;
	temp->exp = (exp * leave) / total;
	exp = exp - temp->exp;
	return temp;
}

int SkillList::GetDays(int skill)
{
	for(auto s: skills) {
		if (s->type == skill) {
			return s->days;
		}
	}
	return 0;
}

void SkillList::SetDays(int skill, int days)
{
	for(auto s: skills) {
		if (s->type == skill) {
			if ((days == 0) && (s->exp <= 0)) {
				std::erase(skills, s); // this is safe because we are not continuing to use the list iterator
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
	s->exp = 0;
	skills.push_back(s);
}

int SkillList::GetExp(int skill)
{
	for(auto s: skills) {
		if (s->type == skill) {
			return s->exp;
		}
	}
	return 0;
}

void SkillList::SetExp(int skill, int exp)
{
	for(auto s: skills) {
		if (s->type == skill) {
			s->exp = exp;
			return;
		}
	}
	if (exp == 0) return;
	Skill *s = new Skill;
	s->type = skill;
	s->days = 0;
	s->exp = exp;
	skills.push_back(s);
}

SkillList *SkillList::Split(int total, int leave)
{
	SkillList *ret = new SkillList;
	for(auto siter = skills.begin(); siter != skills.end();) {
		Skill *s = *siter;
		Skill *n = s->Split(total, leave);
		if ((s->days == 0) && (s->exp == 0)) {
			siter = skills.erase(siter);
			delete s;
		} else {
			++siter;
		}
		ret->skills.push_back(n);
	}
	return ret;
}

void SkillList::Combine(SkillList *b)
{
	for(auto s: *b) {
		SetDays(s->type, GetDays(s->type) + s->days);
		SetExp(s->type, GetExp(s->type) + s->exp);
	}
}

/* Returns the rate of study (days/month and man)
 * for studying a skill
 */
int SkillList::GetStudyRate(int skill, int nummen)
{
	int days = 0;
	int exp = 0;
	if (nummen < 1) return 0;
	for(auto s: skills) {
		if (s->type == skill) {
			days = s->days / nummen;
			if (Globals->REQUIRED_EXPERIENCE)
				exp = s->exp / nummen;
		}
	}

	return StudyRateAdjustment(days, exp);
}

AString SkillList::Report(int nummen)
{
	AString temp;
	if (!size()) {
		temp += "none";
		return temp;
	}
	int i = 0;
	int displayed = 0;
	for(auto s: skills) {
		if (s->days == 0) continue;
		displayed++;
		if (i) {
			temp += ", ";
		} else {
			i=1;
		}
		temp += SkillStrs(s->type);
		temp += AString(" ") + GetLevelByDays(s->days/nummen) +
			AString(" (") + AString(s->days/nummen);
		if (Globals->REQUIRED_EXPERIENCE) {
			temp += AString("+") + AString(GetStudyRate(s->type, nummen));
		}
		temp += AString(")");
	}
	if (!displayed) temp += "none";
	return temp;
}

void SkillList::Readin(std::istream& f)
{
	int n;
	f >> n;
	for (int i = 0; i < n; i++) {
		Skill *s = new Skill;
		s->Readin(f);
		if ((s->days == 0) && (s->exp==0)) delete s;
		else skills.push_back(s);
	}
}

void SkillList::Writeout(std::ostream& f)
{
	f << size() << '\n';
	for(auto s: skills) s->Writeout(f);
}
