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

static char *regionnames[] =
{
// Towns x1000
	"Cahaba",
	"Hordville",
	"Jamestown West",
	"Sodaville",
	"East Syracuse",
	"Nageezi",
	"Quinebaug",
	"Amherst",
	"Juncal",
	"Hennepin",
	"Bartelso",
	"Santa Anna",
	"Cypress",
	"Villa",
	"Northvale",
	"Moab",
	"Moyie Springs",
	"Abilene",
	"Whelen Springs",
	"Princeville",
	"Milbank",
	"Midwest",
	"Wonewoc",
	"Blue Rapids",
	"Cresco",
	"McCallsburg",
	"Walterhill",
	"Lowesville",
	"New Concord",
	"Stover",
	"Forest River",
	"Westchester",
	"Flowing Wells",
	"Central Islip",
	"Reddick",
	"Oketo",
	"Whitefield",
	"Pajaros",
	"Moose Creek",
	"Boise",
	"Carey",
	"Pymatuning South",
	"Toronto",
	"North Manchester",
	"Senecaville",
	"Ashkum",
	"Bronaugh",
	"Locust",
	"Lake Darby",
	"Pesotum",
	"Fruitville",
	"Manteno",
	"Ellis Grove",
	"Westwego",
	"Ridgeway",
	"Keachi",
	"Basehor",
	"Locust Valley",
	"Lacona",
	"Millers Creek",
	"Lluveras",
	"Dumont",
	"Lake Magdalene",
	"Livonia",
	"Thomasboro",
	"Ferron",
	"East Cleveland",
	"Weatherford",
	"Coupeville",
	"Trussville",
	"Neillsville",
	"Maynard",
	"Morehouse",
	"Thorne",
	"Canaan",
	"Elkport",
	"Banner",
	"Silsbee",
	"Ironton",
	"Beech Grove",
	"Clearview",
	"New Egypt",
	"Lily Lake",
	"Lake Inthe",
	"Evendale",
	"Mer Rouge",
	"Parkland",
	"Avenal",
	"St. Xavier",
	"Barrow",
	"Shelby",
	"Thayne",
	"Loachapoka",
	"Wesley",
	"Watkins Glen",
	"Pinson",
	"Albin",
	"Port Tobacco",
	"Anamosa",
	"Johannesburg",
	"Oquirrh",
	"Tom Bean",
	"Chicago Ridge",
	"Red Creek",
	"Point Baker",
	"La Vernia",
	"Chignik Lake",
	"Baroda",
	"Simpsonville",
	"La Farge",
	"Redway",
	"El Reno",
	"Paxtonia",
	"Puckett",
	"Mead",
	"Rivergrove",
	"Trimont",
	"Warrensville",
	"Babson",
	"Daisetta",
	"Catalina",
	"Sierra Vista",
	"Balch Springs",
	"Larrabee",
	"Watauga",
	"Flomaton",
	"Holloman",
	"Swansboro",
	"Coolidge",
	"Galliano",
	"Jacksonburg",
	"Weedpatch",
	"Oreana",
	"Stockbridge",
	"Page",
	"Rothsay",
	"Scipio",
	"Saguache",
	"Ballantine",
	"Dillwyn",
	"Perryopolis",
	"Denham",
	"Gruver",
	"Kalaoa",
	"Leander",
	"Addis",
	"Arbuckle",
	"Bluford",
	"Mack North",
	"Lorton",
	"Stanardsville",
	"Conneaut Lakeshore",
	"Lincoln",
	"Amesti",
	"Spalding",
	"Ulysses",
	"West Crossett",
	"Chistochina",
	"Subiaco",
	"Fond du Lac",
	"Owingsville",
	"German Valley",
	"Applewold",
	"Oolitic",
	"Sunflower",
	"Weston Mills",
	"Grand Tower",
	"Holtville",
	"Cowen",
	"Power",
	"New Johnsonville",
	"Boiling Spring",
	"Moffat",
	"March",
	"Hometown",
	"Dungannon",
	"Shady Cove",
	"Rattan",
	"Sharpes",
	"Everest",
	"Redgranite",
	"Magalia",
	"Milford",
	"Danube",
	"Neche",
	"Amite",
	"Illiopolis",
	"Sitka and",
	"Butteville",
	"Hendley",
	"Ben Lomond",
	"Delanson",
	"East Bernard",
	"Pass Christian",
	"Bystrom",
	"Tesuque",
	"Ballinger",
	"New Carrollton",
	"Ferrysburg",
	"Dasher",
	"St. Lucie",
	"Oriskany",
	"Manati",
	"Keswick",
	"Hopwood",
	"Dulce",
	"Rock",
	"Manvel",
	"Shoreham",
	"Mount Hebron",
	"Ontario",
	"Barahona",
	"Upton",
	"Harmony",
	"Vidor",
	"Niskayuna",
	"Ocean Beach",
	"North Acomita",
	"Heartwell",
	"Tidioute",
	"Ekalaka",
	"Penermon",
	"Rest Haven",
	"New Hope",
	"Ste. Marie",
	"Wixon Valley",
	"Ogdensburg",
	"Grandville",
	"Ucon",
	"Manitou",
	"Rose Valley",
	"Lemont",
	"Mossyrock",
	"Wailua",
	"Norridge",
	"Ferry",
	"Chanhassen",
	"Van Meter",
	"Olivia",
	"Houtzdale",
	"Lufkin",
	"Truro",
	"Clancy",
	"Westmoreland",
	"Miller",
	"Mohall",
	"Savannah",
	"Buena",
	"Willow River",
	"Helper",
	"Zillah",
	"Bergman",
	"Llano Grande",
	"Frostburg",
	"Lac du Flambeau",
	"Butler",
	"Brandywine",
	"New Providence",
	"McLeansville",
	"Big Rapids",
	"Wolf Trap",
	"Pine Crest",
	"Harristown",
	"Hamberg",
	"McDonough",
	"Kingsley",
	"Hiller",
	"Timpson",
	"Arab",
	"Cayucos",
	"Plattsburgh",
	"Smoke Rise",
	"East Arcadia",
	"Minneiska",
	"Pleasant Lake",
	"Hephzibah",
	"Asheboro",
	"Kenefic",
	"Velma",
	"Vancleave",
	"Mendenhall",
	"Coeymans",
	"Pueblo Pintado",
	"Goodlettsville",
	"Buhler",
	"Naco",
	"Yarmouth",
	"Trinity",
	"Rensselaer Falls",
	"Tillson",
	"Pennock",
	"Pleasant",
	"Tarnov",
	"Oconee",
	"Labish",
	"Worland",
	"Loiza",
	"Sulligent",
	"Tullahoma",
	"Spooner",
	"Parmele",
	"Altheimer",
	"Seltzer",
	"Fairway",
	"Haubstadt",
	"Ocean Breeze",
	"Millerton",
	"Lake Grove",
	"Solon",
	"Cushman",
	"Shickley",
	"River Oaks",
	"Madawaska",
	"Talkeetna",
	"Dundarrach",
	"Springville",
	"Elrod",
	"Turon",
	"McIntosh",
	"Hettinger",
	"Cajah's Mountain",
	"Waitsburg",
	"Plum",
	"Valencia",
	"Rosita South",
	"Gagetown",
	"Loxley",
	"Abbott",
	"Palatine",
	"Ponemah",
	"Centereach",
	"Valley Hill",
	"Mantador",
	"Meadow Vale",
	"Carrsville",
	"Viburnum",
	"Culebra",
	"Huntingtown",
	"Pixley",
	"Tuba",
	"Silverton",
	"Patchogue",
	"Higganum",
	"Fairview",
	"Doran",
	"Lake Wisconsin",
	"Warrensburg",
	"Dunwoody",
	"Loma Linda",
	"Ebro",
	"Randalia",
	"Roundup",
	"Nicholasville",
	"Castle Pines",
	"Gifford",
	"New Buffalo",
	"Putney",
	"Seatonville",
	"Delphos",
	"Castalia",
	"Chokio",
	"North Brentwood",
	"Schnecksville",
	"Campanilla",
	"Maria Antonia",
	"Cowden",
	"Warrior",
	"Mosquito Lake",
	"Calion",
	"Newmanstown",
	"Saronville",
	"Cordaville",
	"Harpers Ferry",
	"Vredenburgh",
	"McGill",
	"Luck",
	"Wheaton",
	"Clarksdale",
	"Placid",
	"Burney",
	"Franksville",
	"Kirkland",
	"Akaska",
	"Cylinder",
	"Kenney",
	"Ropesville",
	"Bunn",
	"Argos",
	"Rawson",
	"Brushy Creek",
	"New Baltimore",
	"West End",
	"Glenarden",
	"Brownton",
	"Reese",
	"Holly Springs",
	"Wagoner",
	"Clam Gulch",
	"Vera",
	"Breedsville",
	"Willow Hill",
	"Ellijay",
	"Hart",
	"Alpha",
	"Centralhatchee",
	"Lamoni",
	"Newbern",
	"West Hazleton",
	"Betances",
	"Hatfield",
	"Jupiter",
	"Brooksville",
	"Becker",
	"Buckhead",
	"Muscoda",
	"Hopkinsville",
	"Sebewaing",
	"Shattuck",
	"Sallisaw",
	"Berkley",
	"West Gate",
	"The Crossings",
	"East Dublin",
	"Forestville",
	"Caldwell",
	"Oroville East",
	"Dillsboro",
	"Gatlinburg",
	"Lighthouse Point",
	"Walhalla",
	"Hartman",
	"Leonville",
	"Manderson",
	"Vann",
	"Bellefonte",
	"Brier",
	"West Chester",
	"Clendenin",
	"Moraine",
	"Natalia",
	"Grant",
	"Leonardville",
	"West Mansfield",
	"Wynantskill",
	"Leeds",
	"Corral",
	"Steelton",
	"Blandinsville",
	"Amazonia",
	"Flovilla",
	"Eatonville",
	"Almyra",
	"Eldorado",
	"Dering",
	"Kirkpatrick",
	"West Hartford",
	"Halliday",
	"West Side",
	"Arcola",
	"Juno Beach",
	"Game Creek",
	"South Orange",
	"Banks",
	"Cotesfield",
	"Saltville",
	"Minnewaukan",
	"Oconto",
	"South Paris",
	"Boulder Hill",
	"Lake Worth",
	"Mocksville",
	"Lawai",
	"Hale",
	"Commerce",
	"Bell",
	"Cliff",
	"Guntown",
	"Millstone",
	"Black Creek",
	"Aceitunas",
	"Egeland",
	"Naukati",
	"Waipio",
	"Taneyville",
	"Millsboro",
	"Higgston",
	"New Columbus",
	"Van Vleck",
	"Landrum",
	"Hemlock",
	"Gibsonia",
	"Carnegie",
	"La Ward",
	"Arion",
	"Bend",
	"Warren South",
	"Dazey",
	"Pine Bush",
	"Goodrich",
	"Sinai",
	"Broxton",
	"Gulf Port",
	"Ceiba",
	"Los Banos",
	"Uniondale",
	"Regan",
	"Suttons",
	"Otisville",
	"Starkweather",
	"Idylwood",
	"Morgan",
	"Merced",
	"Arimo",
	"Miles",
	"Ruskin",
	"Orland",
	"Berger",
	"Southbridge",
	"Johnson",
	"Mondovi",
	"Louisiana",
	"Marcus Hook",
	"Bloomville",
	"Guerra",
	"East Rochester",
	"Clifton",
	"Pelion",
	"Ocean",
	"Bicknell",
	"Bonneauville",
	"Star Lake",
	"Great River",
	"Crosby",
	"Fruit",
	"Trion",
	"Yeagertown",
	"Stacy",
	"Parkman",
	"Nespelem",
	"Reeves",
	"Bascom",
	"Shippingport",
	"Hummels Wharf",
	"Hooverson",
	"New Stanton",
	"Locust Fork",
	"Bernalillo",
	"Blossom",
	"Pinch",
	"Trappe",
	"Renwick",
	"Wade",
	"White Springs",
	"Ammon",
	"Tightwad",
	"Panguitch",
	"Lozano",
	"Moreauville",
	"Miner",
	"Coto de Caza",
	"Heflin",
	"North Newton",
	"Ponder",
	"Bonfield",
	"Purdin",
	"Shelter",
	"Corfu",
	"Nielsville",
	"Bayou La Batre",
	"Asheville",
	"Bellmont",
	"Leonardtown",
	"Pottsville",
	"Surrency",
	"Grafton",
	"Rarden",
	"Wapanucka",
	"Monaca",
	"Mauriceville",
	"Black",
	"Krugerville",
	"Haugen",
	"Burgin",
	"South Hill",
	"Keeseville",
	"Murphys",
	"Elkhart",
	"Waterbury",
	"Winstonville",
	"Dandridge",
	"Hensley",
	"Davenport",
	"Amanda",
	"Brownell",
	"Ramblewood",
	"Woodcliff Lake",
	"Tyler",
	"Davy",
	"Slickville",
	"Saxonburg",
	"Rancho Santa Fe",
	"Northwest",
	"Aynor",
	"South Chicago",
	"Freedom",
	"Hayfork",
	"Cedar",
	"Belpre",
	"Gladewater",
	"Moorland",
	"Richmond West",
	"Hope Mills",
	"Perry",
	"Wilsall",
	"Summerland",
	"New Era",
	"Searsport",
	"Greensburg",
	"Fruit Cove",
	"Waller",
	"Los Chaves",
	"Rincon",
	"Valley View",
	"Corsica",
	"Argenta",
	"McAdenville",
	"Lakes",
	"Plainsboro",
	"Wadsworth",
	"East Gaffney",
	"Keego",
	"Hawesville",
	"Bowie",
	"Pembroke Pines",
	"Balsam Lake",
	"Wartrace",
	"Ringwood",
	"Burkettsville",
	"Jameson",
	"Cross Lanes",
	"Centennial",
	"Urbank",
	"Peralta",
	"Cresbard",
	"Lake Aluma",
	"Rippey",
	"Sale",
	"Glorieta",
	"North Webster",
	"Tornillo",
	"Walnut Springs",
	"Chaska",
	"Libby",
	"Ambia",
	"Birch",
	"Port Vincent",
	"Rossburg",
	"Veradale",
	"Plato",
	"Harrell",
	"Tupman",
	"Bogard",
	"Blue River",
	"Lake Lac La Belle",
	"Logan",
	"Turtle Creek",
	"Whitestown",
	"Tara",
	"Old Field",
	"Bennett",
	"New Harmony",
	"Malvern",
	"Santan",
	"Curtisville",
	"Taylorville",
	"Camp Wood",
	"Tiger",
	"East Dunbar",
	"Lemon Grove",
	"Yale",
	"Postville",
	"Vale",
	"Fall Branch",
	"Catano",
	"Edwardsport",
	"Old Bridge",
	"Worden",
	"Free Soil",
	"Nordheim",
	"Long Creek",
	"Dawson Springs",
	"Sholes",
	"Sunrise Beach",
	"Grantwood",
	"Bowmanstown",
	"New Rockford",
	"Miltona",
	"Benoit",
	"Horseheads North",
	"San Bruno",
	"Lindale",
	"South Lake Tahoe",
	"Wilber",
	"Astoria",
	"Startex",
	"Tekonsha",
	"Lechee",
	"San Felipe Pueblo",
	"Haralson",
	"Tillmans Corner",
	"West Peoria",
	"Drew",
	"Grape Creek",
	"Falconer",
	"Vina",
	"Supreme",
	"Hydetown",
	"Taft",
	"Arcadia",
	"Pinebluff",
	"Climax",
	"Grand",
	"Cornersville",
	"Aredale",
	"Manorville",
	"Country Lake",
	"Bevington",
	"Beverly",
	"Little Eagle",
	"Sunrise Manor",
	"La Joya",
	"Waldron",
	"St. Pete Beach",
	"New Smyrna",
	"Shellsburg",
	"East Blythe",
	"Willmar",
	"Honeyville",
	"Shabbona",
	"Monarch Mill",
	"Bigelow",
	"Charles ",
	"Honea Path",
	"Bridgeton",
	"Stacyville",
	"Fajardo",
	"Angola",
	"Estill Springs",
	"Fair Bluff",
	"Miami",
	"White Cloud",
	"Park Forest",
	"Mayodan",
	"Ellport",
	"Del Rey",
	"Alapaha",
	"Austell",
	"South Middletown",
	"Poth",
	"Camano",
	"McGuire",
	"Nelsonville",
	"Wimberley",
	"Relampago",
	"Beech Creek",
	"Keyes",
	"Cordova",
	"Macksburg",
	"Bagnell",
	"Neeses",
	"Naranjito",
	"Experiment",
	"Blyn",
	"Wheeling",
	"Vineyard",
	"Cheney",
	"McAlmont",
	"San Miguel",
	"Stanton",
	"Clarkson",
	"Laurel Lake",
	"Elk Falls",
	"Parkville",
	"Corwith",
	"Napakiak",
	"Greenport West",
	"Keosauqua",
	"Onamia",
	"Sutter Creek",
	"Mountain View",
	"Leal",
	"Waynesville",
	"Evadale",
	"Manitowoc",
	"Bulverde",
	"Ideal",
	"Blennerhassett",
	"Time",
	"La Fermina",
	"Tannersville",
	"Budd Lake",
	"Spring Ridge",
	"Minidoka",
	"Findlay",
	"Anvik",
	"Thousand Oaks",
	"Halbur",
	"Regal",
	"Antimony",
	"Hilshire",
	"Fort Belvoir",
	"Amesville",
	"Arrow Point",
	"Armour",
	"Mayer",
	"Randallstown",
	"Discovery",
	"Good Hope",
	"Tenakee Springs",
	"Heckscherville",
	"Bellflower",
	"East Bronson",
	"Dortches",
	"Guaynabo",
	"New Llano",
	"Beaver Creek",
	"Little Mountain",
	"Travilah",
	"Sharon",
	"Centre",
	"Echelon",
	"Lovilia",
	"Cherry",
	"Many Farms",
	"Carson",
	"Emerald Isle",
	"Piney Point",
	"Marshallton",
	"Zavalla",
	"Gillespie",
	"Schenectady",
	"Qulin",
	"Ellenton",
	"Tarentum",
	"Kennedy",
	"Jacinto",
	"Wiscasset",
	"East Sahuarita",
	"Somersworth",
	"East Camden",
	"Conetoe",
	"Netawaka",
	"Pontiac",
	"Avant",
	"Tamarack",
	"New Bloomington",
	"Swannanoa",
	"Coosada",
	"Huntsville",
	"Long Branch",
	"Elberfeld",
	"Byram",
	"Lakes by the",
	"Lewis Run",
	"Barker",
	"Chugwater",
	"Kevin",
	"Collinsville",
	"Riverlea",
	"Zurich",
	"Mandaree",
	"Percy",
	"Dacoma",
	"Lumberport",
	"Higbee",
	"Cashiers",
	"Iaeger",
	"Simms",
	"Ozark",
	"Slayden",
	"Maxton",
	"Corona de Tucson",
	"Sweet Water",
	"Solen",
	"Rhodell",
	"Fords Prairie",
	"North Philipsburg",
	"Finlayson",
	"Gulf Breeze",
	"Greenwood",
	"Wiley",
	"Houma",
	"Troutdale",
	"Barnegat",
	"Barker",
	"Eleele",
	"Tombstone",
	"Alpine",
	"Wallula",
	"Broussard",
	"Seminole",
	"Bethel Springs",
	"Branson",
	"Norbourne",
	"Mariposa",
	"Clintondale",
	"Tarkio",
	"Verlot",
	"McCall",
	"Sandusky",
	"Gilliam",
	"Altenburg",
	"Niantic",
	"Meservey",
	"Paddock Lake",
	"Rocky Mound",
	"Morada",
	"Ladoga",
	"Buckley",
	"Robeline",
	"Glastonbury",
	"Irwin",
	"Itasca",
	"Media",
	"Pecan Hill",
	"Cortland",
	"Cave",
	"Howards Grove",
	"Meadowlakes",
	"Wyomissing",
	"Lake Clarke Shores",
	"Dona Ana",
	"North Oaks",
	"Rigby",
	"Olivet",
	"Chicago",
	"Oldsmar",
	"Twain",
	"Brightwaters",
	"Norge",
	"Mountain Grove",
	"Owenton",
	"Kapalua",
	"Kings Grant",
	"Hinton",
	"Broeck Pointe",
	"Bay Hill",
	"Bay Lake",
	"Redan",
	"Haslet",
	"Waikoloa",
	"Catonsville",
	"Bonsall",
	"Atlantic",
	"Horntown",
	"Grass Range",
	"Clatonia",
	"Grand Acres",
	"Orchard Grass",
	"Alice",
	"Truxton",
	"Dixon",
	"Elberta",
	"Gray",
	"Crooked Lake",
	"Grand Blanc",
	"Tampico",
	"York Mills",
	"Union Point",
	"Sheldon",
	"Winchester",
	"Plains",
	"Ruma",
	"Roberts",
	"Alderson",
	"Jacksonville",
	"East Missoula",
	"Peavine",
	"Harris",
	"Palo",
	"Ramtown",
	"Sheakleyville",
	"Imlay",
	"Canova",
	"Grand Mound",
	"Lohman",
	"Cobleskill",
	"Old Orchard",
	"Wareham",
	"Morning Glory",
	"Seffner",
	"Rowena",
	"Canonsburg",
	"Rural Hill",
	"Red Hook",
	"Latham",
	"Asbury",
	"Hubbard",
	"Cashton",
	"Wedgewood",
// Provinces
	"Laguna Niguel",
	"Weedsport",
	"Ashtabula",
	"Victory",
	"East Can",
	"Fontenelle",
	"Huntland",
	"Redlands",
	"Davis Junction",
	"Fitzgerald",
	"Cheektowaga",
	"Callimont",
	"Sonoma",
	"Dunn Loring",
	"Campbells",
	"Lewellen",
	"Brattleboro",
	"New Brock",
	"Boone",
	"Assaria",
	"Venedocia",
	"Wallula",
	"Pukwana",
	"West Lebanon",
	"Hayden",
	"Winchendon",
	"Greenevers",
	"Esparto",
	"Detroit Lakes",
	"Braman",
	"Farmers Branch",
	"Waimalu",
	"Quebrada",
	"Sausalito",
	"West Yarmouth",
	"Okmulgee",
	"Fountain Green",
	"College",
	"Tolley",
	"Hershey",
	"Bradenton",
	"Cross Roads",
	"Rock Rapids",
	"Weldon",
	"Casselberry",
	"Holly",
	"O'Fallon",
	"Applewood",
	"Northwest",
	"Jesup",
	"Coles",
	"Maple Rapids",
	"Oyster Bay Cove",
	"Coates",
	"Quimby",
	"Beckett Ridge",
	"Fort Mill",
	"Minnetrista",
	"Wyandanch",
	"Archer",
	"Runnells",
	"Branford",
	"New Palestine",
	"Cushman",
	"Polson",
	"Edgecliff",
	"Bethpage",
	"North Alamo",
	"North Wardell",
	"Dunlevy",
	"San German",
	"Marcellus",
	"Tollette",
	"Richview",
	"Wolf Lake",
	"Carle Place",
	"Buncombe",
	"Early",
	"Taft Mosswood",
	"Pistakee Highlands",
	"Ullin",
	"DeKalb",
	"Muddy",
	"Broeck Pointe",
	"Raleigh Hills",
	"Ather",
	"St. Rose",
	"Romulus",
	"Waldoboro",
	"Bluejacket",
	"Forest Glen",
	"Massillon",
	"Algood",
	"Wrangell",
	"Wolcott",
	"Elsie",
	"Chauncey",
	"Granite Hills",
	"Sea Breeze",
	"South Fulwer",
	"Slana",
	"Struthers",
	"Jeddo",
	"Anawalt",
	"Bixby",
	"Azure",
	"Emden",
	"Falls",
	"Huxley",
	"Cheryla",
	"Westmont",
	"Sea Ranch Lakes",
	"New Tulsa",
	"Hempstead",
	"Manhasset Hills",
	"Tacoma",
	"Brush",
	"Umatilla",
	"Dot Lake",
	"East Palestine",
	"Great River",
	"Pine Glen",
	"China",
	"Luis Llorens Torres",
	"Fostoria",
	"McCullom Lake",
	"Maquoketa",
	"Antrim",
	"Big Flat",
	"Panola",
	"Bisbee",
	"Luna Pier",
	"St. Henry",
	"Tishomingo",
	"West Lake Hills",
	"North Seekonk",
	"Red Boiling Springs",
	"New Windsor",
	"Chalfant",
	"Foosland",
	"New Auburn",
	"Rose",
	"Glen Lyn",
	"Kittrell",
	"North Kensing",
	"Hansell",
	"Heber",
	"East Foothills",
	"Garner",
	"Chevy Chase",
	"Hosch",
	"Hopewell Junction",
	"Fossil",
	"York",
	"Barker Ten Mile",
	"Blowing Rock",
	"Tabor",
	"Rushmere",
	"Keys",
	"Yellow Bluff",
	"Drexel",
	"St. Joe",
	"Port Vue",
	"Newborn",
	"Acres Green",
	"Weston Mills",
	"Juno Ridge",
	"Fertile",
	"Logansport",
	"Princes Lakes",
	"Minerva",
	"Searcy",
	"Beaver",
	"Chickasha",
	"Zapata",
	"Sams Corner",
	"Ebens",
	"South Coates",
	"Takotna",
	"Wards",
	"Bethalto",
	"Parrotts",
	"Box Elder",
	"Moores",
	"Lumber",
	"Snow Hill",
	"Harrington",
	"Jamaica",
	"Walker Mill",
	"Healy",
	"Grand Canyon",
	"Thacker",
	"Mount Moriah",
	"Weyerhaeuser",
	"Williamsfield",
	"Wallins Creek",
	"Picayune",
	"South Shore",
	"Schneider",
	"Duluth",
	"La Prairie",
	"Richland Hills",
	"Coleraine",
	"Regent",
	"Pocola",
	"Knollwood",
	"Spearman",
	"Higden",
	"Canyon",
	"Progreso",
	"North Brunswick",
	"Gun Barrel",
	"Fruland",
	"Ulen",
	"Chadbourn",
	"Veneta",
	"Carbonado",
	"Reedley",
	"Norridgewock",
	"Nimrod",
	"Blue Bell",
	"Smiths",
	"Levasy",
	"Ellsworth",
	"Howards Grove",
	"South",
	"Collier",
	"Helena Valley Northwest",
	"Yampa",
	"Chubbuck",
	"Liscomb",
	"Espanola",
	"Ducor",
	"Deer Grove",
	"Upper Fruland",
	"Cool Valley",
	"Spencerport",
	"Willits",
	"Kincaid",
	"Bayside",
	"Salcha",
	"Risingsun",
	"Union",
	"Alpine Northwest",
	"Ogden",
	"Fyffe",
	"Redkey",
	"Akaska",
	"Port Lavaca",
	"Belle Rose",
	"Frews",
	"Walbridge",
	"Mead",
	"Plant",
	"Ranchos de Taos",
	"Carrizales",
	"West Ocean",
	"Vermillion",
	"Longdale",
	"Jacksboro",
	"Samak",
	"Wrights",
	"Duarte",
	"Mesquite",
	"Ann Arbor",
	"Gills",
	"Waupaca",
	"Dellroy",
	"Klawock",
	"Alligator",
	"Lovejoy",
	"Fingal",
	"Glenside",
	"Severance",
	"Los Molinos",
	"South Barring",
	"Ridgeley",
	"Hildale",
	"Mosses",
	"Okemah",
	"Post Oak Bend",
	"Frederick",
	"Osnabrock",
	"Elburn",
	"Congress",
	"Byrom",
	"Marana",
	"Concepcion",
	"Exeland",
	"Bethany",
	"Lake Placid",
	"Pennsauken",
	"Laurens",
	"Apopka",
	"De Smet",
	"Cold Brook",
	"Wink",
	"Sienna Plantation",
	"Newhalen",
	"Spring",
	"Furman",
	"Waukesha",
	"Albee",
	"Cranes",
	"Buckhead Ridge",
	"City View",
	"Munroe Falls",
	"Moosic",
	"West College Corner",
	"Lower Brule",
	"Davie",
	"Roseburg North",
	"Forest",
	"Limestone",
	"Ludlow",
	"Parkers Crossroads",
	"Beargrass",
	"Sargent",
	"Elkader",
	"Rothsay",
	"Wiederkehr",
	"Ranchos Penitas West",
	"Blanco",
	"Reminder",
	"Grandfield",
	"Pismo",
	"Country Club",
	"Crooked Creek",
	"Mission Hills",
	"Papaikou",
	"Morrison",
	"Fairview",
	"Pendergrass",
	"Pierz",
	"Mount Angel",
	"Orchidlands",
	"Marlborough",
	"Heuvel",
	"Kruger",
	"Hoffman",
	"Shilling",
	"Safford",
	"Ephesus",
	"Calabasas",
	"Maple",
	"Amazonia",
	"Foraker",
	"Tuskegee",
	"Healy Lake",
	"Portola",
	"Shoreline",
	"Ekwok",
	"Port Allen",
	"Murchison",
	"Park River",
	"Watts",
	"Grano",
	"Wrenshall",
	"Kipnuk",
	"Gouverneur",
	"Phil Campbell",
	"Glenolden",
	"East Prairie",
	"Miller",
	"Tarkio",
	"Evans",
	"Latimer",
	"Spotswood",
	"Carrier Mills",
	"Vandemere",
	"Ludden",
	"Gillett",
	"North Bos",
	"Rocky Hill",
	"Fall River",
	"Mount Hope",
	"Maury",
	"Coral Hills",
	"Golden",
	"Spur",
	"Meyersdale",
	"La Salle",
	"Moore",
	"Honea Path",
	"Slater",
	"Texhoma",
	"Hewlett Neck",
	"Campbell",
	"Pawleys",
	"Burnet",
	"Burwell",
	"Tuscaloosa",
	"Bayou Gauche",
	"Kodiak",
	"Citrus Springs",
	"Northome",
	"Lookeba",
	"Flowing Wells",
	"Ridge Wood",
	"Sunrise",
	"Dunnell",
	"Horners",
	"Julian",
	"Granite Bay",
	"Kinmundy",
	"Roth",
	"Woolsey",
	"Spiro",
	"Trotwood",
	"Oak Park",
	"Trinidad",
	"Corcoran",
	"Hicks",
	"Bokchito",
	"Laureldale",
	"Thompson",
	"Mount Charles",
	"Upper Nyack",
	"Trail Creek",
	"Pelican",
	"Litch",
	"Mancelona",
	"Three Lakes",
	"Urbandale",
	"Anguilla",
	"Stewart",
	"Chimney Rock",
	"O'Donnell",
	"Ellis Grove",
	"Rothbury",
	"Kalaoa",
	"Blue Rapids",
	"Hillside",
	"New Smyrna",
	"Hunts Point",
	"Springport",
	"Kenilworth",
	"Ethridge",
	"Gray",
	"Pick",
	"Albert",
	"Tonica",
	"Concil",
	"Pleasant Garden",
	"Vista West",
	"Meyers Lake",
	"Sonoita",
	"Bossier",
	"Malinta",
	"Vadnais",
	"Plantation",
	"Columbia",
	"Pierre",
	"Hous",
	"Green",
	"Fernley",
	"Red Lake",
	"Oak Hills",
	"Sugar Land",
	"Dames Quarter",
	"Ashburn",
	"Federals",
	"Flagstaff",
	"Yelm",
	"Crystal River",
	"Manheim",
	"Washburn",
	"Huetter",
	"Sandy Ridge",
	"Burnside",
	"Rose Bud",
	"Pettit",
	"Glenvar",
	"Peetz",
	"Camp Springs",
	"Rouses Point",
	"Mendenhall",
	"Bushong",
	"Mountain Brook",
	"Woodland Mills",
	"Cerritos",
	"Campus",
	"North Plainfield",
	"Goldonna",
	"Boyle",
	"Hurlock",
	"Nissequogue",
	"Armona",
	"North Judson",
	"Milaca",
	"Cuyahoga Falls",
	"Zwolle",
	"Alpaugh",
	"Hummels Wharf",
	"Ocean",
	"The Boonies",
	"Corozal",
	"Tularosa",
	"Manalapan",
	"Big Wells",
	"West Elmira",
	"Union Springs",
	"Wade Hamp",
	"Plum Branch",
	"Hazel Crest",
	"Butte",
	"Laddonia",
	"Meridian",
	"Cheat Lake",
	"Elk Rapids",
	"Wynnedale",
	"Cold Bay",
	"Annapolis",
	"Griffin",
	"Sabana",
	"Fairfax",
	"Mechanic Falls",
	"Ligonier",
	"Quinwood",
	"Gladbrook",
	"Carlsbad",
	"Newfane",
	"North Acomita",
	"Monahans",
	"Bethel",
	"Granite Falls",
	"Red Devil",
	"Caro",
	"Lamar",
	"Leola",
	"Lakeview North",
	"New Roads",
	"Adwolf",
	"Orcutt",
	"Cave Creek",
	"Belle Glade",
	"Tar Heel",
	"Choctaw Lake",
	"Penalosa",
	"Rancho Calaveras",
	"Watha",
	"Pinehurst",
	"Rutland",
	"Meservey",
	"Thorsby",
	"Overly",
	"Donnelly",
	"De Borgia",
	"New Knox",
	"Conyers",
	"Lithium",
	"Grambling",
	"Pupukea",
	"Gem Lake",
	"Moffett",
	"Cochrane",
	"Niangua",
	"Hamtramck",
	"Moores Mill",
	"Terrell Hills",
	"Menlo",
	"Lake San Marcos",
	"Lake Fen",
	"Rockwell",
	"Carlin",
	"Celina",
	"Viburnum",
	"Wampum",
	"Pasco",
	"Olivarez",
	"Sidon",
	"Edgewater",
	"Gwinner",
	"Daly",
	"Moultrie",
	"Goldsmith",
	"Adelanto",
	"Schaeffers",
	"Fort Belvoir",
	"Cuney",
	"South Rockwood",
	"Cliffside",
	"Dunseith",
	"Grannis",
	"Linneus",
	"Powell",
	"Weidman",
	"La Cygne",
	"Cross",
	"Michigamme",
	"Hamers",
	"Cupertino",
	"Silver Lake",
	"North Potomac",
	"New Cordell",
	"Linn Grove",
	"Kenneth",
	"Camargo",
	"Swansea",
	"Upper Brook",
	"Fair Oaks",
	"Garceno",
	"Farm Loop",
	"Whittingham",
	"Sutter",
	"North Robinson",
	"Richland",
	"Sand Rock",
	"Parksdale",
	"Lakin",
	"Harvey",
	"Conners",
	"Sarles",
	"Tulsita",
	"Farmingdale",
	"Cumberland",
	"Allens",
	"Rensselaer Falls",
	"Reyno",
	"Deer River",
	"Two Rivers",
	"Shavano",
	"Williams Bay",
	"Hitchita",
	"Esbon",
	"Ashby",
	"Moorefield",
	"Eunice",
	"Kiel",
	"Santa Fe Springs",
	"Macomb",
	"King",
	"Fishhook",
	"Aynor",
	"Cashiers",
	"Tangipahoa",
	"Atascadero",
	"Margate",
	"Norwood",
	"Spanish Lake",
	"National",
	"Coppell",
	"Bluewater",
	"Regal",
	"Richwood",
	"Brookfield",
	"Cooter",
	"Wilming",
	"Maple Plain",
	"Fountain Lakes",
	"Flora",
	"Optima",
	"Barnhart",
	"Foley",
	"Coventry Lake",
	"Eastchester",
	"Grass Lake",
	"Scran",
	"Bon Air",
	"La Veta",
	"San Anselmo",
	"Orange",
	"Cedar Grove",
	"Coal Fork",
	"Bradford",
	"Diamondhead",
	"Fairchance",
	"Naylor",
	"Eustace",
	"Idabel",
	"Spade",
	"Mud Bay",
	"East Prospect",
	"Crow Agency",
	"Alice",
	"Mountainair",
	"Narragansett",
	"Shannon",
	"Westphalia",
	"Kosciusko",
	"Hanson",
	"Jacksonville",
	"Lackland",
	"Southport",
	"Elbing",
	"Fisher",
	"Scarsdale",
	"Hilshire",
	"West Marion",
	"North Bethesda",
	"Coamo",
	"Santa Teresa",
	"Tariff",
	"The Plains",
	"Portia",
	"Silver Ridge",
	"Amidon",
	"Cottage Lake",
	"Atlantic",
	"Plain View",
	"Daisy",
	"Scappoose",
	"Mexia",
	"Priest Point",
	"Jewell",
	"Mount Etna",
	"Tull",
	"Buell",
	"Northrop",
	"San Juan",
	"Balrog Point",
	"Angus",
	"South Palm",
	"Lenz",
	"Iroquois",
	"Grandview",
	"Renova",
	"Jericho",
	"Haymarket",
	"Fourche",
	"Gallitzin",
	"Hilltop",
	"Marydel",
	"El Mirage",
	"Medon",
	"Bickle",
	"Madawaska",
	"Lone Centaur Pine",
	"Carrs",
	"Lyncourt",
	"Yellow Springs",
	"Hickam Housing",
	"East Wenatchee",
	"Cincinnati",
	"Kinderhook",
	"Gonzales",
	"Algonquin",
	"Fern",
	"Moose Creek",
	"Pine Island Ridge",
	"Muscoy",
	"Meadow Lark Lake",
	"Kirby",
	"Oatfield",
	"Dallas",
	"Gran",
	"Dennis Port",
	"Fanshawe",
	"Plum",
	"Palco",
	"Gibsland",
	"Windemere",
	"Green Spring",
	"Crested Butte",
	"Plattsburgh",
	"Berkley",
	"Lamoni",
	"Double Springs",
	"Kensing",
	"Wesley",
	"Baraboo",
	"Niagara",
	"Ridgefield",
	"Success",
	"Billington",
	"Hollywood",
	"Lagrange",
	"Citronelle",
	"Cottage",
	"Alatna",
	"The Badlands",
	"Moose Wilson",
	"Palos",
	"Paradis",
	"Grant ",
	"Zilwaukee",
	"Knierim",
	"North Salt Lake",
	"Knobel",
	"Laguna Vista",
	"Thomas",
	"Greater Northdale",
	"Wykoff",
	"Ontario",
	"Bexley",
	"Kinney",
	"West Bay Shore",
	"Lilburn",
	"New Ulm",
	"Fenwood",
	"Safety",
	"Sinking Spring",
	"Jenners",
	"Seminary",
	"Otter Lake",
	"Stanley",
	"Moose Lake",
	"Buckhannon",
	"Cape Neddick",
	"McCall",
	"Fairway",
	"Las Ollas",
	"Clatskanie",
	"Lozano",
	"Faribault",
	"Rocklin",
	"Bowbells",
	"Cedar Hill Lakes",
	"Mendocino",
	"Kennebec",
	"Rochester",
	"Dragon Swimming",
	"Aubrey",
	"Mount Pulaski",
	"McCaskill",
	"Nucla",
	"Mira Monte",
	"Harwich Port",
	"Ocean Acres",
	"Happy Valley",
	"Larson",
	"Wolf Point",
	"Bloom",
	"Southamp",
	"Bucksport",
	"Marthas",
	"Belmond",
	"Thief River Falls",
	"Medora",
	"Anasco",
	"Hacienda",
	"Rose Lodge",
	"Central",
	"West Union",
	"Ottumwa",
	"Broad Brook",
	"Medford Lakes",
	"Saunemin",
	"Ackworth",
	"Three Rivers",
	"Otter",
	"Motain",
	"Woods Bay",
	"Leakey",
	"Meadow Glade",
	"Brent",
	"South Alamo",
	"Hammondsport",
	"Asher",
	"Carl Junction",
	"Rock",
	"Pilot Knob",
	"Pleasure",
	"White Swan",
	"East Brunswick",
	"Rudyard",
	"East Porter",
	"Allenhurst",
	"Dows",
	"Hutchins",
	"Frye",
	"Greenwood Lake",
	"South Connells",
	"Carring",
	"Springlake",
	"Gervais",
	"Pelzer",
	"Sandoval",
	"Selfridge",
	"Pewamo",
	"Old Monroe",
	"Troll Pillow",
	"Unionville",
	"Ghent",
	"McCormick",
	"Sunburst",
	"Holloman",
	"Copper",
	"Abby",
	"Sherando",
	"Merkel",
	"Cutten",
	"Beach Haven West",
	"Paragon",
	"Ogres Bite",
	"Odem",
	"Bladens",
	"Trent",
	"Valley Brook",
	"Snead",
	"Horatio",
	"South Range",
	"Ocean Pine",
	"Mars",
	"England",
	"Capitol",
	"Ravenwood",
	"Granite Quarry",
	"Parish",
	"Chadwick",
	"Glenarden",
	"Krotz Springs",
	"Luck",
	"Govan",
	"Rondo",
	"Pilger",
	"Glovers",
	"Perry",
	"Miami Lakes",
	"Barnegat",
	"Vernal",
	"Willernie",
	"Tekoa",
	"Cavalier",
	"Weed",
	"Amalga",
	"Nyack",
	"Pawnee",
	"Fort Ripley",
	"Russell",
	"G. L. Garcia",
	"Lich Hollow",
	"Hillburn",
	"Centerfield",
	"Vilonia",
	"Lakemore",
	"Farmland",
	"Kings",
	"Parker",
	"Hord",
	"Elys",
	"Pahala",
	"Valley Green",
	"Lost River",
	"Babbitt",
	"Altamont",
	"Absecon",
	"La Vale",
	"Paynes",
	"Elaine",
	"Tower",
	"Walterboro",
	"Bryson",
	"Montebello",
	"Kittredge",
	"Bridgman",
	"Thermalito",
	"Hustisford",
	"Braddock Hills",
	"Northway",
	"Mountain Ranch",
	"Lillie",
	"Nunapitchuk",
	"Evening Shade",
	"Olathe",
	"Atqasuk",
	"Rio Rancho",
	"Fortine",
	"Marble Hill",
	"States",
	"Louann",
	"Sandpoint",
	"Walnutport",
	"Reese",
	"Penns",
	"Haswell",
	"Wessing",
	"Ithaca",
	"Westport",
	"Tennant",
	"Cienegas",
	"Tobin",
	"Elvas",
	"Niobrara",
	"Scherer",
	"Lutz",
	"Hannibal",
	"Mithril Vale",
	"Pickens",
	"Shipshewana",
	"River",
	"Forest River",
	"Hurst",
	"Oxford",
	"South Taft",
	"Auberry",
	"Desert Hot Springs",
	"Orofino",
	"Chualar",
	"Effingham",
	"Palouse",
	"Matewan",
	"Cane Savannah",
	"Neihart",
	"Winifred",
	"Wurtsboro",
	"Helper",
	"The Valley of the Skull",
	"Kiana",
	"Edinburgh",
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
	Awrite(AString("Towns ") + ntowns);
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

char *AGetNameString( int name )
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

	// Leave the Nexus level in even if we're not going to use it;
	// It makes the main body of code more general
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

    // We can leave this one - it sets the starting cities
    if (Globals->NEXUS_EXISTS)
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
            if (r < 32) return R_CERAN_HILL;
            if (r < 50) return R_MOUNTAIN;
            if (r < 58) return R_FOREST;
            return R_PLAIN;
        case 1: /* Colder regions */
            if (r < 16) return R_PLAIN;
            if (r < 32) return R_FOREST;
            if (r < 42) return R_CERAN_HILL;
            if (r < 48) return R_MOUNTAIN;
            return R_SWAMP;
        case 2: /* Warmer regions */
            if (r < 20) return R_PLAIN;
            if (r < 28) return R_FOREST;
            if (r < 36) return R_CERAN_HILL;
            if (r < 42) return R_MOUNTAIN;
            if (r < 48) return R_SWAMP;
            if (r < 52) return R_JUNGLE;
            return R_DESERT;
        case 3: /* tropical */
            if (r < 16) return R_PLAIN;
            if (r < 22) return R_CERAN_HILL;
            if (r < 28) return R_MOUNTAIN;
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
		// Topmost level is full size in x direction
		if(level == 2) return 1;
		// All other levels are 1/2 size
		return 2;
	}
	if(level >= Globals->UNDERWORLD_LEVELS+2 &&
			level < (Globals->UNDERWORLD_LEVELS+Globals->UNDERDEEP_LEVELS+2)){
		// Topmost underdeep level is 1/2 size
		if(level == Globals->UNDERWORLD_LEVELS+2) return 2;
		// All others are 1/4 size
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
	if(type == R_OCEAN) return 0;
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
    if(!Globals->TOWNS_EXIST) return;

    if(Globals->GATES_EXIST) gate = -1;
    if( !town )
    {
        AddTown();
    }
	
    if(!Globals->START_CITIES_EXIST) return;

    town->pop = 5000;
    town->basepop = 5000;

	float ratio;
	Market *m;
    markets.DeleteAll();
	if(Globals->START_CITIES_START_UNLIMITED) {
		for (int i=0; i<NITEMS; i++) {
			if( ItemDefs[i].flags & ItemType::DISABLED) continue;
			if( ItemDefs[ i ].type & IT_NORMAL ) {
				if (i==I_SILVER || i==I_LIVESTOCK || i==I_FISH || i==I_GRAIN)
					continue;
				m = new Market(M_BUY,i,(ItemDefs[i].baseprice * 5 / 2),-1,
						5000,5000,-1,-1);
				markets.Add(m);
			}
		}
		ratio = ItemDefs[race].baseprice / (float)Globals->BASE_MAN_COST;
		m=new Market(M_BUY,race,(int)(Wages()*4*ratio),-1,5000,5000,-1,-1);
		markets.Add(m);
		if(Globals->LEADERS_EXIST) {
			ratio=ItemDefs[I_LEADERS].baseprice/(float)Globals->BASE_MAN_COST;
			m = new Market(M_BUY,I_LEADERS,(int)(Wages()*4*ratio),-1,
					5000,5000,-1,-1);
			markets.Add(m);
		}
	} else {
		SetupCityMarket();
		ratio = ItemDefs[race].baseprice / (float)Globals->BASE_MAN_COST;
		/* Setup Recruiting */
		m = new Market( M_BUY, race, (int)(Wages()*4*ratio),
				Population()/5, 0, 10000, 0, 2000 );
		markets.Add(m);
		if( Globals->LEADERS_EXIST ) {
			ratio=ItemDefs[I_LEADERS].baseprice/(float)Globals->BASE_MAN_COST;
			m = new Market( M_BUY, I_LEADERS, (int)(Wages()*4*ratio),
					Population()/25, 0, 10000, 0, 400 );
			markets.Add(m);
		}
	}
}

int ARegion::IsStartingCity() {
    if (town && town->pop == 5000) return 1;
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
    while (!reg && tries < 10000) {
        //
        // We'll just let AC exits be all over the map.
        //
        int x = getrandom( maxX );
        int y = 2 * getrandom( maxY / 2 ) + x % 2;

        reg = pArr->GetRegion( x, y);

        if(!reg->CanBeStartingCity( pArr )) {
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
		if(reg->type == R_OCEAN) {
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

