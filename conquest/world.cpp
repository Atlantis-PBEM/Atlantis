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
#include "rules.h"

static char *regionnames[] =
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

// Make sure this is correct.   The default is 1000 towns and 1000 regions.
#define NUMBER_OF_TOWNS 1000

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
	Awrite(AString("Towns ") + ntowns);
	Awrite(AString("Regions ") + nregions);
}

int AGetName( ARegion *pReg, int town )
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
    i = getrandom(nnames);
	j = i+offset;
    nameused[j] = 1;
    return j;
}

char *AGetNameString( int name )
{
    return( regionnames[ name ] );
}

char Game::GetRChar(ARegion * r)
{
    int t = r->type;
    char c;
    switch (t) {
    case R_OCEAN:
        return '-';

    case R_PLAIN:
        c = 'p';
        break;
    case R_FOREST:
        c = 'f';
        break;
    case R_MOUNTAIN:
        c = 'm';
        break;
    case R_SWAMP:
        c = 's';
        break;
    case R_JUNGLE:
        c = 'j';
        break;
    case R_DESERT:
        c = 'd';
        break;
    case R_TUNDRA:
        c = 't';
        break;
    case R_CAVERN:
        c = 'c';
        break;
    case R_UFOREST:
        c = 'f';
        break;
    case R_TUNNELS:
        c = 't';
        break;
	case R_ISLAND_PLAIN:
		c = 'a';
	case R_ISLAND_MOUNTAIN:
		c = 'n';
	case R_ISLAND_SWAMP:
		c = 'w';
    default:
        return '?';

    }
    
    if (r->town) {
        c = (c - 'a') + 'A';
    }
    return c;
}

void Game::CreateWorld()
{
    int nPlayers = 0;
	while(nPlayers <= 0) {
		Awrite("How many players iwll be in this game? ");
		nPlayers = Agetint();
	}

	regions.CreateLevels(1);
    SetupNames();

    regions.CreateIslandLevel( 0, nPlayers, 0 );
	CountNames();
	regions.CalcDensities();
}

void Game::CreateNPCFactions()
{
	Faction *f;
	AString *temp;
	if(Globals->CITY_MONSTERS_EXIST) {
		f = new Faction(factionseq++);
		guardfaction = f->num;
		temp = new AString("The Guardsmen");
		f->SetName(temp);
		f->SetNPC();
		factions.Add(f);
	}
	if(Globals->WANDERING_MONSTERS_EXIST || Globals->LAIR_MONSTERS_EXIST) {
		f = new Faction(factionseq++);
		monfaction = f->num;
		temp = new AString("Creatures");
		f->SetName(temp);
		f->SetNPC();
		factions.Add(f);
	}
}

void Game::CreateCityMon( ARegion *pReg, int percent )
{
    int skilllevel;
    int AC = 0;
	int IV = 0;
	int num;
    if( pReg->type == R_NEXUS || pReg->IsStartingCity() )
    {
        skilllevel = TOWN_CITY + 1;
		if(Globals->SAFE_START_CITIES || (pReg->type == R_NEXUS))
			IV = 1;
        AC = 1;
		num = Globals->AMT_START_CITY_GUARDS;
    }
    else
    {
        skilllevel = pReg->town->TownType() + 1;
		num = Globals->CITY_GUARD * skilllevel;
    }
    num = num * percent / 100;

    Faction *pFac = GetFaction( &factions, 1 );

    Unit *u = GetNewUnit( pFac );
    AString *s = new AString("City Guard");
    u->SetName( s );
    u->type = U_GUARD;
    u->guard = GUARD_GUARD;
  
    u->SetMen(I_LEADERS,num);
    u->items.SetNum(I_SWORD,num);
    if (IV) u->items.SetNum(I_AMULETOFI,num);
    u->SetMoney(num * Globals->GUARD_MONEY);	
    u->SetSkill(S_COMBAT,skilllevel);
    if (AC) {
		if(Globals->START_CITY_GUARDS_PLATE)
			u->items.SetNum(I_PLATEARMOR, num);
        u->SetSkill(S_OBSERVATION,10);
    } else {
        u->SetSkill(S_OBSERVATION,skilllevel);
    }
    u->SetFlag(FLAG_HOLDING,1);
    
    u->MoveUnit( pReg->GetDummy() );

	if(AC && Globals->START_CITY_MAGES) {
    	u = GetNewUnit( pFac );
		s = new AString("City Mage");
		u->SetName(s);
		u->type = U_GUARDMAGE;
		u->SetMen(I_LEADERS,1);
		if(IV) u->items.SetNum(I_AMULETOFI,1);
		u->SetMoney(Globals->GUARD_MONEY);
		u->SetSkill(S_FORCE,4);
		u->SetSkill(S_FIRE,4);
		u->combat = S_FIRE;
		u->SetFlag(FLAG_BEHIND, 1);
		u->SetFlag(FLAG_HOLDING, 1);
		u->MoveUnit(pReg->GetDummy());
	}
}

void Game::AdjustCityMons( ARegion *r )
{
    int guard = 0;
    forlist(&r->objects) {
        Object * o = (Object *) elem;
        forlist(&o->units) {
            Unit * u = (Unit *) elem;
            if (u->type == U_GUARD || u->type == U_GUARDMAGE)
            {
                AdjustCityMon( r, u );
                return;
            }
            if (u->guard == GUARD_GUARD) {
                guard = 1;
            }
        }
    }

    if (!guard && getrandom(100) < Globals->GUARD_REGEN)
    {
        CreateCityMon( r, 10 );
    }
}

void Game::AdjustCityMon( ARegion *r, Unit *u )
{
    int towntype;
    int AC = 0;
	int men;
	int IV = 0;
    if( r->type == R_NEXUS || r->IsStartingCity() )
    {
        towntype = TOWN_CITY;
        AC = 1;
		if(Globals->SAFE_START_CITIES || (r->type == R_NEXUS))
			IV = 1;
		if(u->type == U_GUARDMAGE) {
			men = 1;
		} else {
			men = u->GetMen() + (Globals->AMT_START_CITY_GUARDS/10);
			if(men > Globals->AMT_START_CITY_GUARDS)
				men = Globals->AMT_START_CITY_GUARDS;
		}
    } else {
        towntype = r->town->TownType();
		men = u->GetMen() + (Globals->CITY_GUARD/10)*(towntype+1);
		if(men > Globals->CITY_GUARD * (towntype+1))
			men = Globals->CITY_GUARD * (towntype+1);
    }

    u->SetMen(I_LEADERS,men);
	if (IV) u->items.SetNum(I_AMULETOFI,men);

	if(u->type == U_GUARDMAGE) {
		u->SetSkill(S_FORCE, 4);
		u->SetSkill(S_FIRE, 4);
		u->combat = S_FIRE;
		u->SetFlag(FLAG_BEHIND, 1);
        u->SetMoney(Globals->GUARD_MONEY);
	} else {
        u->SetMoney(men * Globals->GUARD_MONEY);
		u->SetSkill(S_COMBAT,towntype + 1);
		if (AC) {
			u->SetSkill(S_OBSERVATION,10);
			if(Globals->START_CITY_GUARDS_PLATE)
				u->items.SetNum(I_PLATEARMOR,men);
		} else {
			u->SetSkill(S_OBSERVATION,towntype + 1);
		}
		u->items.SetNum(I_SWORD,men);
	}
}

int Game::MakeWMon( ARegion *pReg )
{

    if (TerrainDefs[pReg->type].wmonfreq == 0)
    {
        return 0;
    }

    int montype = TerrainDefs[ pReg->type ].smallmon;
    if (getrandom(2))
        montype = TerrainDefs[ pReg->type ].humanoid;
    if (TerrainDefs[ pReg->type ].bigmon != -1 && !getrandom(8)) {
        montype = TerrainDefs[ pReg->type ].bigmon;
    }
    
    int mondef = ItemDefs[montype].index;
    
    Faction *monfac = GetFaction( &factions, 2 );

    Unit *u = GetNewUnit( monfac, 0 );
    u->MakeWMon( MonDefs[mondef].name, montype,
                 (MonDefs[mondef].number +
                  getrandom(MonDefs[mondef].number) + 1) / 2);
    u->MoveUnit( pReg->GetDummy() );
    return( 1 );
}

void Game::MakeLMon( Object *pObj )
{
    int montype = ObjectDefs[ pObj->type ].monster;
    if (montype == I_TRENT)
    {
        montype = TerrainDefs[ pObj->region->type].bigmon;
    }
    if (montype == I_CENTAUR)
    {
        montype = TerrainDefs[ pObj->region->type ].humanoid;
    }

    int mondef = ItemDefs[montype].index;
    Faction *monfac = GetFaction( &factions, 2 );
    Unit *u = GetNewUnit( monfac, 0 );
    if (montype == I_IMP)
    {
        u->MakeWMon( "Demons",
                     I_IMP,
                     getrandom( MonDefs[MONSTER_IMP].number + 1 ));
        u->items.SetNum( I_DEMON,
                         getrandom( MonDefs[MONSTER_DEMON].number + 1 ));
        u->items.SetNum( I_BALROG,
                         getrandom( MonDefs[MONSTER_BALROG].number + 1 ));
    }
    else if (montype == I_SKELETON)
    {
        u->MakeWMon( "Undead",
                     I_SKELETON,
                     getrandom( MonDefs[MONSTER_SKELETON].number + 1 ));
        u->items.SetNum( I_UNDEAD,
                         getrandom( MonDefs[MONSTER_UNDEAD].number + 1 ));
        u->items.SetNum( I_LICH,
                         getrandom( MonDefs[MONSTER_LICH].number + 1 ));
    }
    else
    {
        u->MakeWMon( MonDefs[mondef].name,
                     montype,
                     ( MonDefs[mondef].number +
                       getrandom( MonDefs[mondef].number ) + 1) / 2);
    }

    u->MoveUnit( pObj );
}

int ARegionList::GetRegType( ARegion *pReg )
{
	ARegionArray *pRA = pRegionArrays[ pReg->zloc ];
	int xBase = 0;
	int yBase = 0;
	int isIsland = 0;
	if( pReg->xloc < 8 ) {
		isIsland = 1;
		xBase = pReg->xloc - 2;
		yBase = ( pReg->yloc - 10 ) % 6;
	}
	if( pReg->yloc < 8 ) {
		isIsland = 1;
		xBase = ( pReg->xloc - 10 ) % 6;
		yBase = pReg->yloc - 2;
	}
	if( pReg->xloc >= pRA->x - 8 ) {
		isIsland = 1;
		xBase = pReg->xloc + 6 - pRA->x;
		yBase = ( pReg->yloc - 10 ) % 6;
	}
	if( pReg->yloc >= pRA->y - 8 ) {
		isIsland = 1;
		xBase = ( pReg->xloc - 10 ) % 6;
		yBase = pReg->yloc + 6 - pRA->y;
	}

	if( isIsland ) {
		if( xBase == 0 || xBase == 3 ) {
			return( R_ISLAND_SWAMP );
        }
		if( yBase == 0 || yBase == 3 ) {
			return( R_ISLAND_SWAMP );
		}
		if( xBase == 1 && yBase == 1 ) {
			return( R_ISLAND_MOUNTAIN );
	   	}
		if( xBase == 2 && yBase == 2 ) {
			return( R_ISLAND_PLAIN );
		}
		// This shouldn't get called
		//
		return( R_OCEAN );
	}

	int r = getrandom(5);
	switch (r) {
		case 0:
		case 1:
			return R_PLAIN;
		case 2:
			return R_FOREST;
		case 3:
			return R_MOUNTAIN;
		case 4:
			return R_SWAMP;
	}

	// This really shouldn't get called
	//
	return( R_OCEAN );
}

int ARegionList::CheckRegionExit( int nDir, ARegion *pFrom, ARegion *pTo )
{
	return( 1 );
}

int ARegionList::GetWeather( ARegion *pReg, int month )
{
	return W_NORMAL;
}

int ARegion::CanBeStartingCity( ARegionArray *pRA )
{
	//
	// If the region is in the middle of the map, it isn't on an island,
	// and cannot be a starting region.
	//
	if( xloc >= 8 && yloc >= 8 && xloc < pRA->x - 8 && yloc < pRA->y - 8 ) {
		return( 0 );
	}
	
	//
	// If the region is not a plain, it cannot be a starting region.
	//
	if( type != R_ISLAND_PLAIN ) {
		return( 0 );
	}

	return (1);
}

void ARegion::MakeStartingCity() 
{
	return;
}

int ARegion::IsStartingCity() {
	forlist(&objects) {
		Object *o = (Object *) elem;
		if( o->type == O_CITADEL ) {
			return( 1 );
		}
	}

	return( 0 );
}

int ARegion::IsSafeRegion()
{
    return( 0 );
}

ARegion *ARegionList::GetStartingCity( ARegion *AC,
                                       int i,
                                       int level,
                                       int maxX,
                                       int maxY )
{
    return 0;
}

