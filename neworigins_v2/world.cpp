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
// MODIFICATIONS
// Date        Person            Comments
// ----        ------            --------
// 2000/SEP/06 Joseph Traub      Added base man cost to allow races to have
//                               different base costs
#include "game.h"
#include "gamedata.h"
#include <string.h>

typedef struct
{
	char const	*word;
	int		prob;
} WordList;

// Initial Consonant, Vowel and Final Consonant sequences and
// probabilities were derived by analysis of a convenient unix
// (English) dictionary

WordList ic[] =
{
{ "c", 1804 }, { "m", 1418 }, { "p", 1282 }, { "b", 1272 }, { "s", 1254 },
{ "d", 1251 }, { "h", 1115 }, { "r", 918 }, { "l", 901 }, { "t", 773 },
{ "f", 699 }, { "g", 596 }, { "w", 543 }, { "n", 518 }, { "v", 415 },
{ "pr", 400 }, { "st", 333 }, { "tr", 326 }, { "ch", 309 }, { "j", 300 },
{ "br", 255 }, { "k", 240 }, { "sh", 228 }, { "gr", 216 }, { "cr", 214 },
{ "sp", 183 }, { "th", 171 }, { "cl", 165 }, { "fr", 164 }, { "fl", 160 },
{ "pl", 151 }, { "bl", 147 }, { "qu", 136 }, { "wh", 117 }, { "sc", 111 },
{ "dr", 105 }, { "sl", 104 }, { "y", 100 }, { "gl", 97 }, { "sw", 95 },
{ "ph", 95 }, { "str", 89 }, { "sn", 62 }, { "z", 60 },
{ "sk", 54 }, { "scr", 51 }, { "sch", 46 }, { "kn", 45 }, { "sm", 43 },
{ "wr", 41 }, { "thr", 39 }, { "chr", 38 }, { "squ", 36 }, { "rh", 34 },
{ "tw", 33 }, { "ps", 33 }, { "shr", 26 }, { "spr", 25 }, { "kr", 19 },
{ "spl", 17 }, { "my", 11 }, { "x", 10 }, { "gn", 10 }
};

WordList v[] =
{
{ "e", 1681 }, { "a", 1356 }, { "i", 1045 }, { "o", 931 }, { "u", 410 },
{ "y", 189 }, { "ia", 93 }, { "ea", 86 }, { "io", 82 }, { "ou", 78 },
{ "oo", 51 }, { "ee", 49 }, { "ai", 47 }, { "ie", 46 }, { "au", 34 },
{ "oa", 25 }, { "oi", 22 }, { "ei", 22 }, { "eo", 17 }, { "ue", 15 },
{ "eu", 15 }, { "iu", 15 }, { "iou", 15 }, { "ua", 14 }, { "ui", 12 },
{ "oe", 12 }, { "ae", 6 }
};

WordList fc[] =
{
{ "n", 2642 }, { "r", 1496 }, { "l", 1254 }, { "s", 1064 }, { "t", 763 },
{ "c", 760 }, { "nt", 685 }, { "d", 583 }, { "m", 538 }, { "ng", 389 },
{ "y", 370 }, { "nd", 300 }, { "st", 278 }, { "p", 259 }, { "sh", 252 },
{ "rd", 249 }, { "ck", 244 }, { "ll", 229 }, { "w", 193 }, { "ss", 170 },
{ "rt", 157 }, { "ld", 134 }, { "x", 128 }, { "g", 127 }, { "th", 125 },
{ "ct", 125 }, { "k", 114 }, { "rn", 97 }, { "ght", 93 }, { "sm", 90 },
{ "b", 86 }, { "rk", 84 }, { "ch", 81 }, { "nk", 76 }, { "ff", 73 },
{ "rm", 66 }, { "wn", 65 }, { "lt", 64 }, { "tt", 58 }, { "tch", 54 },
{ "f", 49 }, { "h", 47 }, { "mp", 46 }, { "rg", 44 }, { "ft", 44 },
{ "pt", 41 }, { "gh", 40 }, { "nch", 39 }, { "ns", 37 }, { "ph", 29 },
{ "lk", 29 }, { "z", 28 }, { "rth", 26 }, { "sk", 23 }, { "wl", 22 },
{ "rs", 22 }, { "nn", 22 }, { "mb", 22 }, { "rch", 21 }, { "lm", 20 },
{ "tz", 19 }, { "rl", 19 }, { "nth", 19 }, { "lf", 19 }, { "v", 18 },
{ "rb", 18 }, { "gn", 16 }, { "rst", 14 }, { "nct", 13 }, { "rp", 12 },
{ "sp", 11 }, { "rr", 11 }
};

typedef struct
{
	int		terrain;
	char const	*word;
	int		prob;
	int		town;
	int		port;
} SuffixList;

SuffixList ts[] =
{
{ -1,		"acre",		 3, 0, 0 },
{ -1,		"bach",		 3, 0, 0 },
{ -2,		"bank",		10, 0, 1 },
{ -2,		"bay",		10, 0, 1 },
{ R_MOUNTAIN,	"berg",		10, 0, 0 },
{ -2,		"borough",	 4, 1, 0 },
{ R_PLAIN,	"bost",		10, 0, 0 },
{ -1,		"brook",	 3, 0, 0 },
{ -2,		"bruk",		 4, 1, 0 },
{ -2,		"burg",		 4, 1, 0 },
{ -1,		"burn",		 3, 0, 0 },
{ -2,		"bury",		 4, 1, 0 },
{ -2,		"by",		 4, 1, 0 },
{ R_FOREST,	"cot",		10, 0, 0 },
{ -2,		"dale",		 4, 1, 0 },
{ R_JUNGLE,	"dee",		10, 0, 0 },
{ R_MOUNTAIN,	"del",		10, 0, 0 },
{ R_DESERT,	"dhan",		10, 0, 0 },
{ R_MOUNTAIN,	"don",		10, 0, 0 },
{ -2,		"dorf",		 4, 1, 0 },
{ R_SWAMP,	"fel",		10, 0, 0 },
{ R_PLAIN,	"field",	10, 0, 0 },
{ R_FOREST,	"firth",	10, 0, 0 },
{ -1,		"folk",		 3, 0, 0 },
{ -1,		"ford",		 3, 0, 0 },
{ -1,		"gate",		 3, 0, 0 },
{ R_MOUNTAIN,	"gill",		10, 0, 0 },
{ -1,		"glen",		 3, 0, 0 },
{ R_DESERT,	"gobi",		10, 0, 0 },
{ R_JUNGLE,	"gol",		 3, 0, 0 },
{ -2,		"gost",		 4, 1, 0 },
{ -2,		"grad",		 4, 1, 0 },
{ -2,		"grave",	 4, 1, 0 },
{ R_FOREST,	"grove",	10, 0, 0 },
{ R_SWAMP,	"gwern",	10, 0, 0 },
{ -2,		"ham",		 4, 1, 0 },
{ -2,		"haven",	10, 0, 1 },
{ R_MOUNTAIN,	"head",		10, 0, 0 },
{ R_TUNDRA,	"heath",	10, 0, 0 },
{ -1,		"heim",		 3, 0, 0 },
{ -2,		"hold",		 4, 1, 0 },
{ -2,		"holm",		 4, 1, 0 },
{ R_FOREST,	"hurst",	10, 0, 0 },
{ -2,		"kirk",		 4, 1, 0 },
{ R_DESERT,	"kum",		10, 0, 0 },
{ -1,		"land",		 3, 0, 0 },
{ R_FOREST,	"lea",		 2, 0, 0 },
{ R_FOREST,	"lee",		 2, 0, 0 },
{ R_FOREST,	"leigh",	 2, 0, 0 },
{ R_FOREST,	"ley",		 2, 0, 0 },
{ R_TUNDRA,	"ling",		10, 0, 0 },
{ R_OCEAN,	"loch",		15, 0, 0 },
{ R_FOREST,	"ly",		 2, 0, 0 },
{ R_OCEAN,	"mar",		10, 0, 0 },
{ -1,		"mark",		 3, 0, 0 },
{ R_OCEAN,	"mere",		10, 0, 0 },
{ -2,		"minster",	 4, 1, 0 },
{ R_MOUNTAIN,	"mont",		10, 0, 0 },
{ R_SWAMP,	"moor",		10, 0, 0 },
{ R_SWAMP,	"more",		10, 0, 0 },
{ R_SWAMP,	"moss",		10, 0, 0 },
{ -2,		"mouth",	10, 0, 1 },
{ -2,		"pest",		 4, 1, 0 },
{ -2,		"pol",		 4, 1, 0 },
{ -2,		"pool",		10, 0, 1 },
{ -2,		"port",		10, 0, 1 },
{ R_FOREST,	"rath",		10, 0, 0 },
{ R_PLAIN,	"run",		10, 0, 0 },
{ -2,		"sale",		 4, 1, 0 },
{ R_DESERT,	"sand",		10, 0, 0 },
{ R_FOREST,	"shaw",		10, 0, 0 },
{ R_PLAIN,	"shire",	10, 0, 0 },
{ -2,		"side",		10, 0, 1 },
{ -2,		"stad",		 4, 1, 0 },
{ -1,		"stan",		 3, 0, 0 },
{ -2,		"stead",	 4, 1, 0 },
{ -2,		"stoke",	 4, 1, 0 },
{ -2,		"stowe",	 4, 1, 0 },
{ R_OCEAN,	"tarn",		15, 0, 0 },
{ R_MOUNTAIN,	"tell",		10, 0, 0 },
{ -2,		"ton",		 4, 1, 0 },
{ R_MOUNTAIN,	"tor",		10, 0, 0 },
{ -2,		"town",		 4, 1, 0 },
{ -1,		"vale",		 3, 0, 0 },
{ -2,		"ville",	 4, 1, 0 },
{ R_JUNGLE,	"wald",		10, 0, 0 },
{ R_OCEAN,	"water",	15, 0, 0 },
{ -1,		"way",		 3, 0, 0 },
{ -2,		"wick",		 4, 1, 0 },
{ R_FOREST,	"wood",		10, 0, 0 },
};

int syllprob[] = { 0, 60, 40, 0 };

//
// The following stuff is just for this file, to setup the names during
// world generation
//

static AList regionnames;
static int nnames;
static int ntowns;
static int nregions;
static int tSyll, tIC, tV, tFC;

void SetupNames()
{
	unsigned int i;

	nnames = 0;
	ntowns = 0;
	nregions = 0;

	for (i = 0, tIC = 0; i < sizeof(ic) / sizeof(ic[0]); i++)
		tIC += ic[i].prob;
	for (i = 0, tV = 0; i < sizeof(v) / sizeof(v[0]); i++)
		tV += v[i].prob;
	for (i = 0, tFC = 0; i < sizeof(fc) / sizeof(fc[0]); i++)
		tFC += fc[i].prob;
	for (i = 0, tSyll = 0; i < sizeof(syllprob) / sizeof(syllprob[0]); i++)
		tSyll += syllprob[i];
}

void CountNames()
{
	Aoutfile names;
	AString *name;

	Awrite(AString("Regions ") + nregions);

	// Dump all the names we created to a file so the GM can scan
	// them easily (to check for randomly generated rude words,
	// for example)
	names.OpenByName("names.out");
	forlist(&regionnames) {
		name = (AString *) elem;
		names.PutStr(*name);
	}
	names.Close();
}

int AGetName(int town, ARegion *reg)
{
	int unique, rnd, syllables, i, trail, port, similar;
	unsigned int u;
	char temp[80];
	AString *name;

	port = 0;
	if (town) {
		for (i = 0; i < NDIRS; i++)
			if (reg->neighbors[i] &&
					TerrainDefs[reg->neighbors[i]->type].similar_type == R_OCEAN)
				port = 1;
	}

	unique = 0;
	while (!unique) {
		rnd = getrandom(tSyll);
		for (syllables = 0; rnd >= syllprob[syllables]; syllables++)
			rnd -= syllprob[syllables];
		syllables++;
		temp[0] = 0;
		trail = 0;
		while (syllables-- > 0) {
			if (!syllables) {
				// Might replace the last syllable with a
				// terrain specific suffix
				rnd = getrandom(400);
				similar = TerrainDefs[reg->type].similar_type;
				// Use forest names for underforest
				if (similar == R_UFOREST)
					similar = R_FOREST;
				// ocean (water) names for lakes
				if (similar == R_LAKE)
					similar = R_OCEAN;
				// and plains names for cavern
				if (similar == R_CAVERN)
					similar = R_PLAIN;
				for (u = 0; u < sizeof(ts) / sizeof(ts[0]); u++) {
					if (ts[u].terrain == similar ||
							ts[u].terrain == -1 ||
							(ts[u].town && town) ||
							(ts[u].port && port)) {
						if (rnd >= ts[u].prob)
							rnd -= ts[u].prob;
						else {
							if (trail) {
								switch(ts[u].word[0]) {
									case 'a':
									case 'e':
									case 'i':
									case 'o':
									case 'u':
										strcat(temp, "'");
										break;
									default:
										break;
								}
							}
							strcat(temp, ts[u].word);
							break;
						}
					}
				}
				if (u < sizeof(ts) / sizeof(ts[0]))
					break;
			}
			if (getrandom(5) > 0) {
				// 4 out of 5 syllables start with a consonant sequence
				rnd = getrandom(tIC);
				for (i = 0; rnd >= ic[i].prob; i++)
					rnd -= ic[i].prob;
				strcat(temp, ic[i].word);
			} else if (trail) {
				// separate adjacent vowels
				strcat(temp, "'");
			}
			// All syllables have a vowel sequence
			rnd = getrandom(tV);
			for (i = 0; rnd >= v[i].prob; i++)
				rnd -= v[i].prob;
			strcat(temp, v[i].word);
			if (getrandom(5) > 1) {
				// 3 out of 5 syllables end with a consonant sequence
				rnd = getrandom(tFC);
				for (i = 0; rnd >= fc[i].prob; i++)
					rnd -= fc[i].prob;
				strcat(temp, fc[i].word);
				trail = 0;
			} else {
				trail = 1;
			}
		}
		temp[0] = toupper(temp[0]);
		unique = 1;
		forlist(&regionnames) {
			name = (AString *) elem;
			if (*name == temp) {
				unique = 0;
				break;
			}
		}
		if (strlen(temp) > 12)
			unique = 0;
	}

	nnames++;
	if (town)
		ntowns++;
	else
		nregions++;

	name = new AString(temp);
	regionnames.Add(name);

	return regionnames.Num();
}

const char *AGetNameString(int name)
{
	AString *str;

	forlist(&regionnames) {
		name--;
		if (!name) {
			str = (AString *) elem;
			return str->Str();
		}
	}

	// This should never happen
	return "Error";
}

void Game::CreateWorld()
{
	int nx = 0;
	int ny = 1;
	if (Globals->MULTI_HEX_NEXUS) {
		ny = 2;
		while(nx <= 0) {
			Awrite("How many hexes should the nexus region be?");
			nx = Agetint();
			if (nx == 1) ny = 1;
			else if (nx % 2) {
				nx = 0;
				Awrite("The width must be a multiple of 2.");
			}
		}
	} else {
		nx = 1;
	}

	int xx = 0;
	while (xx <= 0) {
		Awrite("How wide should the map be? ");
		xx = Agetint();
		if ( xx % 8 ) {
			xx = 0;
			Awrite( "The width must be a multiple of 8." );
		}
	}
	int yy = 0;
	while (yy <= 0) {
		Awrite("How tall should the map be? ");
		yy = Agetint();
		if ( yy % 8 ) {
			yy = 0;
			Awrite( "The height must be a multiple of 8." );
		}
	}

	regions.CreateLevels(2 + Globals->UNDERWORLD_LEVELS +
			Globals->UNDERDEEP_LEVELS + Globals->ABYSS_LEVEL);

	SetupNames();

	regions.CreateNexusLevel( 0, nx, ny, "nexus" );
	regions.CreateSurfaceLevel( 1, xx, yy, 0 );

	// Create underworld levels
	int i;
	for (i = 2; i < Globals->UNDERWORLD_LEVELS+2; i++) {
		int xs = regions.GetLevelXScale(i);
		int ys = regions.GetLevelYScale(i);
		regions.CreateUnderworldLevel(i, xx/xs, yy/ys, "underworld");
	}
	// Underdeep levels
	for (i=Globals->UNDERWORLD_LEVELS+2;
			i<(Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+2); i++) {
		int xs = regions.GetLevelXScale(i);
		int ys = regions.GetLevelYScale(i);
		regions.CreateUnderdeepLevel(i, xx/xs, yy/ys, "underdeep");
	}

	if (Globals->ABYSS_LEVEL) {
		regions.CreateAbyssLevel(Globals->UNDERWORLD_LEVELS +
				Globals->UNDERDEEP_LEVELS + 2, "abyss");
	}

	CountNames();

	if (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS == 1) {
		regions.MakeShaftLinks( 2, 1, 8 );
	} else if (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS) {
		int i, ii;
		// shafts from surface to underworld
		regions.MakeShaftLinks(2, 1, 10);
		for (i=3; i<Globals->UNDERWORLD_LEVELS+2; i++) {
			regions.MakeShaftLinks(i, 1, 10*i-10);
		}
		// Shafts from underworld to underworld
		if (Globals->UNDERWORLD_LEVELS > 1) {
			for (i = 3; i < Globals->UNDERWORLD_LEVELS+2; i++) {
				for (ii = 2; ii < i; ii++) {
					if (i == ii+1) {
						regions.MakeShaftLinks(i, ii, 12);
					} else {
						regions.MakeShaftLinks(i, ii, 24);
					}
				}
			}
		}
		// underdeeps to underworld
		if (Globals->UNDERDEEP_LEVELS && Globals->UNDERWORLD_LEVELS) {
			// Connect the topmost of the underdeep to the bottommost
			// underworld
			regions.MakeShaftLinks(Globals->UNDERWORLD_LEVELS+2,
					Globals->UNDERWORLD_LEVELS+1, 12);
		}
		// Now, connect the underdeep levels together
		if (Globals->UNDERDEEP_LEVELS > 1) {
			for (i = Globals->UNDERWORLD_LEVELS+3;
					i < Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+2;
					i++) {
				for (ii = Globals->UNDERWORLD_LEVELS+2; ii < i; ii++) {
					if (i == ii+1) {
						regions.MakeShaftLinks(i, ii, 12);
					} else {
						regions.MakeShaftLinks(i, ii, 25);
					}
				}
			}
		}
	}

	regions.SetACNeighbors( 0, 1, xx, yy );

	regions.InitSetupGates( 1 );
	// Set up gates on all levels of the underworld
	for (int i=2; i < Globals->UNDERWORLD_LEVELS+2; i++) {
		regions.InitSetupGates( i );
	}
	// Underdeep has no gates, only the possible shafts above.

	regions.FixUnconnectedRegions();

	regions.FinalSetupGates();

	regions.CalcDensities();
	
	regions.TownStatistics();
}

int ARegionList::GetRegType( ARegion *pReg )
{
	//
	// Figure out the distance from the equator, from 0 to 3.
	//
	// Note that the -3 applied to y here is because I'm assuming we're using
	// icosahedral levels, which don't quite fill their y-space
	int lat = ( pReg->yloc * 8 ) / ( pRegionArrays[ pReg->zloc ]->y);
	if (lat > 3)
	{
		lat = (7 - lat);
	}
	if (lat < 0) lat = 0;

	// Underworld region
	if ((pReg->zloc > 1) && (pReg->zloc < Globals->UNDERWORLD_LEVELS+2)) {
		int r = getrandom(14);
		switch (r) {
			case 0:
			case 1:
			case 2:
				return R_OCEAN;
			case 3:
			case 4:
			case 5:
				return R_CAVERN;
			case 6:
			case 7:
			case 8:
				return R_UFOREST;
			case 9:
			case 10:
				return R_TUNNELS;
			case 12:
			case 13:
				return R_CHASM;
			default:
				return( 0 );
		}
	}

	// Underdeep region
	if ((pReg->zloc > Globals->UNDERWORLD_LEVELS+1) &&
			(pReg->zloc < Globals->UNDERWORLD_LEVELS+
			 			  Globals->UNDERDEEP_LEVELS+2)) {
		int r = getrandom(4);
		switch(r) {
			case 0:
				return R_OCEAN;
			case 1:
				return R_CHASM;
			case 2:
				return R_DFOREST;
			case 3:
				return R_GROTTO;
			default:
				return (0);
		}
	}

	// surface region
	if ( pReg->zloc == 1 ) {
		int r = getrandom(64);
		switch (lat)
		{
		case 0: /* Arctic regions */
			if (r < 32) return R_TUNDRA;
			if (r < 40) return R_MOUNTAIN;
			if (r < 48) return R_FOREST;
			return R_PLAIN;
		case 1: /* Colder regions */
			if (r < 8) return R_TUNDRA;
			if (r < 24) return R_PLAIN;
			if (r < 40) return R_FOREST;
			if (r < 48) return R_MOUNTAIN;
			return R_SWAMP;
		case 2: /* Warmer regions */
			if (r < 16) return R_PLAIN;
			if (r < 28) return R_FOREST;
			if (r < 36) return R_MOUNTAIN;
			if (r < 44) return R_SWAMP;
			if (r < 52) return R_JUNGLE;
			return R_DESERT;
		case 3: /* tropical */
			if (r < 16) return R_PLAIN;
			if (r < 24) return R_MOUNTAIN;
			if (r < 36) return R_SWAMP;
			if (r < 48) return R_JUNGLE;
			return R_DESERT;
		}
		return R_OCEAN;
	}

	if ( pReg->zloc == 0 )
	{
		//
		// This really shouldn't ever get called.
		//
		return( R_NEXUS );
	}

	//
	// This really shouldn't get called either
	//
	return( R_OCEAN );
}

int ARegionList::GetLevelXScale(int level)
{
	// Surface and nexus are unscaled
	if (level < 2) return 1;

	// If we only have one underworld level it's 1/2 size
	if (Globals->UNDERWORLD_LEVELS == 1 && Globals->UNDERDEEP_LEVELS == 0)
		return 2;

	// We have multiple underworld levels
	if (level >= 2 && level < Globals->UNDERWORLD_LEVELS+2) {
		// Topmost level is full size in x direction
		if (level == 2) return 1;
		// All other levels are 1/2 size
		return 2;
	}
	if (level >= Globals->UNDERWORLD_LEVELS+2 &&
			level < (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+2)){
		// Topmost underdeep level is 1/2 size
		if (level == Globals->UNDERWORLD_LEVELS+2) return 2;
		// All others are 1/4 size
		return 4;
	}
	// We couldn't figure it out, assume not scaled.
	return 1;
}

int ARegionList::GetLevelYScale(int level)
{
	// Surface and nexus are unscaled
	if (level < 2) return 1;

	// If we only have one underworld level it's 1/2 size
	if (Globals->UNDERWORLD_LEVELS == 1 && Globals->UNDERDEEP_LEVELS == 0)
		return 2;

	// We have multiple underworld levels
	if (level >= 2 && level < Globals->UNDERWORLD_LEVELS+2) {
		// Topmost level is 1/2 size in the y direction
		if (level == 2) return 2;
		// Bottommost is 1/4 size in the y direction
		if (level == Globals->UNDERWORLD_LEVELS+1) return 4;
		// All others are 1/2 size in the y direction
		return 2;
	}
	if (level >= Globals->UNDERWORLD_LEVELS+2 &&
			level < (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+2)){
		// All underdeep levels are 1/4 size in the y direction.
		return 4;
	}
	// We couldn't figure it out, assume not scaled.
	return 1;
}

int ARegionList::CheckRegionExit(ARegion *pFrom, ARegion *pTo )
{
	if ((pFrom->zloc==1) ||
		(pFrom->zloc>Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+1)) {
		return( 1 );
	}

	int chance = 0;
	if ( pFrom->type == R_CAVERN || pFrom->type == R_UFOREST ||
		pTo->type == R_CAVERN || pTo->type == R_UFOREST )
	{
		chance = 25;
	}
	if ( pFrom->type == R_TUNNELS || pTo->type == R_TUNNELS)
	{
		chance = 50;
	}
	if (pFrom->type == R_GROTTO || pFrom->type == R_DFOREST ||
	   pTo->type == R_GROTTO || pTo->type == R_DFOREST) {
		// better connected underdeeps
		chance = 40;
	}
	if (pFrom->type == R_CHASM || pTo->type == R_CHASM) {
		chance = 40;
	}
	if (getrandom(100) < chance) {
		return( 0 );
	}
	return( 1 );
}

int ARegionList::GetWeather( ARegion *pReg, int month )
{
	if (pReg->zloc == 0)
	{
		return W_NORMAL;
	}

	if ( pReg->zloc > 1 )
	{
		return( W_NORMAL );
	}

	int ysize = pRegionArrays[ 1 ]->y;

	if ((3*( pReg->yloc+1))/ysize == 0)
	{
		/* Northern third of the world */
		if (month > 9 || month < 2)
		{
			return W_WINTER;
		}
		else
		{
			return W_NORMAL;
		}
	}

	if ((3*( pReg->yloc+1))/ysize == 1)
	{
		/* Middle third of the world */
		if (month == 11 || month == 0 || month == 5 || month == 6)
		{
			return W_MONSOON;
		}
		else
		{
			return W_NORMAL;
		}
	}

	if (month > 3 && month < 8)
	{
		/* Southern third of the world */
		return W_WINTER;
	}
	else
	{
		return W_NORMAL;
	}
}

int ARegion::CanBeStartingCity( ARegionArray *pRA )
{
	if (type == R_OCEAN) return 0;
	if (!IsCoastal()) return 0;
	if (town && town->pop == 5000) return 0;

	int regs = 0;
	AList inlist;
	AList donelist;

	ARegionPtr * temp = new ARegionPtr;
	temp->ptr = this;
	inlist.Add(temp);

	while(inlist.Num()) {
		ARegionPtr * reg = (ARegionPtr *) inlist.First();
		for (int i=0; i<NDIRS; i++) {
			ARegion * r2 = reg->ptr->neighbors[i];
			if (!r2) continue;
			if (r2->type == R_OCEAN) continue;
			if (GetRegion(&inlist,r2->num)) continue;
			if (GetRegion(&donelist,r2->num)) continue;
			regs++;
			if (regs>20) return 1;
			ARegionPtr * temp = new ARegionPtr;
			temp->ptr = r2;
			inlist.Add(temp);
		}
		inlist.Remove(reg);
		donelist.Add(reg);
	}
	return 0;
}

void ARegion::MakeStartingCity()
{
	if (!Globals->TOWNS_EXIST) return;

	if (Globals->GATES_EXIST) gate = -1;
	
	if (town) delete town;
	
	AddTown(TOWN_CITY);

	if (!Globals->START_CITIES_EXIST) return;

	town->hab = 125 * Globals->CITY_POP / 100;
	while (town->pop < town->hab) town->pop += getrandom(200)+200;
	town->dev = TownDevelopment();

	float ratio;
	Market *m;
	markets.DeleteAll();
	if (Globals->START_CITIES_START_UNLIMITED) {
		for (int i=0; i<NITEMS; i++) {
			if ( ItemDefs[i].flags & ItemType::DISABLED ) continue;
			if ( ItemDefs[ i ].type & IT_NORMAL ) {
				if (i==I_SILVER || i==I_LIVESTOCK || i==I_FISH || i==I_GRAIN)
					continue;
				m = new Market(M_BUY,i,(ItemDefs[i].baseprice*5/2),-1,
						5000,5000,-1,-1);
				markets.Add(m);
			}
		}
		ratio = ItemDefs[race].baseprice / ((float)Globals->BASE_MAN_COST * 10);
		// hack: include wage factor of 10 in float calculation above
		m=new Market(M_BUY,race,(int)(Wages()*4*ratio),-1, 5000,5000,-1,-1);
		markets.Add(m);
		if (Globals->LEADERS_EXIST) {
			ratio=ItemDefs[I_LEADERS].baseprice/((float)Globals->BASE_MAN_COST * 10);
			// hack: include wage factor of 10 in float calculation above
			m = new Market(M_BUY,I_LEADERS,(int)(Wages()*4*ratio),
					-1,5000,5000,-1,-1);
			markets.Add(m);
		}
	} else {
		SetupCityMarket();
		ratio = ItemDefs[race].baseprice / ((float)Globals->BASE_MAN_COST * 10);
		// hack: include wage factor of 10 in float calculation above
		/* Setup Recruiting */
		m = new Market( M_BUY, race, (int)(Wages()*4*ratio),
				Population()/5, 0, 10000, 0, 2000 );
		markets.Add(m);
		if ( Globals->LEADERS_EXIST ) {
			ratio=ItemDefs[I_LEADERS].baseprice/((float)Globals->BASE_MAN_COST * 10);
			// hack: include wage factor of 10 in float calculation above
			m = new Market( M_BUY, I_LEADERS, (int)(Wages()*4*ratio),
					Population()/25, 0, 10000, 0, 400 );
			markets.Add(m);
		}
	}
}

int ARegion::IsStartingCity() {
	if (town && town->pop >= (Globals->CITY_POP * 120 / 100)) return 1;
	return 0;
}

int ARegion::IsSafeRegion()
{
	if (type == R_NEXUS) return 1;
	return( Globals->SAFE_START_CITIES && IsStartingCity() );
}

ARegion *ARegionList::GetStartingCity( ARegion *AC,
					int i,
					int level,
					int maxX,
					int maxY )
{
	ARegionArray *pArr = pRegionArrays[ level ];
	ARegion * reg = 0;

	if ( pArr->x < maxX ) maxX = pArr->x;
	if ( pArr->y < maxY ) maxY = pArr->y;

	int tries = 0;
	while (!reg && tries < 10000) {
		//
		// We'll just let AC exits be all over the map.
		//
		int x = getrandom( maxX );
		int y = 2 * getrandom( maxY / 2 ) + x % 2;

		reg = pArr->GetRegion( x, y);

		if (!reg || !reg->CanBeStartingCity( pArr )) {
			reg = 0;
			tries++;
			continue;
		}

		for (int j=0; j<i; j++) {
			if (!AC->neighbors[j]) continue;
			if (GetPlanarDistance(reg,AC->neighbors[j], 0, maxY / 10 + 2) < maxY / 10 + 2 ) {
				reg = 0;
				tries++;
				break;
			}
		}
	}

	// Okay, we failed to find something that normally would work
	// we'll just take anything that's of the right distance
	tries = 0;
	while (!reg && tries < 10000) {
		//
		// We couldn't find a normal starting city, let's just go for ANY
		// city
		//
		int x = getrandom( maxX );
		int y = 2 * getrandom( maxY / 2 ) + x % 2;
		reg = pArr->GetRegion( x, y);
		if (!reg || reg->type == R_OCEAN) {
			tries++;
			reg = 0;
			continue;
		}

		for (int j=0; j<i; j++) {
			if (!AC->neighbors[j]) continue;
			if (GetPlanarDistance(reg,AC->neighbors[j], 0, maxY / 10 + 2) < maxY / 10 + 2 ) {
				reg = 0;
				tries++;
				break;
			}
		}
	}

	// Okay, if we still don't have anything, we're done.
	return reg;
}

