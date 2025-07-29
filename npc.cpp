#include "game.h"
#include "gamedata.h"
#include "rng.hpp"
#include <numeric>
#include <limits>

void Game::CreateCityMons()
{
    if (!Globals->CITY_MONSTERS_EXIST) return;

    for(const auto r : regions) {
        if ((r->type == R_NEXUS) || r->IsStartingCity() || r->town) {
            CreateCityMon(r, 100, 1);
        }
    }
}

void Game::CreateWMons()
{
    if (!Globals->WANDERING_MONSTERS_EXIST) return;

    GrowWMons(50);
}

void Game::CreateLMons()
{
    if (!Globals->LAIR_MONSTERS_EXIST) return;

    GrowLMons(50);
}

void Game::GrowWMons(int rate)
{
    //
    // Now, go through each 8x8 block of the map, and make monsters if
    // needed.
    //
    int level;
    for (level = 0; level < regions.numLevels; level++) {
        ARegionArray *pArr = regions.pRegionArrays[level];

        for (int xsec=0; xsec < pArr->x; xsec+=8) {
            for (int ysec=0; ysec < pArr->y; ysec+=16) {
                int mons=0;
                int wanted=0;

                for (int x=0; x < 8; x++) {
                    if (x+xsec > pArr->x) break;

                    for (int y=0; y < 16; y+=2) {
                        if (y+ysec > pArr->y) break;

                        ARegion *reg = pArr->GetRegion(x+xsec, y+ysec+x%2);
                        if (reg && reg->zloc == level && !reg->IsGuarded()) {
                            mons += reg->CountWMons();
                            /*
                             * Make sure there is at least one monster type
                             * enabled for this region
                             */
                            int avail = 0;
                            int mon = TerrainDefs[reg->type].smallmon;
                            if (!((mon == -1) ||
                                 (ItemDefs[mon].flags & ItemType::DISABLED)))
                                avail = 1;
                            mon = TerrainDefs[reg->type].bigmon;
                            if (!((mon == -1) ||
                                 (ItemDefs[mon].flags & ItemType::DISABLED)))
                                avail = 1;
                            mon = TerrainDefs[reg->type].humanoid;
                            if (!((mon == -1) ||
                                 (ItemDefs[mon].flags & ItemType::DISABLED)))
                                avail = 1;

                            if (avail) {
                                wanted += TerrainDefs[reg->type].wmonfreq;
                            }
                        }
                    }
                }

                wanted /= 10;
                wanted -= mons;
                wanted = (wanted*rate + rng::get_random(100))/100;

                // printf("\n\n WANTED WMON at (xsec: %d, ysec: %d) : %d \n\n", xsec, ysec, wanted);

                if (wanted > 0) {
                    // TODO: instead of loop guard need to check how many available regions
                    // are there and random them
                    int loop_guard = 1000;
                    for (int i=0; i < wanted;) {
                        int x = rng::get_random(8);
                        int y = rng::get_random(16);
                        if (y%2 == 1) {
                            if (y > 0) {
                                y -= 1;
                            } else {
                                y = 0;
                            }
                        }

                        ARegion *reg = pArr->GetRegion(x + xsec, y + ysec + x%2);

                        if (reg && reg->zloc == level && !reg->IsGuarded() && MakeWMon(reg)) {
                            i++;
                        }

                        // In worst case scenario it will randomly pick same not matching regions
                        // with potential of infinitie loop (ie dodgy RNG)
                        loop_guard--;
                        if (loop_guard == 0) break;
                    }
                }
            }
        }
    }
}

void Game::GrowLMons(int rate)
{
    for(const auto r : regions) {
        //
        // Don't make lmons in guarded regions
        //
        if (r->IsGuarded()) continue;

        for(const auto obj : r->objects) {
            if (obj->units.size()) continue;
            int montype = ObjectDefs[obj->type].monster;
            int grow=!(ObjectDefs[obj->type].flags&ObjectType::NOMONSTERGROWTH);
            if ((montype != -1) && grow) {
                if (rng::get_random(100) < rate) {
                    MakeLMon(obj);
                }
            }
        }
    }
}

int Game::MakeWMon(ARegion *pReg)
{
    if (!Globals->WANDERING_MONSTERS_EXIST) return 0;

    if (TerrainDefs[pReg->type].wmonfreq == 0) return 0;

    int montype = TerrainDefs[pReg->type].smallmon;
    if (rng::get_random(2) && (TerrainDefs[pReg->type].humanoid != -1))
        montype = TerrainDefs[pReg->type].humanoid;
    if (TerrainDefs[pReg->type].bigmon != -1 && !rng::get_random(8)) {
        montype = TerrainDefs[pReg->type].bigmon;
    }
    if ((montype == -1) || (ItemDefs[montype].flags & ItemType::DISABLED))
        return 0;

    auto monster = find_monster(ItemDefs[montype].abr, (ItemDefs[montype].type & IT_ILLUSION))->get();
    Faction *monfac = GetFaction(factions, monfaction);
    Unit *u = GetNewUnit(monfac, 0);
    u->MakeWMon(monster.name.c_str(), montype, (monster.number+rng::get_random(monster.number)+1)/2);
    u->MoveUnit(pReg->GetDummy());
    return(1);
}

void Game::MakeLMon(Object *pObj)
{
    if (!Globals->LAIR_MONSTERS_EXIST) return;
    if (ObjectDefs[pObj->type].flags & ObjectType::NOMONSTERGROWTH) return;

    int montype = ObjectDefs[pObj->type].monster;

    if (montype == I_TRENT)
        montype = TerrainDefs[pObj->region->type].bigmon;

    if (montype == I_CENTAUR)
        montype = TerrainDefs[pObj->region->type].humanoid;

    if ((montype == -1) || (ItemDefs[montype].flags & ItemType::DISABLED))
        return;

    auto monster = find_monster(ItemDefs[montype].abr, (ItemDefs[montype].type & IT_ILLUSION))->get();
    Faction *monfac = GetFaction(factions, monfaction);
    Unit *u = GetNewUnit(monfac, 0);
    switch(montype) {
        case I_IMP:
            u->MakeWMon("Demons", I_IMP, rng::get_random(monster.number + 1));

            monster = find_monster(ItemDefs[I_DEMON].abr, (ItemDefs[I_DEMON].type & IT_ILLUSION))->get();
            u->items.SetNum(I_DEMON, rng::get_random(monster.number + 1));

            monster = find_monster(ItemDefs[I_DEVIL].abr, (ItemDefs[I_DEVIL].type & IT_ILLUSION))->get();
            u->items.SetNum(I_DEVIL, rng::get_random(monster.number + 1));
            break;
        case I_SKELETON:
            u->MakeWMon("Undead", I_SKELETON, rng::get_random(monster.number + 1));

            monster = find_monster(ItemDefs[I_UNDEAD].abr, (ItemDefs[I_UNDEAD].type & IT_ILLUSION))->get();
            u->items.SetNum(I_UNDEAD, rng::get_random(monster.number + 1));

            monster = find_monster(ItemDefs[I_LICH].abr, (ItemDefs[I_LICH].type & IT_ILLUSION))->get();
            u->items.SetNum(I_LICH, rng::get_random(monster.number + 1));
            break;
        case I_MAGICIANS:
            u->MakeWMon("Evil Mages", I_MAGICIANS, (monster.number + rng::get_random(monster.number) + 1) / 2);

            monster = find_monster(ItemDefs[I_SORCERERS].abr, (ItemDefs[I_SORCERERS].type & IT_ILLUSION))->get();
            u->items.SetNum(I_SORCERERS, rng::get_random(monster.number + 1));
            u->SetFlag(FLAG_BEHIND, 1);
            u->guard = GUARD_NONE;
            u->MoveUnit(pObj);

            u = GetNewUnit(monfac, 0);

            monster = find_monster(ItemDefs[I_WARRIORS].abr, (ItemDefs[I_WARRIORS].type & IT_ILLUSION))->get();
            u->MakeWMon(monster.name.c_str(), I_WARRIORS, (monster.number + rng::get_random(monster.number) + 1) / 2);
            u->guard = GUARD_NONE;

            break;
        case I_DARKMAGE:
            u->MakeWMon("Dark Mages", I_DARKMAGE, (rng::get_random(monster.number) + 1));

            monster = find_monster(ItemDefs[I_MAGICIANS].abr, (ItemDefs[I_MAGICIANS].type & IT_ILLUSION))->get();
            u->items.SetNum(I_MAGICIANS, (monster.number + rng::get_random(monster.number) + 1) / 2);

            monster = find_monster(ItemDefs[I_SORCERERS].abr, (ItemDefs[I_SORCERERS].type & IT_ILLUSION))->get();
            u->items.SetNum(I_SORCERERS, rng::get_random(monster.number + 1));

            monster = find_monster(ItemDefs[I_DARKMAGE].abr, (ItemDefs[I_DARKMAGE].type & IT_ILLUSION))->get();
            u->items.SetNum(I_DARKMAGE, rng::get_random(monster.number + 1));
            u->SetFlag(FLAG_BEHIND, 1);
            u->guard = GUARD_NONE;
            u->MoveUnit(pObj);

            u = GetNewUnit(monfac, 0);

            monster = find_monster(ItemDefs[I_DROW].abr, (ItemDefs[I_DROW].type & IT_ILLUSION))->get();
            u->MakeWMon(monster.name.c_str(), I_DROW, (monster.number + rng::get_random(monster.number) + 1) / 2);
            u->guard = GUARD_NONE;

            break;
        case I_ILLYRTHID:
            u->MakeWMon(monster.name.c_str(), I_ILLYRTHID, (monster.number + rng::get_random(monster.number) + 1) / 2);
            u->SetFlag(FLAG_BEHIND, 1);
            u->guard = GUARD_NONE;
            u->MoveUnit(pObj);

            u = GetNewUnit(monfac, 0);

            monster = find_monster(ItemDefs[I_SKELETON].abr, (ItemDefs[I_SKELETON].type & IT_ILLUSION))->get();
            u->MakeWMon("Undead", I_SKELETON, rng::get_random(monster.number + 1));

            monster = find_monster(ItemDefs[I_UNDEAD].abr, (ItemDefs[I_UNDEAD].type & IT_ILLUSION))->get();
            u->items.SetNum(I_UNDEAD, rng::get_random(monster.number + 1));
            u->guard = GUARD_NONE;
            break;
        case I_STORMGIANT:
            if (rng::get_random(3) < 1) {
                montype = I_CLOUDGIANT;
                monster = find_monster(ItemDefs[montype].abr, (ItemDefs[montype].type & IT_ILLUSION))->get();
            }
            u->MakeWMon(monster.name.c_str(), montype, (monster.number + rng::get_random(monster.number) + 1) / 2);
            break;
        default:
            u->MakeWMon(monster.name.c_str(), montype, (monster.number + rng::get_random(monster.number) + 1) / 2);
            break;
    }
    u->MoveUnit(pObj);
}

// Helper struct for weapon selection
struct SuitableWeapon {
    int index; // Index into WeaponDefs
    unsigned int weight;
};

Unit *Game::MakeManUnit(Faction *fac, int mantype, int num, int level, int weaponlevel, int armor, int behind)
{
    Unit *u = GetNewUnit(fac);
    auto men = find_race(ItemDefs[mantype].abr)->get();

    int scomb = men.defaultlevel;
    int sxbow = men.defaultlevel;
    int slbow = men.defaultlevel;
    for (unsigned int i = 0; i < (sizeof(men.skills) / sizeof(men.skills[0])); i++) {
        if (!men.skills[i]) continue;
        auto pS = FindSkill(men.skills[i]->c_str())->get();
        if (pS == FindSkill("COMB")->get()) scomb = men.speciallevel;
        if (pS == FindSkill("XBOW")->get()) sxbow = men.speciallevel;
        if (pS == FindSkill("LBOW")->get()) slbow = men.speciallevel;
    }

    int combat_level = scomb;
    int sk = lookup_skill("COMB");
    if (behind) {
        if (slbow >= sxbow) {
            sk = lookup_skill("LBOW");
            combat_level = slbow;
        } else {
            sk = lookup_skill("XBOW");
            combat_level = sxbow;
        }
    }

    if (combat_level < level) {
        weaponlevel += level - combat_level;
    }

    int weapon_index = -1;
    int witem = -1;

    while (weapon_index == -1) {
        std::vector<SuitableWeapon> suitable_weapons;

        for (size_t i = 0; i < WeaponDefs.size(); ++i) {
            int current_witem = lookup_item(WeaponDefs[i].abbr);

            if (ItemDefs[current_witem].flags & ItemType::DISABLED) continue;
            if (current_witem == lookup_item("PICK")) continue;
            if (ItemDefs[current_witem].pSkill && ItemDefs[current_witem].pSkill != FindSkill("WEAP")->get().abbr) continue;

            bool is_ranged = (WeaponDefs[i].flags & WeaponType::RANGED);
            if (is_ranged && !behind) continue;

            int weapon_base_skill_idx = lookup_skill(WeaponDefs[i].baseSkill);
            int weapon_or_skill_idx = lookup_skill(WeaponDefs[i].orSkill);
            bool skill_match = (weapon_base_skill_idx == sk || weapon_or_skill_idx == sk);

            bool javelin_case = false;
            if (behind && !skill_match && scomb > combat_level) {
                if (is_ranged && (weapon_base_skill_idx == lookup_skill("COMB") || weapon_or_skill_idx == lookup_skill("COMB"))) {
                    skill_match = true;
                    javelin_case = true;
                }
            }

            if (!skill_match) continue;

            int attack = WeaponDefs[i].attackBonus;
            int producelevel = ItemDefs[current_witem].pLevel;
            if (attack < (producelevel - 1)) attack = producelevel - 1;

            bool level_match = false;
            unsigned int weight = 1;
            if (behind) {
                if (attack + (javelin_case ? scomb : combat_level) <= weaponlevel) {
                    level_match = true;
                    if (WeaponDefs[i].attackBonus == weaponlevel) {
                        weight = 5;
                    }
                }
            } else {
                if (attack == weaponlevel) {
                    level_match = true;
                }
            }

            if (!level_match) continue;

            if (!men.CanUse(current_witem)) continue;

            suitable_weapons.push_back({static_cast<int>(i), weight});
        }

        if (suitable_weapons.empty()) {
            weaponlevel++;
            continue;
        }

        std::vector<unsigned int> weights;
        weights.reserve(suitable_weapons.size());
        for (const auto& sw : suitable_weapons) {
            weights.push_back(sw.weight);
        }

        std::optional<size_t> selected_suitable_index_opt = rng::get_weighted_index(weights);

        // If the weighted selection failed, we know that suitable_weapons is not empty, so we can safely pick the first one.
        if (selected_suitable_index_opt) weapon_index = suitable_weapons[*selected_suitable_index_opt].index;
        else weapon_index = suitable_weapons[0].index;
        witem = lookup_item(WeaponDefs[weapon_index].abbr);
    }

    int final_skill_idx = lookup_skill(WeaponDefs[weapon_index].baseSkill);
    if (final_skill_idx != sk && lookup_skill(WeaponDefs[weapon_index].orSkill) != sk) sk = final_skill_idx;

    int maxskill = men.defaultlevel;
    for (unsigned int i = 0; i < (sizeof(men.skills) / sizeof(men.skills[0])); i++) {
        if (men.skills[i] && FindSkill(men.skills[i]->c_str())->get() == FindSkill(SkillDefs[sk].abbr.c_str())->get()) {
            maxskill = men.speciallevel;
            break;
        }
    }

    if (level > maxskill) level = maxskill;

    u->SetMen(mantype, num);
    u->items.SetNum(witem, num);
    u->SetSkill(sk, level);
    if (behind) u->SetFlag(FLAG_BEHIND, 1);

    if (armor) {
        int ar = I_PLATEARMOR;
        if (!men.CanUse(ar)) ar = I_CHAINARMOR;
        if (!men.CanUse(ar)) ar = I_LEATHERARMOR;
        if (men.CanUse(ar)) u->items.SetNum(ar, num);
    }

    return u;
}
