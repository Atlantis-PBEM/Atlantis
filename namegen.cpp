// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 2022 Valdis Zobēla
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
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

#include "namegen.h"

#include "game.h"
#include "gamedata.h"

#include <cctype>
#include "rng.hpp"
#include "string_filters.hpp"
std::vector<std::string> aPrefAbstract = {
    "A", "Ab", "Ach", "Ad", "Ae", "Ag", "Ai", "Ak", "Al", "Am", "An", "Ap", "Ar", "As", "Ash", "At", "Ath", "Au", "Ay",
    "Ban", "Bar", "Brel", "Bren",
    "Cam", "Cla", "Cler", "Col", "Con", "Cor", "Cul", "Cuth", "Cy", "Chal", "Chan", "Chi", "Chon", "Chul", "Chur",
    "Del", "Dur", "Dwar", "Dwur",
    "Ek", "El", "En", "Eth", "Fal", "Far", "Fel", "Fell", "Fen", "Flan", "Flar", "Fly", "Fur", "Fy",
    "Gal", "Gan", "Gar", "Gel", "Glan", "Glar", "Glen", "Glir", "Glyn", "Glyr", "Glyth", "Gogh", "Gor", "Goth", "Gwal", "Gwen", "Gwur", "Gy", "Gyl", "Gyn", "Ghal", "Ghash", "Ghor", "Ghoz", "Ghul",
    "Hach", "Haj", "Hal", "Ham", "Hel", "Hen", "Hil", "Ho", "Hol", "Hul",
    "Ice", "Id", "Ie", "Il", "Im", "In", "Ir", "Is", "Iz",
    "Ja", "Jak", "Jar", "Jaz", "Jeth", "Jez", "Ji", "Jul", "Jur", "Juz",
    "Kag", "Kai", "Kaj", "Kal", "Kam", "Ken", "Kor", "Kul", "Kwal", "Kwar", "Kwel", "Kwen", "Kha", "Khel", "Khor", "Khul", "Khuz",
    "Lagh", "Lar", "Lin", "Lir", "Loch", "Lor", "Lyn", "Lyth",
    "Mal", "Man", "Mar", "Me", "Mer", "Meth", "Mil", "Min", "Mir",
    "Nam", "Nar", "Nel", "Nem", "Nen", "Nor", "Noth", "Nyr",
    "Ob", "Oe", "Ok", "Ol", "On", "Or", "Ow",
    "Par", "Pel", "Por", "Py", "Pyr", "Pyl",
    "Ral", "Ra", "Ram", "Rath", "Re", "Rel", "Ren", "Ri", "Ril", "Ro", "Ror", "Ruk", "Ry",
    "Sen", "Seth", "Sul", "Shae", "Shal", "Shar", "Shen", "Shir",
    "Tal", "Tam", "Tar", "Tel", "Ten", "Tir", "Tol", "Tul", "Tur", "Thor", "Thul",
    "U", "Uk", "Un", "Ul",
    "Va", "Val", "Van", "Vel", "Ven", "Veren", "Vul",
    "Wal", "War", "We", "Wel", "Wil", "Win",
    "Y", "Ya", "Ych", "Ye", "Yg", "Yi", "Yl", "Yn", "Yo", "Yp", "Yr", "Yth", "Yu", "Yul", "Za", "Zar",
    "Zel", "Zi", "Zim", "Zir", "Zol", "Zor", "Zhok", "Zhu", "Zhuk", "Zhul"
};

std::vector<std::string> aSufAbstract = {
    "a", "ach", "aech", "ael", "aem", "aen", "aer", "aeth", "ail", "ain", "air", "aith", "all", "an", "and", "ar", "ash", "auch", "aul", "aun", "aur",
    "baen", "bain", "bar", "bath", "ben", "byr",
    "cael", "caer", "can", "cen", "cor", "cynd",
    "dach", "dail", "dain", "dan", "dar", "dik", "dir", "dy",
    "e", "eal", "el", "eld", "eth",
    "gar", "gath", "grim",
    "i", "ian", "ield", "ien", "ieth", "il", "ior", "ioth", "ish",
    "maer", "mail", "main", "mar", "maren", "miel", "mieth",
    "nain", "nair", "naith", "nal", "nar", "nath", "nen", "ner", "niel", "nien", "nieth", "nor", "noth", "nul", "nur", "nyr",
    "o", "och", "or", "oth", "oum", "owen",
    "rach", "raid", "rail", "rain", "raish", "raith", "ran", "rar", "ras", "raven", "ren", "riel", "rien", "rier", "rik", "ril", "rish", "ron", "ror", "ros", "roth", "rych", "ryl", "ryr", "rych", "thach", "thain", "thak", "thal", "than", "thar", "thiel", "thien", "thor", "thul", "thur",
    "ug", "uild", "uin", "uith", "uk", "ul", "un", "ur", "uth",
    "wain", "waith", "wald", "war", "ward", "well", "wen", "win",
    "y", "yll", "ynd", "yr", "yth",
    "zak", "zel", "zen", "zokh", "zor", "zul", "zuth"
};


std::vector<std::string> aPrefEscimo = {
    "ach", "achana", "achka", "achuk", "ak", "akip", "akun", "ani", "aninnik", "ap", "apat", "api", "apucha", "chat", "chit", "chua", "chut", "ichis", "ichitt", "ikissik", "ikiutta", "iniki", "iniss", "inissik", "ippik", "issi", "it", "ita", "itani", "kass", "kikk", "kiun", "kyap", "kyaun", "kyu", "kyup", "kuk", "kup", "kupp", "kut", "pi", "pyaun", "pyi", "pyu", "pyuin", "pyup", "siss", "syaun", "syiin", "syu", "syuss", "syuun", "saun", "suss", "suun", "tap", "tikk", "tya", "tyaan", "tyiun", "tuan", "uch", "uchach", "uchk", "ucht", "upani", "upik", "utin", "unuchut"
};

std::vector<std::string> aSufEscimo = {
    "ach", "akan", "akya", "ani", "anu", "chach", "chat", "chin", "chit", "chun", "iakap", "ichak", "ikin", "ikip", "ikta", "in", "innik", "ipa", "ippik", "ipuch", "issik", "it", "itut", "kan", "kip", "kuk", "kukik", "kya", "pich", "pin", "pip", "pput", "sup", "tait", "tin", "tip", "tit", "titut", "uchi", "uk", "ukta", "utta"
};


std::vector<std::string> aPrefGreek = { "aer", "agamen", "agor", "aion", "air", "aker", "akrogon", "akro", "amen", "ametan", "amiant", "ampel", "anan", "anankamom", "andro", "aner", "antano", "aorat", "apolytro", "athem", "autark", "biast", "byblo", "chrono", "dogmo", "dokim", "ekbalo", "ekpipto", "ektrom", "entell", "epikatar", "ereun", "exang", "exod", "gorgo", "hekono", "heter", "hikan", "hilar", "hymno", "hypno", "ianno", "ierem", "kalym", "katar", "klepto", "kreman", "makar", "malak", "maran", "metan", "nest", "oikonom", "optan", "orgil", "otar", "ouran", "papyr", "parait", "paramen", "parthen", "perik", "peril", "perim", "philag", "polyl", "poro", "prax", "sin", "sken", "smyrno", "strato", "thron", "trit", "troglo", "zelot"};
std::vector<std::string> aSufGreek = { "akos", "alizo", "alotos", "arenos", "aros", "arotes", "arx", "askalos", "atos", "atres", "einos", "elos", "eros", "eryx", "etes", "imos", "irmos", "itos", "okles", "opos", "otus" "polis" "us"};

std::vector<std::string> aPrefAztec = { "Acayu", "Alar", "Apatzin", "Ayoquez", "Ayu", "Cham", "Chetu", "Chi", "Cho", "Chun", "Colo", "Comalcal", "Comi", "Cuet", "Hala", "Huicha", "Huimax", "Hunuc", "Ix", "Ixmiquil", "Iza", "Jal", "Jamil", "Juchi", "Kaminal", "Kantunil", "Maya", "Mapas", "Maxcan", "Maz", "Miahu", "Minatit", "Mul", "Noch", "Oax", "Oco", "Ome", "Ozibilchal", "Panab", "Pet", "Pochu", "Popoca", "Say", "Sayax", "Tehuan", "Tenoxtit", "Tep", "Tik", "Tiz", "Tizi", "Tlaco", "Tom", "Ton", "Tul", "Tun", "Tux", "Uaxac", "Urua", "Yaxchi", "Zacat", "Zana", "Zima"};
std::vector<std::string> aSufAztec = { "atlan", "ixtlan", "huas", "juyu", "poton", "talpan", "tepec", "tepetl", "titlan", "zalan"};

std::vector<std::string> aPrefDrow = { "Alean", "Ale", "Arab", "Arken", "Auvry", "Baen", "Barri", "Cladd", "Desp", "De", "Do'", "Eils", "Everh", "Fre", "Gode", "Helvi", "Hla", "Hun", "Ken", "Kil", "Mae", "Mel", "My", "Noqu", "Orly", "Ouss", "Rilyn", "Teken'", "Tor", "Zau"};
std::vector<std::string> aSufDrow = { "afin", "ana", "ani", "ar", "arn", "ate", "ath", "duis", "ervs", "ep", "ett", "ghym", "iryn", "lyl", "mtor", "ndar", "neld", "rae", "rahel", "rret", "sek", "th", "tlar", "t'tar", "tyl", "und", "urden", "val", "virr", "zynge"};

std::vector<std::string> aPrefScotish = { "Aber", "Ar", "As", "At", "Avie", "Bal", "Ben", "Bran", "Brech", "Bro", "Cairn", "Can", "Carl", "Colon", "Clyde", "Craig", "Cum", "Dearg", "Don", "Dor", "Dun", "Dur", "El", "Fal", "For", "Fyne", "Glas", "Hal", "Inver", "Ju", "Kil", "Kilbran", "Kirrie", "Lairg", "Lin", "Lo", "Loch", "Lorn", "Lyb", "Ma", "Mal", "Mel", "Monadh", "Nairn", "Nith", "Ob", "Oron", "Ran", "Scar", "Scour", "Spey", "Stom", "Strom", "Tar", "Tay", "Ti", "Tober", "Uig", "Ulla", "Wick"};
std::vector<std::string> aSufScotish = { "aline", "an", "aray", "avon", "ba", "bert", "bis", "blane", "bran", "da", "dee", "deen", "far", "feldy", "gin", "gorm", "ie", "in", "kaig", "kirk", "laig", "liath", "maol", "mond", "moral", "more", "mory", "muir", "na", "nan", "ner", "ness", "nhe", "nock", "noth", "nure", "ock", "pool", "ra", "ran", "ree", "res", "say", "ster", "tow"};

std::vector<std::string> aPrefAfrica = { "Ag", "Ahr", "Ba", "Bor", "Dar", "Don", "Dor", "Dung", "Ga", "Gal", "Gam", "Gul", "Gur", "Gwa", "Gwah", "Gwar", "Gwul", "Ig", "Ja", "Jih", "Jug", "Kas", "Kesh", "Kides", "Kili", "Kor", "Kul", "Kush", "Lar", "Lu", "Ma", "Mat", "Mbeg", "Mbeng", "Min", "Ngor", "Ngul", "N'Gul", "Nyag", "N'Yag", "N'Zin", "Ong", "Rod", "Sha", "Sum", "Swa", "Ti", "Tot", "Ug", "Ung", "Wad", "Waz", "Wur", "Ya", "Za", "Zang", "Zar", "Zem", "Zik", "Zim", "Zu", "Zul"};
std::vector<std::string> aSufAfrica = { "a", "ad", "aga", "ara", "ai", "al", "alo", "ang", "anga", "ani", "bab", "bal", "balla", "biba", "bu", "buk", "buru", "daja", "dar", "donga", "dor", "du", "dul", "duru", "daza", "'guba", "'gung", "hili", "i", "id", "iji", "ili", "jari", "jaro", "juri", "'ka", "lah", "lur", "mala", "mim", "mu", "munga", "mur", "nur", "nuzi", "o", "od", "ofo", "oja", "onga", "ozi", "ra", "sala", "sula", "sunga", "tulo", "u", "ula", "ulga", "unga", "wa", "wath", "we", "wuzi", "zaja", "zaza", "zin", "zum", "zung", "zur"};

std::vector<std::string> aPrefElven1 = { "Ama", "Ari", "Aza", "Cla", "Cy", "Dae", "Dho", "Dre", "Fi", "Ia", "Ky", "Lue", "Ly", "Mai", "My", "Na", "Nai", "Nu", "Ny", "Py", "Ry", "Rua", "Sae", "Sha", "She", "Si", "Tia", "Ty", "Ya", "Zy"};
std::vector<std::string> aSufElven1 = { "nae", "lae", "dar", "drimme", "lath", "lith", "lyth", "lan", "lanna", "lirr", "lis", "lys", "lyn", "llinn", "lihn", "nal", "nin", "nine", "nyn", "nis", "sal", "sel", "tas", "thi", "thil", "vain", "vin", "wyn", "zair"};

std::vector<std::string> aPrefElven2 = { "Aer", "Al", "Am", "Ang", "Ansr", "Ar", "Arn", "Bael", "Cael", "Cal", "Cas", "Cor", "Eil", "Eir", "El", "Er", "Ev", "Fir", "Fis", "Gael", "Gil", "Il", "Kan", "Ker", "Keth", "Koeh", "Kor", "Laf", "Lam", "Mal", "Nim", "Rid", "Rum", "Seh", "Sel", "Sim", "Syl", "Tahl", "Vil"};
std::vector<std::string> aSufElven2 = { "ael", "aer", "aera", "aias", "aia", "aith", "aira", "ala", "ali", "ani", "uanna", "ari", "aro", "ibrar", "adar", "odar", "udrim", "emar", "esti", "evar", "afel", "efel", "ihal", "ihar", "ahel", "ihel", "ian", "ianna", "iat", "iel", "ila", "inar", "ine", "ith", "elis", "ellon", "inal", "anis", "aruil", "eruil", "isal", "sali", "sar", "asar", "isar", "asel", "isel", "itas", "ethil", "avain", "avin", "azair"};

std::vector<std::string> aPrefDwarven = { "agar", "agaz", "barak", "baruk", "baraz", "bizar", "bizul", "bul", "buzar", "garak", "gor", "gog", "gorog", "gothol", "guzib", "ibin", "ibiz", "izil", "izuk", "kelek", "kezan", "kibil", "kinil", "kun", "kheled", "khelek", "khimil", "khuz", "laruk", "luz", "moran", "moril", "nibin", "nukul"};
std::vector<std::string> aSufDwarven = { "akar", "agul", "amen", "gib", "gol", "gog", "gul", "guluth", "gundil", "gundag", "guzun", "lib", "lizil", "loth", "mab", "mor", "mud", "mur", "nazar", "nigin", "niz", "nizil", "nuz", "nuzum", "thibil", "thizar", "ulin", "uzar", "uzun", "zad", "zakar", "zal", "zalak", "zam", "zan", "zaral", "zarak", "zeg", "zerek", "zibith", "zikil", "zokh", "zukum"};

std::vector<std::string> aPrefOrchish = { "Arg", "Az", "Bad", "Balkh", "Bol", "Dreg", "Dur", "Durba", "Ghash", "Lurg", "Luz", "Mor", "Nazg", "Og", "Tarkh", "Urg", "Ug", "Vol", "Yazh"};
std::vector<std::string> aSufOrchish = { "agal", "buz", "dor", "dur", "gar", "mog", "narb", "nazg", "rod", "shak", "waz", "ubal"};

std::vector<std::string> aPrefArabic = { "Aaza", "Abha", "Ad", "Aga", "Ah", "Ain", "Ait", "Ajda", "Ali", "Al", "Arrer", "As", "Ash", "Ay", "Az", "Bab", "Bani", "Bari", "Bat", "Birak", "Bitam", "Bou", "Dakh", "Dha", "Dham", "Djaz", "Djeb", "Fash", "Ghad", "Ghar", "Ghat", "Gra", "Had", "Ham", "Har", "Jawf", "Jer", "Jid", "Jir", "Kabir", "Kebir", "Ket", "Khat", "Khem", "Kher", "Khum", "Ksar", "Mak", "Mara", "Men", "Mu", "Qat", "Qay", "Sa", "Sab", "Sah", "Sal", "Sidi", "Sma", "Sulay", "Tabel", "Tar", "Tay", "Taza", "Ubay", "Wah", "Yab", "Yaf", "Yous", "Zil", "Zou"};
std::vector<std::string> aSufArabic = { "ada", "ah", "air", "ama", "amis", "aq", "ar", "ash", "at", "bala", "biya", "dah", "dir", "el", "faya", "fi", "fir", "ha", "hab", "ia", "idj", "ir", "is", "ja", "jel", "ka", "kah", "kha", "khari", "la", "lah", "ma", "na", "nen", "ra", "ran", "rar", "rata", "rin", "rem", "run", "sef", "sumah", "tar", "ya", "yan", "yil"};

std::vector<std::string> aPrefViking = { "al", "ber", "drammen", "grong", "hag", "hauge", "hed", "kinsar", "kol", "koper", "lin", "nas", "norr", "olof", "os", "Ost", "Oster", "skellef", "soder", "stal", "stavan", "stock", "tons", "trond", "vin"};
std::vector<std::string> aSufViking = { "fors", "gard", "heim", "holm", "lag", "mar", "marden", "mark", "stad", "strom"};

std::vector<std::string> aPrefHumans = { "basing", "birming", "black", "bland", "bletch", "brack", "brent", "bridge", "broms", "bur", "cam", "canter", "chelten", "chester", "col", "dor", "dun", "glaston", "grim", "grin", "harro", "hastle", "hels", "hemp", "herne", "horn", "hors", "hum", "ketter", "lei", "maiden", "marble", "mar", "mel", "new", "nor", "notting", "oak", "ox", "ports", "sher", "stam", "stan", "stock", "stroud", "tuan", "warring", "wind"};
std::vector<std::string> aSufHumans = { "dare", "don", "field", "ford", "grove", "ham", "hill", "lock", "mere", "moor", "ton", "vil", "wood"};

std::vector<std::string> aPrefInn = { "Bent", "Black", "Blind", "Blue", "Bob's", "Joe's", "Broken", "Buxom", "Cat's", "Crow's", "Dirty", "Dragon", "Dragon's", "Drunken", "Diamond", "Eagle's", "Eastern", "Falcon's", "Fawning", "Fiend's", "Flaming", "Frosty", "Frozen", "Gilded", "Genie's", "Golden", "Golden", "Gray", "Green", "King's", "Licked", "Lion's", "Iron", "Mended", "Octopus", "Old", "Old", "Orc's", "Pink", "Pot", "Puking", "Queen's", "Red", "Ruby", "Delicate", "Sea", "Sexy", "Shining", "Silver", "Singing", "Steel", "Strange", "Thirsty", "Violet", "White", "Wild", "Yawing"};
std::vector<std::string> aSufInn = { " Axe", " Anchor", " Barrel", " Basilisk", " Belly", " Blade", " Boar", " Breath", " Brew", " Claw", " Coin", " Delight", " Den", " Dragon", " Drum", " Dwarf", " Fist", " Flower", " Gem", " Gryphon", " Hand", " Head", " Hole", " Inn", " Lady", " Maiden", " Lantern", " Monk", " Mug", " Nest", " Orc", " Paradise", " Pearl", " Pig", " Pit", " Place", " Tavern", " Portal", " Ranger", " Rest", " Sailor", " Sleep", " Song", " Swan", " Swords", " Tree", " Unicorn", " Whale", " Wish", " Wizard", " Rain"};

std::vector<std::string> aPrefFort = { "Mind ", "Iron ", "Demention ", "Demonic ", "Blood ", "Mistery ", "Ancient ", "Doom ", "Black ", "Crimson ", "Blue ", "Eternal ", "Cursed ", "Funny ", "Stone", "Etherial ", "Phantom ", "Forgotten ", "King's ", "Queen's ", "Royal ", "Fallen", "Lost ", "Warrior's ", "Sorcerer's ", "Steel ", "Blademaster's ", "Screaming ", "Ice ", "Frozen ", "Dragon ", "Glorious ", "Infernal "};
std::vector<std::string> aSufFort = { "Storm", "Fist", "Keep", "Rage", "Rose", "Residence", "Mansion", "Haven", "Gates", "" };

std::vector<std::string> aPrefShip = { "Absolute", "Adventure", "Alisa", "Altered", "Amber", "Ancient", "Angel's", "Animal", "Another", "Azure", "Bad", "Bad Moon", "Betty", "Big", "Black", "Blue", "Breaking", "Crime", "Crimson", "Dancing", "Dark", "Dawn", "Dirty", "Distant", "Double", "Dragon", "Dream", "Emerald", "Empty", "Enchanted", "Exotic", "Extra", "Extreme", "Fallen", "Fast", "Fatal", "Fifth", "Final", "Fine", "Fire", "First", "Flying", "Foreign", "Fortune", "Funny", "Gentle", "Golden", "Grand", "Great", "Green", "Grey", "Gypsy", "Half", "Happy", "High", "Impossible", "Jade", "Little", "Lone", "Lucky", "Mad", "Mermaid", "Midnight", "Moon", "Morning", "Naked", "Naughty", "Naval", "New", "Night", "Ocean", "Old", "Pacific", "Perfect", "Pretty", "Quick", "Quiet", "Red", "Saint", "Sea", "Sapphire", "Second", "Silver", "Southern", "Stella", "Sun", "Sunset", "Sweet", "Third", "Thunder", "Treasure", "Ultimate", "Wave", "Zephyr", "Zodiac" };
std::vector<std::string> aSufShip = { " Adventure", " Amore", " Angel", " Answer", " Attraction", " Bird", " Boat", " Body", " Bound", " Boy", " Breaker", " Breeze", " Cat", " Catcher", " Chaser", " Courier", " Crusher", " Devil", " Diamond", " Dog", " Dolphin", " Dream", " Dreamer", " Eagle", " Elf", " Fish", " Flash", " Flight", " Fox", " Girl", " Ghost", " Goose", " Gull", " Hawk", " Huntress", " Hunter", " Jack", " Jane", " Jewel", " Jumper", " Karma", " King", " Kiss", " Knight", " Lady", " Lion", " Love", " Lover", " Madness", " Magic", " Marie", " Minstrel", " Mist", " Mistake", " Money", " Monkey", " Monster", " Nest", " Nightmare", " Owl", " Queen", " Quest", " Pig", " Pirate", " Plainsman", " Phantom", " Power", " Presence", " Prince", " Princess", " Rainbow", " Rising", " Rose", " Runner", " Scare", " Seeker", " Sight", " Sirena", " Sixteen", " Shadow", " Shift", " Shine", " Stalker", " Stripe", " Song", " Spirit", " Spice", " Star", " Storm", " Swan", " Tide", " Tiger", " Toy", " Trouble", " Turtle", " Viking", " Unicorn", " Walker", " Wind", " Wine", " Wish", " Witch", " Wizard", " White", " Wolf", " Woman", " Zebra" };

std::vector<std::string> aPrefFemale = { "Ail", "Ara", "Ay", "Bren", "Astar", "Dae", "Dren", "Dwen", "El", "Erin", "Eth", "Fae", "Fay", "Gae", "Gay", "Glae", "Gwen", "Il", "Jey", "Lae", "Lan", "Lin", "Mae", "Mara", "More", "Mi", "Min", "Ne", "Nel", "Pae", "Pwen", "Rae", "Ray", "Re", "Ri", "Si", "Sal", "Say", "Tae", "Te", "Ti", "Tin", "Tir", "Vi", "Vul" };
std::vector<std::string> aSufFemale = { "ta", "alle", "ann", "arra", "aye", "da", "dolen", "ell", "enn", "eth", "eya", "fa", "fey", "ga", "gwenn", "hild", "ill", "ith", "la", "lana", "lar", "len", "lwen", "ma", "may", "na", "narra", "navia", "nwen", "ola", "pera", "pinn", "ra", "rann", "rell", "ress", "reth", "riss", "sa", "shann", "shara", "shea", "shell", "tarra", "tey", "ty", "unn", "ura", "valia", "vara", "vinn", "wen", "weth", "wynn", "wyrr", "ya", "ye", "yll", "ynd", "yrr", "yth" };

std::vector<std::string> aPrefMale = { "ache", "aim", "bald", "bear", "cron", "boar", "boast", "boil", "boni", "boy", "bower", "churl", "corn", "cuff", "dark", "dire", "dour", "dross", "dupe", "dusk", "dwar", "dwarf", "ebb", "el", "elf", "fag", "fate", "fay", "fell", "fly", "fowl", "gard", "gay", "gilt", "girth", "glut", "goad", "gold", "gorge", "grey", "groan", "haft", "hale", "hawk", "haught", "hiss", "hock", "hoof", "hook", "horn", "kin", "kith", "lank", "leaf", "lewd", "louse", "lure", "man", "mars", "meed", "moat", "mould", "muff", "muse", "not", "numb", "odd", "ooze", "ox", "pale", "port", "quid", "rau", "red", "rich", "rob", "rod", "rud", "ruff", "run", "rush", "scoff", "skew", "sky", "sly", "sow", "stave", "steed", "swar", "thor", "tort", "twig", "twit", "vain", "vent", "vile", "wail", "war", "whip", "wise", "worm", "yip" };
std::vector<std::string> aSufMale = { "os", "ard", "bald", "ban", "baugh", "bert", "brand", "cas", "celot", "cent", "cester", "cott", "dane", "dard", "doch", "dolph", "don", "doric", "dower", "dred", "fird", "ford", "fram", "fred", "frid", "fried", "gal", "gard", "gernon", "gill", "gurd", "gus", "ham", "hard", "hart", "helm", "horne", "ister", "kild", "lan", "lard", "ley", "lisle", "loch", "man", "mar", "mas", "mon", "mond", "mour", "mund", "nald", "nard", "nath", "ney", "olas", "pold", "rad", "ram", "rard", "red", "rence", "reth", "rick", "ridge", "riel", "ron", "rone", "roth", "sander", "sard", "shall", "shaw", "son", "steen", "stone", "ter", "than", "ther", "thon", "thur", "ton", "tor", "tran", "tus", "ulf", "vald", "van", "vard", "ven", "vid", "vred", "wald", "wallader", "ward", "werth", "wig", "win", "wood", "yard" };

std::vector<std::string> aEpithetMageM = { "Black", "White", "Blue", "Green", "Brown", "Ruthless", "Heartless", "Ugly", "Mad", "Careless", "Restless", "Impartial", "Immortal", "Colourless", "Callous", "Cruel", "Powerful", "Mage", "Evil", "Weak", "Wize", "Handless", "Furious", "Flamer", "Malicius" };
std::vector<std::string> aEpithetMageF = { "Black", "White", "Blue", "Green", "Brown", "Ruthless", "Heartless", "Ugly", "Mad", "Careless", "Restless", "Impartial", "Immortal", "Colourless", "Callous", "Cruel", "Powerful", "Witch", "Evil", "Weak", "Wize", "Handless", "Furious", "Malicius" };
std::vector<std::string> aEpithetClericM = { "Good", "Merciful", "Compassionate", "Healer", "Magnanimous", "Saint", "High Priest", "Hermit", "Monk", "Brother", "Father", "Cleric", "Wizard", "Elder", "Deathkiller", "Painkiller", "Reviver", "Accurate", "Silver", "Snake", "Silent", "Quiet", "Kindest", "Childless", "Herbologist", "Alchemist", "Modest" };
std::vector<std::string> aEpithetClericF = { "Good", "Merciful", "Compassionate", "Healer", "Magnanimous", "Saint", "High Priestess", "Cleric", "Sorceress", "Sister", "Mother", "Elder", "Accurate", "Silver", "Beautiful", "Silent", "Quiet", "Kindest", "Childless", "Herbologist", "Alchemist" };
std::vector<std::string> aEpithetShamanM = { "Summoner", "Shaman", "Chanter", "Scull", "Claw", "Warlock", "Dragon", "Elder", "Incomer", "Dominator", "Beastlord", "Master", "Outsider", "Exiled", "Halfhuman", "Ancient", "Returned", "Banisher", "Lonely", "Seeker", "Wereman" };
std::vector<std::string> aEpithetShamanF = { "Dominatrix", "Mistress", "Black Widow", "Ancient", "Lonely", "Werewoman" };
std::vector<std::string> aEpithetGeneralM = { "Ruthless", "Bloody", "Mighty", "Wild", /*"Demigod",*/ "Lord", "Headcutter", "Executor", "Loud", "Prince", "King", "Don", "Commander", "Great", "Scullsmasher", "Big Axe", "Heavy Axe", "Heavy Fist", "Quick Sword", "Golden Sword", "Silver Sword", "Iron Sword", "Iron Fist", "Wooden Sword", "Sharp Sword", "Deadly Blade", "Shining Sword", "Black Sword", "First Sword", "Champion", "Big Mug", "Demon Hunter", "Big Helm", "Horny", "Dragonslayer", "Peacemaker", "Liberator", "Barbarian", "Dark Knight", "White Knight", "Glorious", "One-eye", "Butcher", "Murderer", "Capitan", "General", "Warlord", "Chief" };
std::vector<std::string> aEpithetGeneralF = { "Ruthless", "Bloody", "Mighty", "Wild", /*"Demigoddess",*/ "Executor", "Princess", "Queen", "Great", "Lady", "Quick Sword", "Golden Sword", "Silver Sword", "Iron Sword", "Wooden Sword", "Sharp Sword", "Deadly Blade", "Shining Sword", "Black Sword", "Demon Hunter", "Dragonslayer", "Glorious", "Capitan", "General", "Warlord", "Chief" };

//---------------------------------------------------------------------------

std::string getPrefix(std::vector<std::string>& prefixTable) {
    return rng::one_of(prefixTable);
}

std::string getSuffix(std::vector<std::string>& suffixTable) {
    return rng::one_of(suffixTable);
}

std::string getEthnicPrefix(Ethnicity etnos) {
    switch(etnos) {
        case Ethnicity::VIKING:    return getPrefix(aPrefViking);
        case Ethnicity::BARBARIAN: return getPrefix(aPrefScotish);
        case Ethnicity::MAN:       return getPrefix(aPrefHumans);
        case Ethnicity::ESKIMO:    return getPrefix(aPrefEscimo);
        case Ethnicity::NOMAD:     return getPrefix(aPrefArabic);
        case Ethnicity::TRIBESMAN: return getPrefix(aPrefAfrica);
        case Ethnicity::HIGHELF:   return getPrefix(aPrefElven1);
        case Ethnicity::ELF:       return getPrefix(aPrefElven2);
        case Ethnicity::DWARF:     return getPrefix(aPrefDwarven);
        case Ethnicity::ORC:       return getPrefix(aPrefOrchish);
        case Ethnicity::LIZARDMAN: return getPrefix(aPrefGreek);
        case Ethnicity::DROW:      return getPrefix(aPrefDrow);
        case Ethnicity::TITAN:     return getPrefix(aPrefAztec);
        default:                   return getPrefix(aPrefMale);
    }
}

std::string getEthnicSuffix(Ethnicity etnos) {
    switch(etnos) {
        case Ethnicity::VIKING:    return getSuffix(aSufViking);
        case Ethnicity::BARBARIAN: return getSuffix(aSufScotish);
        case Ethnicity::MAN:       return getSuffix(aSufHumans);
        case Ethnicity::ESKIMO:    return getSuffix(aSufEscimo);
        case Ethnicity::NOMAD:     return getSuffix(aSufArabic);
        case Ethnicity::TRIBESMAN: return getSuffix(aSufAfrica);
        case Ethnicity::HIGHELF:   return getSuffix(aSufElven1);
        case Ethnicity::ELF:       return getSuffix(aSufElven2);
        case Ethnicity::DWARF:     return getSuffix(aSufDwarven);
        case Ethnicity::ORC:       return getSuffix(aSufOrchish);
        case Ethnicity::LIZARDMAN: return getSuffix(aSufGreek);
        case Ethnicity::DROW:      return getSuffix(aSufDrow);
        case Ethnicity::TITAN:     return getSuffix(aSufAztec);
        default:                   return getSuffix(aSufMale);
    }
}

std::string getAbstractName() {
    std::string first = getPrefix(aPrefAbstract);
    std::string second = getSuffix(aSufAbstract);

    return (first + second) | filter::capitalize;
}

std::string getEthnicName(Ethnicity etnos) {
    std::string first = getEthnicPrefix(etnos);
    std::string second = getEthnicSuffix(etnos);

    return (first + second) | filter::capitalize;
}

std::string getShipName() {
    std::string first = getPrefix(aPrefShip);
    std::string second = getSuffix(aSufShip);

    return (first + second) | filter::capitalize;
}

std::string getFortressName(const ObjectType& type) {
    std::string first = getPrefix(aPrefFort);
    std::string second = getSuffix(aSufFort);

    if (second.empty()) {
        second = type.name;
    }

    return (first + second) | filter::capitalize;
}

std::string getInnName() {
    std::string first = getPrefix(aPrefInn);
    std::string second = getSuffix(aSufInn);

    return (first + second) | filter::capitalize;
}

std::string getObjectName(const int typeIndex, const ObjectType& type) {
    switch(typeIndex)
    {
        case O_DUMMY:
            break;

        case O_LONGBOAT:
        case O_CLIPPER:
        case O_GALLEON:
        case O_BALLOON:
        case O_AGALLEON:
        case O_DERELICT:
            return getShipName();

        case O_TOWER:
        case O_FORT:
        case O_CASTLE:
        case O_CITADEL:
        case O_MCASTLE:
        case O_MCITADEL:
        case O_MTOWER:
        case O_PALACE:
        case O_STOCKADE:
        case O_CPALACE:
        case O_HTOWER:
        case O_MAGETOWER:
        case O_DARKTOWER:
        case O_GIANTCASTLE:
        case O_HPTOWER:
            return getFortressName(type);

        case O_SHAFT:
        case O_LAIR:
        case O_RUIN:
        case O_CAVE:
        case O_DEMONPIT:
        case O_CRYPT:
        case O_MINE:
        case O_FARM:
        case O_RANCH:
        case O_TIMBERYARD:
        case O_QUARRY:
        case O_MQUARRY:
        case O_AMINE:
        case O_PRESERVE:
        case O_SACGROVE:
        case O_TRAPPINGHUT:
        case O_STABLE:
        case O_MSTABLE:
        case O_TRAPPINGLODGE:
        case O_FAERIERING:
        case O_ALCHEMISTLAB:
        case O_OASIS:
        case O_GEMAPPRAISER:
                break;

        case O_INN:
            return getInnName();

        case O_ISLE:
        case O_OCAVE:
        case O_WHIRL:
            break;

        case O_ROADN:
        case O_ROADNW:
        case O_ROADNE:
        case O_ROADSW:
        case O_ROADSE:
        case O_ROADS:
            return "Ancient Road";

        case O_TEMPLE:
        case O_BKEEP:
        case O_DCLIFFS:
        case O_HUT:
        case O_NGUILD:
        case O_AGUILD:
        case O_ATEMPLE:
        case O_ILAIR:
        case O_ICECAVE:
        case O_BOG:
            break;
    }

    return type.name;
}

std::string getForestName(std::string s, int area) {
    if (area < 15) {
        return s + rng::one_of({" Forest", " Woods"});
    }

    return "Great " + s + " Forest";
}

std::string getJungleName(std::string s, int area) {
    if (area < 15) {
        return s + rng::one_of({" Jungle", " Woods"});
    }

    return "Great " + s + " Jungle";
}

std::string getDesertName(std::string s, int area) {
    if (area < 15) {
        return s + rng::one_of({" Desert", " Sands"});
    }

    return "Great " + s + " Desert";
}

std::string getVolcanoName(std::string s) {
    return s + rng::one_of({" Volcano", " Peak"});
}

std::string getMountainName(std::string s, int area) {
    if (area == 1) {
        return s + rng::one_of({" Mountain", " Peak"});
    }

    if (area < 15) {
        return s + rng::one_of({" Mountains", " Heights", " Rocks"});
    }

    return "Great " + s + " Mountains";
}

std::string getHillsName(std::string s, int area) {
    if (area == 1) {
        return s + rng::one_of({" Hill", " Barrow"});
    }

    if (area < 15) {
        return s + rng::one_of({" Hills", " Barrows", " Heights"});
    }

    return "Great " + s + " Hills";
}

std::string getSwampName(std::string s, int area) {
    if (area < 15) {
        return s + rng::one_of({" Swamp", " Marshes"});
    }

    return "Great " + s + " Swamp";
}

std::string getPlainName(std::string s, int area) {
    if(area == 1) {
        return s + rng::one_of({" Dale", " Plain"});
    }

    if (area < 15) {
        return s + rng::one_of({" Plains", " Valley"});
    }

    return "Great " + s + " Plains";
}

std::string getTundraName(std::string s) {
    return s + " Tundra";
}

std::string getOceanName(std::string s, int area) {
    if(area == 1) {
        return s + rng::one_of({" Lake", " Pond"});
    }

    if (area < 15) {
        return s + " Sea";
    }

    return s + " Ocean";
}

std::string getRegionName(const Ethnicity etnos, const int type, const int size, const bool island) {
    std::string name = getEthnicName(etnos);

    if (island) {
        return name + " Island";
    }

    switch(type)
    {
        case R_FOREST:
        case R_UFOREST:
        case R_CERAN_FOREST1:
        case R_CERAN_FOREST2:
        case R_CERAN_FOREST3:
        case R_CERAN_MYSTFOREST:
        case R_CERAN_MYSTFOREST1:
        case R_CERAN_MYSTFOREST2:
        case R_CERAN_UFOREST1:
        case R_CERAN_UFOREST2:
        case R_CERAN_UFOREST3:
        case R_DFOREST:
        case R_CERAN_DFOREST1:
            return getForestName(name, size);

        case R_JUNGLE:
        case R_CERAN_JUNGLE1:
        case R_CERAN_JUNGLE2:
        case R_CERAN_JUNGLE3:
            return getJungleName(name, size);

        case R_DESERT:
        case R_CERAN_DESERT1:
        case R_CERAN_DESERT2:
        case R_CERAN_DESERT3:
            return getDesertName(name, size);

        case R_VOLCANO:
            return getVolcanoName(name);

        case R_MOUNTAIN:
        case R_ISLAND_MOUNTAIN:
        case R_CERAN_MOUNTAIN1:
        case R_CERAN_MOUNTAIN2:
        case R_CERAN_MOUNTAIN3:
            return getMountainName(name, size);

        case R_CERAN_HILL:
        case R_CERAN_HILL1:
        case R_CERAN_HILL2:
            return getHillsName(name, size);

        case R_SWAMP:
        case R_ISLAND_SWAMP:
        case R_CERAN_SWAMP1:
        case R_CERAN_SWAMP2:
        case R_CERAN_SWAMP3:
            return getSwampName(name, size);

        case R_PLAIN:
        case R_ISLAND_PLAIN:
        case R_CERAN_PLAIN1:
        case R_CERAN_PLAIN2:
        case R_CERAN_PLAIN3:
            return getPlainName(name, size);

        case R_TUNDRA:
        case R_CERAN_TUNDRA1:
        case R_CERAN_TUNDRA2:
        case R_CERAN_TUNDRA3:
            return getTundraName(name);

        case R_OCEAN:
            return getOceanName(name, size);

        default:
            return getAbstractName();
    }
}

std::string getRiverName(const int size, const int min, const int max) {
    std::string s = getAbstractName();

    int d = max - min;
	int greatRiver = max - d / 3;
    if (size >= greatRiver) {
        s = "Great " + s;
    }

    s += " River";

    return s;
}
