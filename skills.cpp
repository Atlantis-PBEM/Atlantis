#include <stdlib.h>

#include "game.h"
#include "skills.h"
#include "items.h"
#include "gamedata.h"
#include "string_parser.hpp"

std::optional<std::reference_wrapper<RangeType>> find_range(strings::ci_string range)
{
    if (range.empty()) return std::nullopt;

    auto it = std::find_if(RangeDefs.begin(), RangeDefs.end(), [range](const RangeType& r) {
        return range == r.key;
    });
    if (it != RangeDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<SpecialType>> find_special(const strings::ci_string& key)
{
    if (key.empty()) return std::nullopt;
    auto it = std::find_if(SpecialDefs.begin(), SpecialDefs.end(), [key](const SpecialType& special) {
        return key == special.key;
    });
    if (it != SpecialDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<EffectType>> FindEffect(char const *effect)
{
    if (effect == NULL) return std::nullopt;
    strings::ci_string effect_ci(effect);

    auto it = std::find_if(EffectDefs.begin(), EffectDefs.end(), [effect_ci](const EffectType& effect) {
        return effect_ci == effect.name;
    });
    if (it != EffectDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<AttribModType>> FindAttrib(char const *attrib)
{
    if (attrib == NULL) return std::nullopt;
    strings::ci_string attrib_ci(attrib);

    auto it = std::find_if(AttribDefs.begin(), AttribDefs.end(), [attrib_ci](const AttribModType& attrib) {
        return attrib_ci == attrib.key;
    });
    if (it != AttribDefs.end()) return std::ref(*it);

    return std::nullopt;
}

std::optional<std::reference_wrapper<SkillType>> FindSkill(char const *skname)
{
    if (skname == nullptr) return std::nullopt;
    strings::ci_string skname_ci(skname);

    auto it = std::find_if(SkillDefs.begin(), SkillDefs.end(), [skname_ci](const SkillType& skill) {
        return skname_ci == skill.abbr;
    });

    if (it != SkillDefs.end()) return std::ref(*it);
    return std::nullopt;
}

int lookup_skill(const parser::token& token)
{
    for (int i=0; i<NSKILLS; i++) {
        if (token == SkillDefs[i].abbr) return i;
    }
    return -1;
}

int parse_skill(const parser::token& token)
{
    for (int i = 0; i < NSKILLS; i++) {
        if (SkillDefs[i].flags & SkillType::DISABLED) continue;
        if (token == SkillDefs[i].name || token == SkillDefs[i].abbr) {
            return i;
        }
    }
    return -1;
}

std::string SkillStrs(const SkillType& pS)
{
    return pS.name + " [" + pS.abbr + "]";
}

std::string SkillStrs(int i)
{
    return SkillDefs[i].name + " [" + SkillDefs[i].abbr + "]";
}

int SkillCost(int skill)
{
    return SkillDefs[skill].cost;
}

int SkillMax(char const *skill, int race)
{
    auto pS = FindSkill(skill);
    if (!Globals->MAGE_NONLEADERS) {
        if (pS && (pS->get().flags & SkillType::MAGIC)) {
            if (!(ItemDefs[race].type & IT_LEADER)) return 0;
        }
    }

    auto man_def = find_race(ItemDefs[race].abr);
    if (!man_def) return 0;
    auto mt = man_def->get();

    std::string skname = pS->get().abbr;
    std::string mani("MANI");
    for (unsigned int c=0; c < (sizeof(mt.skills)/sizeof(mt.skills[0])); c++) {
        if (skname == mt.skills[c])
            return mt.speciallevel;
        // Allow MANI to act as a placeholder for all magical skills
        if ((pS->get().flags & SkillType::MAGIC) && mani == mt.skills[c])
            return mt.speciallevel;
    }
    return mt.defaultlevel;
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
        rate += std::abs(diff) / 3;
    } else  {
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
            if ((clevel > level)    && (rate > 5)) {
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

    std::string temp;

    f >> std::ws >> temp;
    type = lookup_skill(temp);

    f >> days;

    exp = 0;
    if (Globals->REQUIRED_EXPERIENCE) f >> exp;
}

void Skill::Writeout(std::ostream& f) const
{
    if (type != -1) {
        f << SkillDefs[type].abbr << " " << days;
        if (Globals->REQUIRED_EXPERIENCE) f << " " << exp;
    } else {
        f << "NO_SKILL 0";
        if (Globals->REQUIRED_EXPERIENCE) f << " 0";
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
    for(const auto s: skills) {
        if (s->type == skill) {
            return s->days;
        }
    }
    return 0;
}

void SkillList::SetDays(int skill, int days)
{
    for(const auto s: skills) {
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
    for(const auto s: skills) {
        if (s->type == skill) {
            return s->exp;
        }
    }
    return 0;
}

void SkillList::SetExp(int skill, int exp)
{
    for(const auto s: skills) {
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
    for(const auto s: *b) {
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
    for(const auto s: skills) {
        if (s->type == skill) {
            days = s->days / nummen;
            if (Globals->REQUIRED_EXPERIENCE)
                exp = s->exp / nummen;
        }
    }

    return StudyRateAdjustment(days, exp);
}

std::string SkillList::report(int nummen)
{
    if (!size()) return "none";

    int i = 0;
    int displayed = 0;
    std::string temp;
    for(const auto s: skills) {
        if (s->days == 0) continue;
        displayed++;
        if (i) {
            temp += ", ";
        } else {
            i=1;
        }
        temp += SkillStrs(s->type);
        temp += " " + std::to_string(GetLevelByDays(s->days/nummen)) + " (" + std::to_string(s->days/nummen);
        if (Globals->REQUIRED_EXPERIENCE) {
            temp += "+" + std::to_string(GetStudyRate(s->type, nummen));
        }
        temp += ")";
    }
    if (!displayed) return "none";
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
    for(const auto s: skills) s->Writeout(f);
}
