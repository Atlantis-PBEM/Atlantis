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

// Make sure this is correct.   The default is 1000 towns and 1000 regions.
#define NUMBER_OF_TOWNS 1000

static char const *regionnames[] =
{
    "A'irhin",
    "A'vespol",
    "Abernecht",
    "Abernethy",
    "Abeuton",
    "Abrenton",
    "Achash",
    "Achoep't",
    "Achydale",
    "Ackisse",
    "Adem",
    "Adoen'ph",
    "Agemos",
    "Aigiaton",
    "Aikoburg",
    "Aildadale",
    "Aisheaberg",
    "Alabrin-a-grim",
    "Alabrin-a-karak",
    "Alabrin-a-thol",
    "Alabrin-ban",
    "Alabrin-dor",
    "Alabrin-killuk",
    "Alabrin-lum",
    "Alabrin-tor",
    "Alabrin-ungol",
    "Aleberg",
    "Alettin",
    "Aleuld",
    "Aleym",
    "Altenarchen",
    "Altengrad",
    "Altenheim",
    "Altenhowe",
    "Altenmar",
    "Altenmark",
    "Altenmeir",
    "Altenten",
    "Altenvoltan",
    "Aman-a-gun",
    "Aman-a-karak",
    "Aman-a-lin",
    "Aman-dor",
    "Aman-killuk",
    "Aman-krag",
    "Aman-tor",
    "Aman-ungol",
    "Anneeton",
    "Ar'lor",
    "Ardden",
    "Ardeton",
    "Ardorm",
    "Ardvale",
    "Arouton",
    "Arreydale",
    "Asaberg",
    "Asend",
    "Ashdim",
    "Askul-a-grim",
    "Askul-a-luk",
    "Askul-a-menak",
    "Askul-dum",
    "Askul-tor",
    "Askul-ungol",
    "Asouton",
    "Asyust",
    "Athatgost",
    "Athbrod",
    "Athbrodtor",
    "Athbroduen",
    "Athdor",
    "Athen",
    "Athfin",
    "Athfindor",
    "Atoodale",
    "Aubuiville",
    "Aughsale",
    "Auqeiberg",
    "Aweville",
    "Awych",
    "Aynedale",
    "Ayrayberg",
    "Bach",
    "Baeshieton",
    "Bal-a-menak",
    "Bal-ban",
    "Bal-dor",
    "Bal-kai",
    "Bal-krag",
    "Banon",
    "Banyc",
    "Bekr",
    "Beldani",
    "Beldorf",
    "Belelt",
    "Belfelt",
    "Belgrad",
    "Belholm",
    "Belmar",
    "Belport",
    "Belten",
    "Belvoltan",
    "Benlech",
    "Berriedale",
    "Bi-bet",
    "Bihat",
    "Blelscha",
    "Bleuroiberg",
    "Bochest",
    "Boseedale",
    "Bray-whoed",
    "Breuneuburg",
    "Breunt",
    "Brilr",
    "Brimathanwe",
    "Brimaund",
    "Brimdiadel",
    "Brimdor",
    "Brimfinanfel",
    "Brimfinin",
    "Brimgoldon",
    "Brimgost",
    "Brimloroth",
    "Brimoldon",
    "Brimsordon",
    "Brimsormar",
    "Brimthanund",
    "Brok-ban",
    "Brupque",
    "Bugblod",
    "Bugburg",
    "Buk-a-for",
    "Buk-a-grim",
    "Buk-a-gun",
    "Buk-a-luk",
    "Buk-a-menak",
    "Buk-a-thol",
    "Buk-kai",
    "Buk-mun",
    "Buk-tor",
    "Bulss",
    "Burbruk",
    "Burdan",
    "Burdotn",
    "Burer",
    "Burfeld",
    "Burhate",
    "Burheimport",
    "Burmark",
    "Burmund",
    "Burn",
    "Burrgtur",
    "Burscough",
    "Burten",
    "Burvoltan",
    "Busst",
    "By-loep",
    "Byvneld",
    "Cay-rouc",
    "Cedaiburg",
    "Ceiqoeville",
    "Cekin",
    "Cerlat",
    "Cesheville",
    "Ceydayburg",
    "Chai-touss",
    "Chakal",
    "Chaold",
    "Charyn",
    "Chelauton",
    "Chordeburg",
    "Chraisouton",
    "Chres",
    "Chretlt",
    "Chrordrreld",
    "Chryas",
    "Chryqyville",
    "Cirannost",
    "Ciranoth",
    "Cirdiadel",
    "Cirfinar",
    "Cirfingost",
    "Cirlorar",
    "Cirmar",
    "Cirmarmar",
    "Cirreyton",
    "Cirrond",
    "Cirsoranwe",
    "Cirsorrond",
    "Cirsorund",
    "Co-cil",
    "Cobel",
    "Cochtai",
    "Coemeeton",
    "Conandon",
    "Condialun",
    "Conolin",
    "Conrollun",
    "Conthiel",
    "Corarchen",
    "Corbruk",
    "Corburg",
    "Cordotn",
    "Corhowe",
    "Corlanque",
    "Cormar",
    "Corport",
    "Corten",
    "Craumeudale",
    "Crawnkim",
    "Creibeyville",
    "Cretin",
    "Crirtoeberg",
    "Cryage",
    "Crylim",
    "Da-teay",
    "Danug",
    "Darbruk",
    "Darburg",
    "Dardorf",
    "Darfelt",
    "Dargrad",
    "Darheim",
    "Darholm",
    "Darill",
    "Darrshy",
    "Darstad",
    "Darvoltan",
    "Dea-phount",
    "Deeneuburg",
    "Del'ougha",
    "Delgnal",
    "Denche",
    "Denhver",
    "Densen",
    "Denum",
    "Descton",
    "Detan",
    "Die-meiq",
    "Die-tand",
    "Diseville",
    "Doedbygd",
    "Donnbrun",
    "Doratoth",
    "Dorbroddor",
    "Dorenath",
    "Dorfinanfel",
    "Dospan",
    "Drae-phen",
    "Draon",
    "Drogburg",
    "Droggrod",
    "Drogungol",
    "Drumcollogher",
    "Duath",
    "Duikouburg",
    "Dun-a-for",
    "Dun-a-grim",
    "Dun-a-gun",
    "Dun-a-menak",
    "Dun-dum",
    "Dun-kai",
    "Dun-krag",
    "Dun-lum",
    "Dunarchen",
    "Dunbeath",
    "Dunbruk",
    "Duncansby",
    "Dunfanaghy",
    "Dunfeld",
    "Dunhowestad",
    "Duniville",
    "Dunmar",
    "Dunmeir",
    "Dunstad",
    "Dunvoltan",
    "Dur-a-dum",
    "Dur-a-for",
    "Dur-a-grim",
    "Dur-a-gun",
    "Dur-a-lin",
    "Dur-a-menak",
    "Dur-a-thol",
    "Dur-dum",
    "Dur-kai",
    "Dur-mun",
    "Dwor-a-dum",
    "Dwor-a-lin",
    "Dwor-a-luk",
    "Dwor-ungol",
    "Dyckkel",
    "Dynati",
    "Dynayr'a",
    "Dynryno",
    "Dynvery",
    "Dyqua",
    "E'delar",
    "E'vertur",
    "Eakoedale",
    "Eaphaidale",
    "Echrang",
    "Echuy",
    "Eedoiberg",
    "Eemauton",
    "Eephiadale",
    "Einarchen",
    "Einbruk",
    "Einburg",
    "Eindorf",
    "Einfelt",
    "Eingrad",
    "Einheim",
    "Einmund",
    "Einstad",
    "Eintenheim",
    "Eirraeville",
    "Eisedale",
    "Eiyodale",
    "Eldiamar",
    "Elfinund",
    "Ellorgost",
    "Eloanfel",
    "Eloat",
    "Eloatanfel",
    "Eloinin",
    "Eloolanwe",
    "Elosordor",
    "Elosornost",
    "Elothananfel",
    "Elothantor",
    "Elothielath",
    "Elothieldel",
    "Elotor",
    "Eltayton",
    "Em'enth'ilt",
    "Em'urny",
    "Emata",
    "Emeld",
    "Emez",
    "Emtkal",
    "Endauen",
    "Enddordor",
    "Endforar",
    "Endfordel",
    "Endlorgost",
    "Endthanath",
    "Endthielath",
    "Endthielnost",
    "Enormo",
    "Entiny",
    "Enuville",
    "Esheaberg",
    "Esidale",
    "Essblor",
    "Esttend",
    "Estundo",
    "Estyz",
    "Eter'ck",
    "Ethilath",
    "Ethilatuen",
    "Ethilenmar",
    "Ethilfinlun",
    "Ethilfintor",
    "Ethilfordor",
    "Ethilforlun",
    "Ethilmarar",
    "Ethilor",
    "Ethilthielar",
    "Ethilthielath",
    "Etiburg",
    "Eubiton",
    "Euldiberg",
    "Eurauberg",
    "Eyldaton",
    "Eynaudale",
    "Eyneberg",
    "Faetooton",
    "Fo-rhes",
    "Fontenbrun",
    "Forbrin-a-lin",
    "Forbrin-a-thol",
    "Forbrin-dum",
    "Forbrin-killuk",
    "Forbrin-lum",
    "Forbrin-mun",
    "Forbrin-tor",
    "Forbrodnost",
    "Forfindon",
    "Forgolost",
    "Forloroth",
    "Frainberg",
    "Fundslye",
    "Garoldi",
    "Gei-steit",
    "Gha'ech'ounn",
    "Githaanwe",
    "Githanost",
    "Githathgost",
    "Githatund",
    "Githbrodost",
    "Githdiator",
    "Githforlun",
    "Githolor",
    "Githsoror",
    "Giththanost",
    "Glenagallagh",
    "Glorforgost",
    "Gnackstein",
    "Gon-a-grim",
    "Gon-a-karak",
    "Gon-kai",
    "Gon-killuk",
    "Gon-tor",
    "Gonas",
    "Graevbygd",
    "Gragburg",
    "Grandotn",
    "Granfeld",
    "Granfelt",
    "Grangrad",
    "Granheim",
    "Granhowe",
    "Granmar",
    "Granmark",
    "Granmund",
    "Granport",
    "Granstadfeld",
    "Granten",
    "Gresberg",
    "Grimburg",
    "Grimgrod",
    "Grisbygd",
    "Gycer",
    "Haphr",
    "Hathden",
    "Hatria",
    "Helarchen",
    "Helbruk",
    "Heldorf",
    "Helfeld",
    "Helfelt",
    "Helheim",
    "Helhowe",
    "Helmsdale",
    "Helmund",
    "Helten",
    "Hiraor",
    "Hiratund",
    "Hirdiagost",
    "Hirdor",
    "Hirindor",
    "Hirrolor",
    "Hirsorost",
    "Hirthieldon",
    "Hirthielgost",
    "Ho-luid",
    "Ho-zak",
    "Hoersalsveg",
    "Holeiville",
    "Honess",
    "Hootauburg",
    "Huilooton",
    "Hynodale",
    "I'rothnal",
    "Iadaeburg",
    "Iaia",
    "Iaierd's",
    "Iamayberg",
    "Ianteydale",
    "Iarada",
    "Iaryton",
    "Ieteuville",
    "Ietooton",
    "Ightonn",
    "Ightrady",
    "Ikoville",
    "Imacho",
    "Imad",
    "Immtur",
    "Inadon",
    "Inaet",
    "Inaid",
    "Inath",
    "Inathanwe",
    "Inaveri",
    "Inbrodin",
    "Ineault'y",
    "Ineed'u",
    "Ingaik't",
    "Ingard'w",
    "Ingoeq'o",
    "Ingolnost",
    "Ingtiao",
    "Inlorin",
    "Inlorlun",
    "Intaiton",
    "Inuen",
    "Iphouville",
    "Irom'n",
    "Irord",
    "Irteaville",
    "Isbur",
    "Issac",
    "Issel",
    "Ithrag-a-gun",
    "Ithrag-a-thol",
    "Ithrag-killuk",
    "Ithrag-krag",
    "Itis",
    "Iyoton",
    "Jailouton",
    "Jervbygd",
    "Joerruberg",
    "Joveaburg",
    "Jydel",
    "Kae-yik",
    "Kar-a-dum",
    "Kar-a-karak",
    "Kar-a-lin",
    "Kar-killuk",
    "Kar-mun",
    "Karaz-a-gun",
    "Karaz-a-karak",
    "Karaz-a-luk",
    "Karaz-kai",
    "Karaz-killuk",
    "Karaz-tor",
    "Karaz-ungol",
    "Kargslag",
    "Kau-throos",
    "Ke-throuz",
    "Kedsul",
    "Keltver",
    "Kim'eng",
    "Kinccer",
    "Kintura",
    "Ko-stran",
    "Kuileyton",
    "Kul-a-lin",
    "Kul-a-thol",
    "Kul-kai",
    "Kul-lum",
    "Landldtas",
    "Landtorm",
    "Langholm",
    "Larusk",
    "Lau-sment",
    "Lee-rhaig",
    "Lei-neac",
    "Leiroodale",
    "Lemaberg",
    "Lephngha",
    "Leu-sarra",
    "Li-laif",
    "Linarchen",
    "Linfeld",
    "Linfelt",
    "Lingrad",
    "Linhowe",
    "Linmark",
    "Linmeir",
    "Linport",
    "Linstad",
    "Linten",
    "Linvoltan",
    "Linvoltanstad",
    "Llathmsul",
    "Llau-kaiss",
    "Lloyer",
    "Lo-riag",
    "Loiss",
    "Loo-luk",
    "Lorador",
    "Lorfinath",
    "Lorforin",
    "Lormarnost",
    "Lorol",
    "Lorrolnost",
    "Lortkel",
    "Lostoodale",
    "Lothatar",
    "Lothatlun",
    "Lothenlun",
    "Lothforor",
    "Lothinath",
    "Lothmar",
    "Lothmarund",
    "Lotholmar",
    "Lothrolnost",
    "Lothsormar",
    "Lothsorost",
    "Lou-jort",
    "Lu-drep",
    "Ludnt",
    "Lyeissy",
    "Lyekalo",
    "Lyeoso",
    "Lynton",
    "Magdenarchen",
    "Magdenburg",
    "Magdenfelt",
    "Magdengrad",
    "Magdenholm",
    "Magdenmar",
    "Magdenmark",
    "Magdenten",
    "Magdenvoltan",
    "Mararchen",
    "Marburg",
    "Mardotn",
    "Marfeld",
    "Marheim",
    "Marholm",
    "Marmark",
    "Marmeir",
    "Marmund",
    "Marport",
    "Marsbrun",
    "Marsburg",
    "Mautayville",
    "Melq",
    "Meqiville",
    "Mikedale",
    "Morer",
    "Mosghae",
    "Mullaghcarn",
    "Mun-a-dol",
    "Mun-a-dum",
    "Mun-a-karak",
    "Mun-a-luk",
    "Mun-a-menak",
    "Mun-ban",
    "Mun-dum",
    "Mun-kai",
    "Mun-krag",
    "Mun-mun",
    "Mun-tor",
    "Mundarchen",
    "Munddotn",
    "Mundfeld",
    "Mundfelt",
    "Mundholm",
    "Mundmar",
    "Mundmeir",
    "Mundmund",
    "Mundport",
    "Mundvoltan",
    "Mustlor",
    "Mym",
    "N'omiss",
    "Naild",
    "Naqrd",
    "Narga-a-dum",
    "Narga-a-gun",
    "Narga-a-karak",
    "Narga-dor",
    "Narga-lum",
    "Narga-tor",
    "Nargburg",
    "Narggog",
    "Navenby",
    "Ne-coiss",
    "Nellvit",
    "Nepler",
    "Netos",
    "Neuarchen",
    "Neubruk",
    "Neuburg",
    "Neufelt",
    "Neuheim",
    "Neuholm",
    "Neuhowe",
    "Neumeir",
    "Neumund",
    "Ney-brer",
    "Niavauton",
    "Nildss",
    "Nima",
    "Nimathgost",
    "Nimdoranwe",
    "Nimfortor",
    "Nimloror",
    "Nimolrond",
    "Nimthanlun",
    "Nimthiel",
    "Nimthielanfel",
    "No-tan",
    "Nolaville",
    "Nomeaburg",
    "Norarchen",
    "Norburg",
    "Normeir",
    "Norport",
    "Norstad",
    "Norten",
    "Nuchr",
    "Nugm",
    "Nyaec'p",
    "Nyon",
    "O'arbur",
    "Oephoville",
    "Oigeadale",
    "Oiloedale",
    "Oimiaton",
    "Okaberg",
    "Oldbtur",
    "Oliaburg",
    "Ollo-a-for",
    "Ollo-a-gun",
    "Ollo-a-lin",
    "Ollo-a-luk",
    "Ollo-ban",
    "Ollo-dor",
    "Ollo-krag",
    "Ollo-ungol",
    "Oltouton",
    "Omeerr'a",
    "Omert",
    "Omlir",
    "Onaberg",
    "Ondeydale",
    "Onym",
    "Onyq",
    "Oontaedale",
    "Oopyberg",
    "Oosadale",
    "Ootaton",
    "Oqauton",
    "Orart",
    "Ordaville",
    "Orm'ser",
    "Ormfmor",
    "Ormgryte",
    "Ormwor",
    "Oroldu",
    "Orrebygd",
    "Orrooville",
    "Orys",
    "Osdelu",
    "Oseberg",
    "Osray",
    "Oufodale",
    "Oughang",
    "Oughen",
    "Ounuton",
    "Phyrdynal",
    "Pirrtor",
    "Pogelveir",
    "Poghkaugh",
    "Poloon'u",
    "Polrayo",
    "Porthcawl",
    "Praestbygd",
    "Qua-stass",
    "Quataia",
    "Quaundeville",
    "Quayfeeton",
    "Quiage",
    "Quivuberg",
    "Quoraeberg",
    "Quynnoedale",
    "Radeng",
    "Radiaanwe",
    "Radol",
    "Radwary",
    "Raennost",
    "Ragolmar",
    "Rak'ves",
    "Rakhat",
    "Ramaroth",
    "Ran'kal'aet",
    "Ranlvor",
    "Rappbygd",
    "Rasorath",
    "Ravsurn",
    "Raypero",
    "Reilburg",
    "Rephv",
    "Reycaberg",
    "Ri-bliz",
    "Ri-ruif",
    "Riecburg",
    "Riecdorf",
    "Riecfelt",
    "Riecheim",
    "Riecholm",
    "Riecmeir",
    "Riecstad",
    "Riecvoltan",
    "Rieloidale",
    "Rilad",
    "Rillen",
    "Riltas",
    "Rirdieberg",
    "Robaedale",
    "Rodmosa",
    "Roi-nael",
    "Rosh",
    "Ruage",
    "Rudoeton",
    "Rudyburg",
    "Rulale",
    "Ruran",
    "Rusrddyn",
    "Ruzb",
    "Ryssler",
    "Sa-lan",
    "Sacphin",
    "Sae-wim",
    "Sam'iae",
    "Samath",
    "Saran",
    "Sas",
    "Say-sweyld",
    "Sch'oskin",
    "Schai-reuf",
    "Scheemaburg",
    "Schiesieburg",
    "Schockdorm",
    "Schodild",
    "Schointoville",
    "Seathayburg",
    "Seedadale",
    "Seetouburg",
    "Sei-yuss",
    "Sei-zhet",
    "Shasnlye",
    "Shasz",
    "Shookeedale",
    "Shul",
    "Shy'est'ud",
    "Shyelda",
    "Shykssit",
    "Siqness",
    "Sirshaugh",
    "Sissul",
    "Skokholm",
    "Skottskog",
    "Sleckz",
    "Slelt",
    "Slethkden",
    "Slisuburg",
    "Sluen",
    "Smagoberg",
    "Smauleaton",
    "Smianoville",
    "Smuchrris",
    "Smyzlia",
    "Sneyleeberg",
    "Snoi-theyn",
    "Somat",
    "Soo-tyn",
    "Soqmold",
    "Soranwe",
    "Soratath",
    "Soratgost",
    "Sorathanfel",
    "Sordorost",
    "Sorfindon",
    "Sorindel",
    "Sorlorar",
    "Soroldel",
    "Sorthandon",
    "Sorthanuen",
    "Sost",
    "Stay-blin",
    "Steesaville",
    "Stram",
    "Strardouton",
    "Stroqough",
    "Stugslett",
    "Sul'em'uq",
    "Swae-raind",
    "Swedtves",
    "Swie-sar",
    "Sychser",
    "Tabck",
    "Tai'urn'ys",
    "Tailiton",
    "Tan'at",
    "Tanaughe",
    "Tanbald",
    "Tar-a-for",
    "Tar-a-lin",
    "Tar-a-luk",
    "Tar-dor",
    "Tar-dum",
    "Tar-krag",
    "Tar-tor",
    "Tar-ungol",
    "Tas'bur",
    "Tha-seurr",
    "Thaifooton",
    "Thandiaanfel",
    "Thandianost",
    "Thanmaranfel",
    "Thanthielnost",
    "Ther'awu",
    "Ther'nal",
    "Thereck",
    "Therl",
    "Thidest",
    "Tho-phiag",
    "Thor-a-for",
    "Thor-a-thol",
    "Thor-dor",
    "Thor-kai",
    "Thor-tor",
    "Thr'taiton",
    "Thradiedale",
    "Threves",
    "Thrindedale",
    "Throk",
    "Throzmunt",
    "Thrysay",
    "Thyrkim",
    "Tiault",
    "Tinien'nt",
    "Tinynt",
    "Tiphklye",
    "Tonisi",
    "Tonraky",
    "Tonusty",
    "Toraville",
    "Tourburg",
    "Tovale",
    "Trabddel",
    "Tral",
    "Treasaeton",
    "Treen",
    "Treshrd",
    "Triasheuton",
    "Trydt",
    "Tui-trouph",
    "Tur'kel",
    "Turfold",
    "Turturn",
    "Turuy",
    "U'anghon",
    "Uissaeburg",
    "Und'em",
    "Und'ris'ian",
    "Undildo",
    "Unnuiton",
    "Uriberg",
    "Useville",
    "Ustayl'rt",
    "Ustoist'ch",
    "Uxelberg",
    "Uzburg",
    "Verburg",
    "Verdorf",
    "Verholm",
    "Vermar",
    "Vermark",
    "Verstad",
    "Vesler",
    "Voiyeuville",
    "Volarchen",
    "Volburg",
    "Voldorf",
    "Volfeld",
    "Volgrad",
    "Volholm",
    "Volhowe",
    "Volvoltan",
    "Vor'ary",
    "W'tiaenth",
    "Waidbruk",
    "Waiddorf",
    "Waidgrad",
    "Waidheim",
    "Waidholm",
    "Waidmar",
    "Waidmeir",
    "Waidten",
    "Wastenburg",
    "Wastendotnfeld",
    "Wastenfelt",
    "Wastenheim",
    "Wastenholm",
    "Wastenmar",
    "Wastenmund",
    "Wastenvoltan",
    "Wenghmor",
    "Wh'inekal",
    "Whoezeaburg",
    "Wiadeuberg",
    "Worad",
    "Worir",
    "Yeroll",
    "Yertiss",
    "Yleaberg",
    "Yoneiburg",
    "Yqauton",
    "Yriaberg",
    "Zh'ostas",
    "Zho-raud",
    "Zigar",
    "Zogburg",
    "A'emwor",
    "A'rilurn",
    "Aathon",
    "Aberaeron",
    "Aberdaron",
    "Aberdovey",
    "Abersoch",
    "Abrantes",
    "Achen",
    "Achuimi",
    "Ackler",
    "Adrano",
    "AeBrey",
    "Age'acho",
    "Age'radi",
    "Ageen",
    "Aghleam",
    "Aisbrun",
    "Aisfor",
    "Aisgrois",
    "Aisnuon",
    "Aispuit",
    "Akbou",
    "Aldage",
    "Aldan",
    "Aleccer",
    "Aletiss",
    "Alfaro",
    "Alghero",
    "Almeria",
    "Altnaharra",
    "Ancroft",
    "Ang'vora",
    "Anom",
    "Anque",
    "Anran",
    "Anshun",
    "Anstruther",
    "Antor",
    "Arbroath",
    "Arcila",
    "Ardfert",
    "Ardraka",
    "Ardurn",
    "Arezzo",
    "Ariano",
    "Arlon",
    "Asay",
    "Ash'sul",
    "Ashblod",
    "Ashdrog",
    "Ashgog",
    "Ashrot",
    "Ashsay",
    "Ashshybel",
    "Ashslag",
    "Ashuk",
    "Ashwaz",
    "Asjild",
    "Asnrak",
    "Asonack",
    "Ater",
    "Athmardor",
    "Athrust",
    "Aughaugha",
    "Avanos",
    "Aveiro",
    "Awtorm",
    "Badalona",
    "Baechahoela",
    "Baest",
    "Baiaisle",
    "Baibrun",
    "Baibur",
    "Baiburbur",
    "Baidinon",
    "Baifel",
    "Baifelgris",
    "Baigris",
    "Baimais",
    "Bainuon",
    "Baipuit",
    "Bairiennegrois",
    "Ballindine",
    "Balta",
    "Bancest",
    "Banlar",
    "Barika",
    "Bastak",
    "Bayonne",
    "Beaubur",
    "Beaudinon",
    "Beaufor",
    "Bejaia",
    "Bel'em",
    "Belver",
    "Bemeniky",
    "Beragh",
    "Bergland",
    "Berneray",
    "Berori",
    "Binhai",
    "Birde",
    "Bocagobi",
    "Bocholt",
    "Bogbad",
    "Bogdrog",
    "Bogmadie",
    "Bogrot",
    "Bogslag",
    "Boguz",
    "Bopol",
    "Borodit",
    "Braga",
    "Branbad",
    "Branblod",
    "Brandor",
    "Brangrod",
    "Branrun",
    "Branshak",
    "Branthang",
    "Branthor",
    "Brechlin",
    "Brimdordel",
    "Briminnost",
    "Brimmarmar",
    "Brimolanwe",
    "Brimost",
    "Brodick",
    "Bugbad",
    "Bugdor",
    "Bugungol",
    "Bugwaz",
    "Burgare",
    "Burongha",
    "Buyet",
    "Cadel",
    "Calpio",
    "Canna",
    "Capperwe",
    "Caprera",
    "Carahue",
    "Carbost",
    "Carnforth",
    "Carrigaline",
    "Caserta",
    "Cathcer",
    "Catrianchi",
    "Cerrir",
    "Ch'therir",
    "Cheath",
    "Chedaru",
    "Chedely",
    "Chesuli",
    "Cheswor",
    "Chiem",
    "Cimtas",
    "Ciroldon",
    "Cirrolmar",
    "Clatter",
    "Cloaugh",
    "Cluen",
    "Coilaco",
    "Conaint",
    "Conanath",
    "Conanwe",
    "Conator",
    "Condorath",
    "Condorund",
    "Conforost",
    "Conrolgost",
    "Conthieldel",
    "Coraisle",
    "Corbur",
    "Cordel",
    "Corinth",
    "Cormais",
    "Cornon",
    "Corofin",
    "Corpuit",
    "Corran",
    "Corrienne",
    "Corwen",
    "Cossul",
    "Crail",
    "Cremona",
    "Crerakroth",
    "Crieff",
    "Cromarty",
    "Cumbraes",
    "Cuormver",
    "D'adlor",
    "D'aughough",
    "Daingean",
    "Dankest",
    "Darag",
    "Dariro",
    "Darm",
    "Darunt",
    "Decca",
    "Dedihow",
    "Dedotyl",
    "Denbur",
    "Denew'rt",
    "Denilt",
    "Denlor",
    "Derron",
    "Derwent",
    "Deubrun",
    "Deubur",
    "Deudinon",
    "Deugrande",
    "Deveron",
    "Dezhou",
    "Dhakdor",
    "Dhakghul",
    "Dhakgrod",
    "Dhakwaz",
    "Dilavyk",
    "Donnais",
    "Donndinon",
    "Donnnuon",
    "Doramed",
    "Dorantor",
    "Dormardel",
    "Dormaruen",
    "Dornoch",
    "Dorothtin",
    "Dorsorin",
    "Draerusk",
    "Draib",
    "Drall",
    "Drammes",
    "Drard",
    "Dremmer",
    "Drense",
    "Drimnin",
    "Drogbad",
    "Drogbul",
    "Drummore",
    "Dryck",
    "Drymen",
    "Dunkeld",
    "Dunmanus",
    "Dunster",
    "Durness",
    "Durthblod",
    "Durthrot",
    "Durthrun",
    "Durththang",
    "Duucshire",
    "Dynaw",
    "Dyrutote",
    "E'ervor",
    "E'shyough",
    "Ech'hon'oec",
    "Echech",
    "Echim",
    "Echlir",
    "Echvild",
    "Eighteiai",
    "Eightobana",
    "Eingeeldu",
    "Elathmar",
    "Elfinin",
    "Elgomaar",
    "Ellesmere",
    "Ellon",
    "Elmanu",
    "Elmrake",
    "Elmut",
    "Elodiauen",
    "Emlther",
    "Emys'a",
    "Enburen",
    "Endandon",
    "Endbrodanfel",
    "Endinmar",
    "Endlormar",
    "Endolnost",
    "Endris",
    "Endrolar",
    "Enechan",
    "Enfar",
    "Engogh",
    "Engwina",
    "Enhen",
    "Enlcha",
    "Enrile",
    "Enthon",
    "Enthyl'nt",
    "Erisort",
    "Eriss",
    "Erootu",
    "Erqueo",
    "Errak",
    "Eskerfan",
    "Essend",
    "Essuck",
    "Estkal",
    "Etdshy",
    "Ethaqi",
    "Ethilatnost",
    "Ethildiaanfel",
    "Ethilforanfel",
    "Ethilsortor",
    "Ethilsoruen",
    "Etilda",
    "Ettrick",
    "F'darche",
    "Fanders",
    "Faoughia",
    "Farafra",
    "Ferbane",
    "Fetlar",
    "Flock",
    "Florina",
    "Fontenais",
    "Fontenaisle",
    "Fontenfor",
    "Fontengrois",
    "Fontennuondel",
    "Fontenois",
    "Fontenquel",
    "Fontenrienne",
    "Fordiauen",
    "Fordorlun",
    "Formarund",
    "Formby",
    "Forthieldel",
    "Galloway",
    "Ganzhou",
    "Gar'em",
    "GealCharn",
    "Gecijej",
    "Gededyki",
    "Gerr",
    "Ghaoth",
    "Ghasver",
    "Ghathat",
    "Gifford",
    "Girvan",
    "Githdormar",
    "Githdoror",
    "Githendor",
    "Githforar",
    "Githsoroth",
    "Glenanane",
    "Glin",
    "Glomera",
    "Glormandia",
    "Glorsordel",
    "Gluggby",
    "Gnoelhaala",
    "Golconda",
    "Gorbad",
    "Gorbul",
    "Gordor",
    "Gorthang",
    "Gorungol",
    "Gourock",
    "Gragblod",
    "Gragbul",
    "Graggabab",
    "Gragghul",
    "Gragthang",
    "Grandola",
    "Grendel",
    "Grendinon",
    "Grenmais",
    "Grenois",
    "Grenpuit",
    "Gresir",
    "Greverre",
    "Grimbad",
    "Griminish",
    "Grimrot",
    "Grimrun",
    "Groddland",
    "Grue",
    "Gudup",
    "Gurkacre",
    "Gytokas",
    "Haikou",
    "Halkirk",
    "Handan",
    "Hasmerr",
    "Hateto",
    "Hawodu",
    "Heald",
    "Helmsley",
    "Helsicke",
    "Helvete",
    "Heold",
    "Hikub",
    "Hiren",
    "Hirenuen",
    "Hoisais",
    "Hoisaisle",
    "Hoisburaisle",
    "Hoisdel",
    "Hoisdinon",
    "Hoisgris",
    "Hoislanque",
    "Hoisnuon",
    "Hoisois",
    "Hoisquel",
    "Honeer'o",
    "Honeng",
    "Hostcer",
    "Hullevala",
    "I'ceraw",
    "Iaortai",
    "Iath",
    "Ichidi",
    "Ickellund",
    "Ightrodat",
    "Ildit",
    "Ildtury",
    "Imltai",
    "Imoretore",
    "Imyld",
    "Inaden",
    "Inananwe",
    "Inber",
    "Indortor",
    "Ineisu",
    "Ingingo",
    "Inoltor",
    "Inthielost",
    "Inverie",
    "Ioughosula",
    "Iruiv'w",
    "Iseteng",
    "Isom",
    "Issche",
    "Issnina",
    "Itkali",
    "Ityest",
    "Ityt",
    "Jaca",
    "Jahrom",
    "Jeormel",
    "Jining",
    "Joen",
    "Jotel",
    "Kaddervar",
    "Kal'oma",
    "Kalaur'o",
    "Kalol",
    "Kalshy",
    "Karand",
    "Kargbul",
    "Karggabab",
    "Karggog",
    "Kargslag",
    "Kargthang",
    "Kargthor",
    "Kargungol",
    "Karothea",
    "Kashmar",
    "Kawapo",
    "Kekeloh",
    "Kelade",
    "Keswick",
    "Ketyst",
    "Kielder",
    "Killorglin",
    "Kimrem",
    "Kimsay",
    "Kinbrace",
    "Kintore",
    "Kirriemuir",
    "Klen",
    "Knesekt",
    "Kobbe",
    "Koheda",
    "Komarken",
    "Kovel",
    "Krod",
    "Kuny",
    "Kursk",
    "Kynys",
    "L'oria",
    "Lagos",
    "Lagysar",
    "Lakah",
    "Lamlash",
    "Lapsam",
    "Larache",
    "Larkanth",
    "Larmet",
    "Larok",
    "Larothold",
    "Latuz",
    "Lautaro",
    "Lavan",
    "Lediro",
    "Leighlin",
    "Lemos",
    "Ler'usk'uil",
    "Lerbeli",
    "Lervir",
    "Leven",
    "Licata",
    "Licert",
    "Light",
    "Lijyxebo",
    "Lilais",
    "Lilaisle",
    "Lildinon",
    "Lilgrande",
    "Lilgrois",
    "Lilmais",
    "Lilnon",
    "Lilnuon",
    "Lilois",
    "Lilrienne",
    "Limavady",
    "Lingen",
    "Lintan",
    "Lion",
    "Lipicuxe",
    "Liscannor",
    "Llirad",
    "Llolald",
    "Locarno",
    "Loceria",
    "Lochalsh",
    "Lochcarron",
    "Lochinver",
    "Lochmaben",
    "Lolosutu",
    "Lom",
    "Loner",
    "Loras",
    "Lorathlun",
    "Lorathor",
    "Lorayther",
    "Lorbroddel",
    "Lorbrodrond",
    "Lorgolgost",
    "Lorima",
    "Lormaroth",
    "Lormartor",
    "Lorthalm",
    "Lorthielmar",
    "Lotan",
    "Lothamar",
    "Lothathath",
    "Lothdornost",
    "Lothinin",
    "Lothmarlun",
    "Lothmaruen",
    "Lotholuen",
    "Louer",
    "Lurkabo",
    "Lutedose",
    "Luthiir",
    "Lybster",
    "Lydel",
    "Lye'mos",
    "Lyeak",
    "Lyehir",
    "Lyfel",
    "Lyquel",
    "Macila",
    "Maharoxe",
    "Mallaig",
    "Marsdel",
    "Mataro",
    "Mauvtan",
    "Meet",
    "Melfi",
    "Melvaig",
    "Menter",
    "Methven",
    "Moffat",
    "Moifbel",
    "Monaisle",
    "Monamolin",
    "Monbrun",
    "Monbur",
    "Mondinon",
    "Monfel",
    "Monfor",
    "Mongris",
    "Mongrois",
    "Monlanque",
    "Monmais",
    "Monzon",
    "Mor'ser",
    "Morblod",
    "Morella",
    "Morena",
    "Morgabab",
    "Morgel",
    "Morgog",
    "Morrun",
    "Morslag",
    "Mortenford",
    "Morungol",
    "Morwara",
    "Moryr",
    "Mosiap'i",
    "Mosuf",
    "Mulle",
    "Mupac",
    "Murom",
    "Muskel",
    "Nabrun",
    "Nadam",
    "Nadinon",
    "Nafor",
    "Nagrande",
    "Nagrois",
    "Nairn",
    "Nakyce",
    "Naletran",
    "Namais",
    "Namaisbur",
    "Nanuon",
    "Naper",
    "Naquelrienne",
    "Narbul",
    "Nargabab",
    "Nargbul",
    "Nargrod",
    "Nargshak",
    "Nargslag",
    "Nargungol",
    "Nargwaz",
    "Narienne",
    "Narrot",
    "Narshak",
    "Nazblod",
    "Nazghul",
    "Nazgog",
    "Nazgrod",
    "Nazrot",
    "Nazshak",
    "Nazthang",
    "Negar",
    "Nenol",
    "NephinBeg",
    "Nimathost",
    "Nimathtor",
    "Nimenor",
    "Niskby",
    "Nolle",
    "Nonyvu",
    "Nork",
    "Nosen",
    "Nugyru",
    "Nurac",
    "Nusay",
    "Nutaru",
    "Nyauss'c",
    "Nybusa",
    "Nyduvy",
    "Nylarey",
    "Nyon",
    "Nysasha",
    "Nysaw",
    "Nysgarim",
    "O'agetin",
    "Odirtu",
    "Oldgale",
    "Oldreng",
    "Olenek",
    "Oloron",
    "Onuise",
    "Oranmore",
    "Orelda",
    "Ormwary",
    "Orrach",
    "Oruivu",
    "Os'radi",
    "Osereunti",
    "Osurnkim",
    "Oughlaw",
    "Oughxpol",
    "Palmi",
    "Panyu",
    "Parais",
    "Parfel",
    "Partry",
    "Pary",
    "Pauer",
    "Payawofy",
    "Penhalolen",
    "Perald",
    "Perel",
    "Peres",
    "Perkel",
    "Perrana",
    "Perski",
    "Phaitwar",
    "Phidyntia",
    "Planken",
    "Plattland",
    "Pleagne",
    "Poleld",
    "Polyk",
    "Portimao",
    "Potenza",
    "Poval",
    "Preetsome",
    "Presu",
    "Prettstern",
    "Pycatoca",
    "Quainai",
    "Quiser",
    "Quokelhin",
    "Radiaoth",
    "Ragolrond",
    "Rainanwe",
    "Rainlun",
    "Rakach",
    "Rakiy",
    "Rakkal",
    "Ranaw",
    "Randar",
    "Ranshya",
    "Rantlu",
    "RathLuire",
    "Rayhat",
    "Raymdyn",
    "Rayos",
    "Rayting",
    "Reildel",
    "Reilfel",
    "Reilfor",
    "Reilgris",
    "Reilgrois",
    "Reilmais",
    "Reilnon",
    "Reilois",
    "Reilrienne",
    "Rekelny",
    "Remosyne",
    "Rethel",
    "Retica",
    "Rhakin",
    "Rhihin",
    "Riggenthorpe",
    "Riltend",
    "Ristril",
    "Robune",
    "Rochfort",
    "Rod'ar",
    "Rodair'n",
    "Roddendor",
    "Rodoro",
    "Rodwor",
    "Roin",
    "Roptille",
    "Roril",
    "Roskelem",
    "Roter",
    "Rothrila",
    "Rothtim",
    "Rozocil",
    "Rueve",
    "Runahesy",
    "Rusila",
    "Rycydome",
    "Rynok",
    "Sagunto",
    "Saklebille",
    "Salen",
    "Sandwick",
    "Sarab",
    "Sarkanvale",
    "Sash",
    "Satiasam",
    "Sayqua",
    "Scandamia",
    "Scarinish",
    "Schiaqlor",
    "Schoineenth",
    "Schyacher",
    "Schyine",
    "Schywor",
    "Scourie",
    "Seen",
    "Sefitat",
    "Ser'en'et",
    "Seress'a",
    "Serilelo",
    "Serov",
    "Seruph",
    "Setihe",
    "Setur",
    "Shanyin",
    "Shyselm",
    "Siegen",
    "Sienbrun",
    "Siendel",
    "Siendinon",
    "Sienlanque",
    "Siennuon",
    "Sienrienne",
    "Sihep",
    "Sinan",
    "Sines",
    "Sisasa",
    "Situciru",
    "Situlim",
    "Skelid",
    "Skelnysard",
    "Skim",
    "Skomer",
    "Sl'tinough",
    "Sledmere",
    "Sm'kelmos",
    "Smotur",
    "Smuiss",
    "Snyaldshy",
    "Sobabaho",
    "Sondhon",
    "Soratath",
    "Sorator",
    "Soratuen",
    "Sorbrodanfel",
    "Sorisdale",
    "Spakker",
    "Stackforth",
    "Staklesse",
    "Stilye",
    "Stinchar",
    "Stoem",
    "Stoer",
    "Streang",
    "Strichen",
    "Stroest",
    "Stroma",
    "Suide",
    "Sulsum",
    "Sultenth",
    "Sulwor",
    "Surdel",
    "Surfel",
    "Surlanque",
    "Surpuit",
    "Swedelale",
    "Swuor",
    "Syimler",
    "Syque",
    "Sytiti",
    "Tabuk",
    "Tageick",
    "Taial",
    "Taidald",
    "Taiq",
    "Tanros",
    "Tantona",
    "Taold",
    "Tarraspan",
    "Tearque",
    "Teeleld",
    "Terixybi",
    "Tetuan",
    "Teyaludu",
    "Thanathmar",
    "Thanloruen",
    "Ther'ban",
    "Thorbad",
    "Thorbul",
    "Thordor",
    "Thorgrod",
    "Thorrot",
    "Thorungol",
    "Thurso",
    "Tia'en'ab",
    "Tiadar",
    "Tiaougha",
    "Tiemcen",
    "Tiksi",
    "Tiny",
    "Tolsta",
    "Toluis",
    "Tonays'u",
    "Tonentin",
    "Tonther",
    "Toppola",
    "Toreg",
    "Torell",
    "Torridon",
    "Torril",
    "Tortur",
    "Toumser",
    "Tourgrande",
    "Tourlanque",
    "Tourois",
    "Tourquel",
    "Tourrienne",
    "Trapani",
    "Tromeforth",
    "Tudela",
    "Tur'enda",
    "Turaym'k",
    "Tureer'l",
    "Turia",
    "Tynumyv",
    "U'ildaw",
    "U'osad",
    "Umoc",
    "Umoth",
    "Und'rak",
    "Untnqua",
    "Untvory",
    "Urnraynys",
    "Uskus'k",
    "Ustumi",
    "Uthbad",
    "Uthblod",
    "Uthbul",
    "Uthghul",
    "Uthrun",
    "Uththor",
    "Uzbad",
    "Uzdor",
    "Uzghul",
    "Uzgog",
    "Uzgoth",
    "Uzrot",
    "Uzrun",
    "Uzshak",
    "Uzslag",
    "Vaila",
    "Valga",
    "Vather",
    "Ver'buro",
    "Verguin",
    "Vernlund",
    "Veroit",
    "Verrod",
    "Versaisle",
    "Versfor",
    "Versgris",
    "Verslanque",
    "Verslanquebur",
    "Versmais",
    "Versnon",
    "Versrienne",
    "Verum",
    "Ves'kel",
    "Vesul",
    "Victoria",
    "Vigurr",
    "Vojowex",
    "Vorild",
    "Vosetosi",
    "Waimer",
    "Wariav'gh",
    "Warick",
    "Warilt",
    "Wett",
    "Whald",
    "Whaor",
    "Wheescha",
    "Wheinwor",
    "Wicaess",
    "Woiknal",
    "Worelm",
    "Worhon",
    "Wygece",
    "Wysakol",
    "Xontormia",
    "Y'tinny",
    "Yahevoru",
    "Yakleks",
    "Ydaso",
    "Yejel",
    "Yeritoni",
    "Yleya",
    "Ypipix",
    "Ypoluemo",
    "Yuci",
    "Zaalsehuur",
    "Zafahyro",
    "Zamora",
    "Zapulla",
    "Zhedar",
    "Zicoge",
    "Zoddor",
    "Zodgabab",
    "Zodghul",
    "Zodgrod",
    "Zodrun",
    "Zodslag",
    "Zodthang",
    "Zodungol",
    "Zogghul",
    "Zoggog",
    "Zoggoth",
    "Zoggrod",
    "Zogthang",
    "Zyrosihu",
};

//
// The following stuff is just for in this file, to setup the names during
// world setup
//

static int nnames;
static int * nameused;
static int ntowns;
static int nregions;

void SetupNames()
{
    nnames = sizeof regionnames / sizeof (char *);
    nameused = new int[nnames];

    for (int i=0; i<nnames; i++) nameused[i] = 0;
	ntowns = 0;
	nregions = 0;
}

void CountNames()
{
	Awrite(AString("Regions ") + nregions);
}

int AGetName(int town )
{
	int offset, number;
	if(town) {
		offset = 0;
		number = NUMBER_OF_TOWNS;
	} else {
		offset = NUMBER_OF_TOWNS;
		number = nnames-NUMBER_OF_TOWNS;
	}

	if(town) ntowns++;
	else nregions++;

    int i=getrandom(number);
	int j;
	for(int count=0; count < number; count++) {
		j = i+offset;
		if(nameused[j] == 0) {
			nameused[j] = 1;
			return j;
		}
		if(++i >= number) i=0;
	}
    for (i=0; i<number; i++) nameused[i+offset] = 0;
    i = getrandom(number);
	j = i+offset;
    nameused[j] = 1;
    return j;
}

char const *AGetNameString( int name )
{
    return( regionnames[ name ] );
}

void Game::CreateWorld()
{
	int nx = 0;
	int ny = 1;
	if(Globals->MULTI_HEX_NEXUS) {
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
        if( xx % 8 ) {
            xx = 0;
            Awrite( "The width must be a multiple of 8." );
        }
    }
    int yy = 0;
    while (yy <= 0) {
        Awrite("How tall should the map be? ");
        yy = Agetint();
        if( yy % 8 ) {
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
	for(i = 2; i < Globals->UNDERWORLD_LEVELS+2; i++) {
		int xs = regions.GetLevelXScale(i);
		int ys = regions.GetLevelYScale(i);
		regions.CreateUnderworldLevel(i, xx/xs, yy/ys, "underworld");
	}
	// Underdeep levels
	for(i=Globals->UNDERWORLD_LEVELS+2;
			i<(Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+2); i++) {
		int xs = regions.GetLevelXScale(i);
		int ys = regions.GetLevelYScale(i);
		regions.CreateUnderdeepLevel(i, xx/xs, yy/ys, "underdeep");
	}

	if(Globals->ABYSS_LEVEL) {
		regions.CreateAbyssLevel(Globals->UNDERWORLD_LEVELS +
				Globals->UNDERDEEP_LEVELS + 2, "abyss");
	}

	CountNames();

	if(Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS == 1) {
		regions.MakeShaftLinks( 2, 1, 8 );
	} else if(Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS) {
		int i, ii;
		// shafts from surface to underworld
		regions.MakeShaftLinks(2, 1, 10);
		for(i=3; i<Globals->UNDERWORLD_LEVELS+2; i++) {
			regions.MakeShaftLinks(i, 1, 10*i-10);
		}
		// Shafts from underworld to underworld
		if(Globals->UNDERWORLD_LEVELS > 1) {
			for(i = 3; i < Globals->UNDERWORLD_LEVELS+2; i++) {
				for(ii = 2; ii < i; ii++) {
					if(i == ii+1) {
						regions.MakeShaftLinks(i, ii, 12);
					} else {
						regions.MakeShaftLinks(i, ii, 24);
					}
				}
			}
		}
		// underdeeps to underworld
		if(Globals->UNDERDEEP_LEVELS && Globals->UNDERWORLD_LEVELS) {
			// Connect the topmost of the underdeep to the bottommost
			// underworld
			regions.MakeShaftLinks(Globals->UNDERWORLD_LEVELS+2,
					Globals->UNDERWORLD_LEVELS+1, 12);
		}
		// Now, connect the underdeep levels together
		if(Globals->UNDERDEEP_LEVELS > 1) {
			for(i = Globals->UNDERWORLD_LEVELS+3;
					i < Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+2;
					i++) {
				for(ii = Globals->UNDERWORLD_LEVELS+2; ii < i; ii++) {
					if(i == ii+1) {
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
	for(int i=2; i < Globals->UNDERWORLD_LEVELS+2; i++) {
		regions.InitSetupGates( i );
	}
	// Underdeep has no gates, only the possible shafts above.
    regions.FinalSetupGates();

    regions.CalcDensities();
}

int ARegionList::GetRegType( ARegion *pReg )
{
    //
    // Figure out the distance from the equator, from 0 to 3.
    //
    int lat = ( pReg->yloc * 8 ) / ( pRegionArrays[ pReg->zloc ]->y );
    if (lat > 3)
    {
        lat = (7 - lat);
    }

	// Underworld region
    if((pReg->zloc > 1) && (pReg->zloc < Globals->UNDERWORLD_LEVELS+2)) {
        int r = getrandom(4);
        switch (r) {
			case 0:
				return R_OCEAN;
			case 1:
				return R_CAVERN;
			case 2:
				return R_UFOREST;
			case 3:
				return R_TUNNELS;
			default:
				return( 0 );
        }
    }

	// Underdeep region
	if((pReg->zloc > Globals->UNDERWORLD_LEVELS+1) &&
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
    if( pReg->zloc == 1 ) {
        int r = getrandom(64);
        switch (lat)
        {
        case 0: /* Arctic regions */
            if (r < 24) return R_TUNDRA;
            if (r < 32) return R_MOUNTAIN;
            if (r < 40) return R_FOREST;
            return R_PLAIN;
        case 1: /* Colder regions */
            if (r < 16) return R_PLAIN;
            if (r < 40) return R_FOREST;
            if (r < 48) return R_MOUNTAIN;
            return R_SWAMP;
        case 2: /* Warmer regions */
            if (r < 20) return R_PLAIN;
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

    if( pReg->zloc == 0 )
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
	if(level < 2) return 1;

	// If we only have one underworld level it's 1/2 size
	if(Globals->UNDERWORLD_LEVELS == 1 && Globals->UNDERDEEP_LEVELS == 0)
		return 2;

	// We have multiple underworld levels
	if(level >= 2 && level < Globals->UNDERWORLD_LEVELS+2) {
		// Topmost underworld level is full size in x direction
		if(level == 2) return 1;
		// All other levels are 1/2 size in the x direction
		return 2;
	}

	if(level >= Globals->UNDERWORLD_LEVELS+2 &&
			level < (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+2)){
		// Topmost underdeep level is 1/2 size in the x direction
		if(level == Globals->UNDERWORLD_LEVELS+2) return 2;
		// All others are 1/4 size in the x direction
		return 4;
	}
	// We couldn't figure it out, assume not scaled.
	return 1;
}

int ARegionList::GetLevelYScale(int level)
{
	// Surface and nexus are unscaled
	if(level < 2) return 1;

	// If we only have one underworld level it's 1/2 size
	if(Globals->UNDERWORLD_LEVELS == 1 && Globals->UNDERDEEP_LEVELS == 0)
		return 2;

	// We have multiple underworld levels
	if(level >= 2 && level < Globals->UNDERWORLD_LEVELS+2) {
		// Topmost level is 1/2 size in the y direction
		if(level == 2) return 2;
		// Bottommost is 1/4 size in the y direction
		if(level == Globals->UNDERWORLD_LEVELS+1) return 4;
		// All others are 1/2 size in the y direction
		return 2;
	}
	if(level >= Globals->UNDERWORLD_LEVELS+2 &&
			level < (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+2)){
		// All underdeep levels are 1/4 size in the y direction.
		return 4;
	}
	// We couldn't figure it out, assume not scaled.
	return 1;
}

int ARegionList::CheckRegionExit(ARegion *pFrom, ARegion *pTo )
{
    if((pFrom->zloc==1) ||
		(pFrom->zloc>Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+1)) {
        return( 1 );
    }

    int chance = 0;
    if( pFrom->type == R_CAVERN || pFrom->type == R_UFOREST ||
        pTo->type == R_CAVERN || pTo->type == R_UFOREST )
    {
        chance = 25;
    }
    if( pFrom->type == R_TUNNELS || pTo->type == R_TUNNELS)
    {
        chance = 50;
    }
	if(pFrom->type == R_GROTTO || pFrom->type == R_DFOREST ||
	   pTo->type == R_GROTTO || pTo->type == R_DFOREST) {
		// better connected underdeeps
		chance = 40;
	}
	if(pFrom->type == R_CHASM || pTo->type == R_CHASM) {
		chance = 60;
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

    if( pReg->zloc > 1 )
    {
        return( W_NORMAL );
    }

	if (!Globals->OPEN_ENDED && pReg->zloc == 3)
	{
		return (W_NORMAL);
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

    while(inlist.Num())
    {
        ARegionPtr * reg = (ARegionPtr *) inlist.First();
        for (int i=0; i<NDIRS; i++)
        {
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
	if(!Globals->TOWNS_EXIST) return;

	if(Globals->GATES_EXIST) gate = -1;
	
	if(town) delete town;
    
    AddTown(TOWN_CITY);

	if(!Globals->START_CITIES_EXIST) return;

	town->hab = 125 * Globals->CITY_POP / 100;
    while (town->pop < town->hab) town->pop += getrandom(200)+200;
    town->dev = TownDevelopment();

	float ratio;
	Market *m;
    markets.DeleteAll();
	if(Globals->START_CITIES_START_UNLIMITED) {
		for (int i=0; i<NITEMS; i++) {
			if( ItemDefs[i].flags & ItemType::DISABLED ) continue;
			if( ItemDefs[ i ].type & IT_NORMAL ) {
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
		if(Globals->LEADERS_EXIST) {
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
		if( Globals->LEADERS_EXIST ) {
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
	if(type == R_NEXUS) return 1;
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

    if( pArr->x < maxX ) maxX = pArr->x;
    if( pArr->y < maxY ) maxY = pArr->y;

	int tries = 0;
    while (!reg && tries < 10000)
    {
        //
        // We'll just let AC exits be all over the map.
        //
        int x = getrandom( maxX );
        int y = 2 * getrandom( maxY / 2 ) + x % 2;

        reg = pArr->GetRegion( x, y );

        if(!reg || !reg->CanBeStartingCity( pArr )) {
            reg = 0;
			tries++;
            continue;
        }

        for (int j=0; j<i; j++) {
			if(!AC->neighbors[j]) continue;
            if (GetDistance(reg,AC->neighbors[j]) < maxY / 10 + 2 ) {
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
		if(!reg || reg->type == R_OCEAN) {
			tries++;
			reg = 0;
			continue;
		}

		for (int j=0; j<i; j++) {
			if(!AC->neighbors[j]) continue;
			if (GetDistance(reg,AC->neighbors[j]) < maxY / 10 + 2 ) {
				reg = 0;
				tries++;
				break;
			}
		}
	}

	// Okay, if we still don't have anything, we're done.
    return reg;
}

