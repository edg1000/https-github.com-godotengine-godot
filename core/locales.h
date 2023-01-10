/**************************************************************************/
/*  locales.h                                                             */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef LOCALES_H
#define LOCALES_H

// Windows has some weird locale identifiers which do not honor the ISO 639-1
// standardized nomenclature. Whenever those don't conflict with existing ISO
// identifiers, we override them.
//
// Reference:
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms693062(v=vs.85).aspx

static const char *locale_renames[][2] = {
	{ "in", "id" }, //  Indonesian
	{ "iw", "he" }, //  Hebrew
	{ "no", "nb" }, //  Norwegian Bokmål
	{ "C", "en" }, // Locale is not set, fallback to English.
	{ nullptr, nullptr }
};

// Additional script information to preferred scripts.
// Language code, script code, default country, supported countries.
// Reference:
// - https://lh.2xlibre.net/locales/
// - https://www.localeplanet.com/icu/index.html
// - https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-lcid/70feba9f-294e-491e-b6eb-56532684c37f

static const char *locale_scripts[][4] = {
	{ "az", "Latn", "", "AZ" },
	{ "az", "Arab", "", "IR" },
	{ "bs", "Latn", "", "BA" },
	{ "ff", "Latn", "", "BF,CM,GH,GM,GN,GW,LR,MR,NE,NG,SL,SN" },
	{ "pa", "Arab", "PK", "PK" },
	{ "pa", "Guru", "IN", "IN" },
	{ "sd", "Arab", "PK", "PK" },
	{ "sd", "Deva", "IN", "IN" },
	{ "shi", "Tfng", "", "MA" },
	{ "sr", "Cyrl", "", "BA,RS,XK" },
	{ "sr", "Latn", "", "ME" },
	{ "uz", "Latn", "", "UZ" },
	{ "uz", "Arab", "AF", "AF" },
	{ "vai", "Vaii", "", "LR" },
	{ "yue", "Hans", "CN", "CN" },
	{ "yue", "Hant", "HK", "HK" },
	{ "zh", "Hans", "CN", "CN,SG" },
	{ "zh", "Hant", "TW", "HK,MO,TW" },
	{ nullptr, nullptr, nullptr, nullptr }
};

// Additional mapping for outdated, temporary or exceptionally reserved country codes.
// Reference:
// - https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2
// - https://www.iso.org/obp/ui/#search/code/

static const char *country_renames[][2] = {
	{ "BU", "MM" }, // Burma, name changed to Myanmar.
	{ "KV", "XK" }, // Kosovo (temporary FIPS code to European Commission code), no official ISO code assigned.
	{ "TP", "TL" }, // East Timor, name changed to Timor-Leste.
	{ "UK", "GB" }, // United Kingdom, exceptionally reserved code.
	{ nullptr, nullptr }
};

// Country code, country name.
// Reference:
// - https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2
// - https://www.iso.org/obp/ui/#search/code/

static const char *country_names[][2] = {
	{ "AC", "Ascension Island" }, // Exceptionally reserved.
	{ "AD", "Andorra" },
	{ "AE", "United Arab Emirates" },
	{ "AF", "Afghanistan" },
	{ "AG", "Antigua and Barbuda" },
	{ "AI", "Anguilla" },
	{ "AL", "Albania" },
	{ "AM", "Armenia" },
	{ "AN", "Netherlands Antilles" }, // Transitionally reserved, divided into BQ, CW and SX.
	{ "AO", "Angola" },
	{ "AQ", "Antarctica" },
	{ "AR", "Argentina" },
	{ "AS", "American Samoa" },
	{ "AT", "Austria" },
	{ "AU", "Australia" },
	{ "AW", "Aruba" },
	{ "AX", "Åland Islands" },
	{ "AZ", "Azerbaijan" },
	{ "BA", "Bosnia and Herzegovina" },
	{ "BB", "Barbados" },
	{ "BD", "Bangladesh" },
	{ "BE", "Belgium" },
	{ "BF", "Burkina Faso" },
	{ "BG", "Bulgaria" },
	{ "BH", "Bahrain" },
	{ "BI", "Burundi" },
	{ "BJ", "Benin" },
	{ "BL", "St. Barthélemy" },
	{ "BM", "Bermuda" },
	{ "BN", "Brunei" },
	{ "BO", "Bolivia" },
	{ "BQ", "Caribbean Netherlands" },
	{ "BR", "Brazil" },
	{ "BS", "Bahamas" },
	{ "BT", "Bhutan" },
	{ "BV", "Bouvet Island" },
	{ "BW", "Botswana" },
	{ "BY", "Belarus" },
	{ "BZ", "Belize" },
	{ "CA", "Canada" },
	{ "CC", "Cocos (Keeling) Islands" },
	{ "CD", "Congo - Kinshasa" },
	{ "CF", "Central African Republic" },
	{ "CG", "Congo - Brazzaville" },
	{ "CH", "Switzerland" },
	{ "CI", "Côte d'Ivoire" },
	{ "CK", "Cook Islands" },
	{ "CL", "Chile" },
	{ "CM", "Cameroon" },
	{ "CN", "China" },
	{ "CO", "Colombia" },
	{ "CP", "Clipperton Island" }, // Exceptionally reserved.
	{ "CR", "Costa Rica" },
	{ "CQ", "Island of Sark" }, // Exceptionally reserved.
	{ "CU", "Cuba" },
	{ "CV", "Cabo Verde" },
	{ "CW", "Curaçao" },
	{ "CX", "Christmas Island" },
	{ "CY", "Cyprus" },
	{ "CZ", "Czechia" },
	{ "DE", "Germany" },
	{ "DG", "Diego Garcia" }, // Exceptionally reserved.
	{ "DJ", "Djibouti" },
	{ "DK", "Denmark" },
	{ "DM", "Dominica" },
	{ "DO", "Dominican Republic" },
	{ "DZ", "Algeria" },
	{ "EA", "Ceuta and Melilla" }, // Exceptionally reserved.
	{ "EC", "Ecuador" },
	{ "EE", "Estonia" },
	{ "EG", "Egypt" },
	{ "EH", "Western Sahara" },
	{ "ER", "Eritrea" },
	{ "ES", "Spain" },
	{ "ET", "Ethiopia" },
	{ "EU", "European Union" }, // Exceptionally reserved.
	{ "EZ", "Eurozone" }, // Exceptionally reserved.
	{ "FI", "Finland" },
	{ "FJ", "Fiji" },
	{ "FK", "Falkland Islands" },
	{ "FM", "Micronesia" },
	{ "FO", "Faroe Islands" },
	{ "FR", "France" },
	{ "FX", "France, Metropolitan" }, // Exceptionally reserved.
	{ "GA", "Gabon" },
	{ "GB", "United Kingdom" },
	{ "GD", "Grenada" },
	{ "GE", "Georgia" },
	{ "GF", "French Guiana" },
	{ "GG", "Guernsey" },
	{ "GH", "Ghana" },
	{ "GI", "Gibraltar" },
	{ "GL", "Greenland" },
	{ "GM", "Gambia" },
	{ "GN", "Guinea" },
	{ "GP", "Guadeloupe" },
	{ "GQ", "Equatorial Guinea" },
	{ "GR", "Greece" },
	{ "GS", "South Georgia and South Sandwich Islands" },
	{ "GT", "Guatemala" },
	{ "GU", "Guam" },
	{ "GW", "Guinea-Bissau" },
	{ "GY", "Guyana" },
	{ "HK", "Hong Kong" },
	{ "HM", "Heard Island and McDonald Islands" },
	{ "HN", "Honduras" },
	{ "HR", "Croatia" },
	{ "HT", "Haiti" },
	{ "HU", "Hungary" },
	{ "IC", "Canary Islands" }, // Exceptionally reserved.
	{ "ID", "Indonesia" },
	{ "IE", "Ireland" },
	{ "IL", "Israel" },
	{ "IM", "Isle of Man" },
	{ "IN", "India" },
	{ "IO", "British Indian Ocean Territory" },
	{ "IQ", "Iraq" },
	{ "IR", "Iran" },
	{ "IS", "Iceland" },
	{ "IT", "Italy" },
	{ "JE", "Jersey" },
	{ "JM", "Jamaica" },
	{ "JO", "Jordan" },
	{ "JP", "Japan" },
	{ "KE", "Kenya" },
	{ "KG", "Kyrgyzstan" },
	{ "KH", "Cambodia" },
	{ "KI", "Kiribati" },
	{ "KM", "Comoros" },
	{ "KN", "St. Kitts and Nevis" },
	{ "KP", "North Korea" },
	{ "KR", "South Korea" },
	{ "KW", "Kuwait" },
	{ "KY", "Cayman Islands" },
	{ "KZ", "Kazakhstan" },
	{ "LA", "Laos" },
	{ "LB", "Lebanon" },
	{ "LC", "St. Lucia" },
	{ "LI", "Liechtenstein" },
	{ "LK", "Sri Lanka" },
	{ "LR", "Liberia" },
	{ "LS", "Lesotho" },
	{ "LT", "Lithuania" },
	{ "LU", "Luxembourg" },
	{ "LV", "Latvia" },
	{ "LY", "Libya" },
	{ "MA", "Morocco" },
	{ "MC", "Monaco" },
	{ "MD", "Moldova" },
	{ "ME", "Montenegro" },
	{ "MF", "St. Martin" },
	{ "MG", "Madagascar" },
	{ "MH", "Marshall Islands" },
	{ "MK", "North Macedonia" },
	{ "ML", "Mali" },
	{ "MM", "Myanmar" },
	{ "MN", "Mongolia" },
	{ "MO", "Macao" },
	{ "MP", "Northern Mariana Islands" },
	{ "MQ", "Martinique" },
	{ "MR", "Mauritania" },
	{ "MS", "Montserrat" },
	{ "MT", "Malta" },
	{ "MU", "Mauritius" },
	{ "MV", "Maldives" },
	{ "MW", "Malawi" },
	{ "MX", "Mexico" },
	{ "MY", "Malaysia" },
	{ "MZ", "Mozambique" },
	{ "NA", "Namibia" },
	{ "NC", "New Caledonia" },
	{ "NE", "Niger" },
	{ "NF", "Norfolk Island" },
	{ "NG", "Nigeria" },
	{ "NI", "Nicaragua" },
	{ "NL", "Netherlands" },
	{ "NO", "Norway" },
	{ "NP", "Nepal" },
	{ "NR", "Nauru" },
	{ "NU", "Niue" },
	{ "NZ", "New Zealand" },
	{ "OM", "Oman" },
	{ "PA", "Panama" },
	{ "PE", "Peru" },
	{ "PF", "French Polynesia" },
	{ "PG", "Papua New Guinea" },
	{ "PH", "Philippines" },
	{ "PK", "Pakistan" },
	{ "PL", "Poland" },
	{ "PM", "St. Pierre and Miquelon" },
	{ "PN", "Pitcairn Islands" },
	{ "PR", "Puerto Rico" },
	{ "PS", "Palestine" },
	{ "PT", "Portugal" },
	{ "PW", "Palau" },
	{ "PY", "Paraguay" },
	{ "QA", "Qatar" },
	{ "RE", "Réunion" },
	{ "RO", "Romania" },
	{ "RS", "Serbia" },
	{ "RU", "Russia" },
	{ "RW", "Rwanda" },
	{ "SA", "Saudi Arabia" },
	{ "SB", "Solomon Islands" },
	{ "SC", "Seychelles" },
	{ "SD", "Sudan" },
	{ "SE", "Sweden" },
	{ "SG", "Singapore" },
	{ "SH", "St. Helena, Ascension and Tristan da Cunha" },
	{ "SI", "Slovenia" },
	{ "SJ", "Svalbard and Jan Mayen" },
	{ "SK", "Slovakia" },
	{ "SL", "Sierra Leone" },
	{ "SM", "San Marino" },
	{ "SN", "Senegal" },
	{ "SO", "Somalia" },
	{ "SR", "Suriname" },
	{ "SS", "South Sudan" },
	{ "ST", "Sao Tome and Principe" },
	{ "SV", "El Salvador" },
	{ "SX", "Sint Maarten" },
	{ "SY", "Syria" },
	{ "SZ", "Eswatini" },
	{ "TA", "Tristan da Cunha" }, // Exceptionally reserved.
	{ "TC", "Turks and Caicos Islands" },
	{ "TD", "Chad" },
	{ "TF", "French Southern Territories" },
	{ "TG", "Togo" },
	{ "TH", "Thailand" },
	{ "TJ", "Tajikistan" },
	{ "TK", "Tokelau" },
	{ "TL", "Timor-Leste" },
	{ "TM", "Turkmenistan" },
	{ "TN", "Tunisia" },
	{ "TO", "Tonga" },
	{ "TR", "Turkey" },
	{ "TT", "Trinidad and Tobago" },
	{ "TV", "Tuvalu" },
	{ "TW", "Taiwan" },
	{ "TZ", "Tanzania" },
	{ "UA", "Ukraine" },
	{ "UG", "Uganda" },
	{ "UM", "U.S. Outlying Islands" },
	{ "US", "United States of America" },
	{ "UY", "Uruguay" },
	{ "UZ", "Uzbekistan" },
	{ "VA", "Holy See" },
	{ "VC", "St. Vincent and the Grenadines" },
	{ "VE", "Venezuela" },
	{ "VG", "British Virgin Islands" },
	{ "VI", "U.S. Virgin Islands" },
	{ "VN", "Viet Nam" },
	{ "VU", "Vanuatu" },
	{ "WF", "Wallis and Futuna" },
	{ "WS", "Samoa" },
	{ "XK", "Kosovo" }, // Temporary code, no official ISO code assigned.
	{ "YE", "Yemen" },
	{ "YT", "Mayotte" },
	{ "ZA", "South Africa" },
	{ "ZM", "Zambia" },
	{ "ZW", "Zimbabwe" },
	{ nullptr, nullptr }
};

// Languages code, language name.
// Reference:
// - https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
// - https://www.localeplanet.com/icu/index.html
// - https://lh.2xlibre.net/locales/

static const char *language_list[][2] = {
	{ "aa", "Afar" },
	{ "ab", "Abkhazian" },
	{ "ace", "Achinese" },
	{ "ach", "Acoli" },
	{ "ada", "Adangme" },
	{ "ady", "Adyghe" },
	{ "ae", "Avestan" },
	{ "aeb", "Tunisian Arabic" },
	{ "af", "Afrikaans" },
	{ "afh", "Afrihili" },
	{ "agq", "Aghem" },
	{ "ain", "Ainu" },
	{ "agr", "Aguaruna" },
	{ "ak", "Akan" },
	{ "akk", "Akkadian" },
	{ "akz", "Alabama" },
	{ "ale", "Aleut" },
	{ "aln", "Gheg Albanian" },
	{ "alt", "Southern Altai" },
	{ "am", "Amharic" },
	{ "an", "Aragonese" },
	{ "ang", "Old English" },
	{ "anp", "Angika" },
	{ "ar", "Arabic" },
	{ "arc", "Aramaic" },
	{ "arn", "Mapudungun" },
	{ "aro", "Araona" },
	{ "arp", "Arapaho" },
	{ "arq", "Algerian Arabic" },
	{ "ars", "Najdi Arabic" },
	{ "arw", "Arawak" },
	{ "ary", "Moroccan Arabic" },
	{ "arz", "Egyptian Arabic" },
	{ "as", "Assamese" },
	{ "asa", "Asu" },
	{ "ase", "American Sign Language" },
	{ "ast", "Asturian" },
	{ "av", "Avaric" },
	{ "avk", "Kotava" },
	{ "awa", "Awadhi" },
	{ "ayc", "Southern Aymara" },
	{ "ay", "Aymara" },
	{ "az", "Azerbaijani" },
	{ "ba", "Bashkir" },
	{ "bal", "Baluchi" },
	{ "ban", "Balinese" },
	{ "bar", "Bavarian" },
	{ "bas", "Bassa" },
	{ "bax", "Bamun" },
	{ "bbc", "Batak Toba" },
	{ "bbj", "Ghomala" },
	{ "be", "Belarusian" },
	{ "bej", "Beja" },
	{ "bem", "Bemba" },
	{ "ber", "Berber" },
	{ "bew", "Betawi" },
	{ "bez", "Bena" },
	{ "bfd", "Bafut" },
	{ "bfq", "Badaga" },
	{ "bg", "Bulgarian" },
	{ "bhb", "Bhili" },
	{ "bgn", "Western Balochi" },
	{ "bho", "Bhojpuri" },
	{ "bi", "Bislama" },
	{ "bik", "Bikol" },
	{ "bin", "Bini" },
	{ "bjn", "Banjar" },
	{ "bkm", "Kom" },
	{ "bla", "Siksika" },
	{ "bm", "Bambara" },
	{ "bn", "Bengali" },
	{ "bo", "Tibetan" },
	{ "bpy", "Bishnupriya" },
	{ "bqi", "Bakhtiari" },
	{ "br", "Breton" },
	{ "brh", "Brahui" },
	{ "brx", "Bodo" },
	{ "bs", "Bosnian" },
	{ "bss", "Akoose" },
	{ "bua", "Buriat" },
	{ "bug", "Buginese" },
	{ "bum", "Bulu" },
	{ "byn", "Bilin" },
	{ "byv", "Medumba" },
	{ "ca", "Catalan" },
	{ "cad", "Caddo" },
	{ "car", "Carib" },
	{ "cay", "Cayuga" },
	{ "cch", "Atsam" },
	{ "ccp", "Chakma" },
	{ "ce", "Chechen" },
	{ "ceb", "Cebuano" },
	{ "cgg", "Chiga" },
	{ "ch", "Chamorro" },
	{ "chb", "Chibcha" },
	{ "chg", "Chagatai" },
	{ "chk", "Chuukese" },
	{ "chm", "Mari" },
	{ "chn", "Chinook Jargon" },
	{ "cho", "Choctaw" },
	{ "chp", "Chipewyan" },
	{ "chr", "Cherokee" },
	{ "chy", "Cheyenne" },
	{ "cic", "Chickasaw" },
	{ "ckb", "Central Kurdish" },
	{ "csb", "Kashubian" },
	{ "cmn", "Mandarin Chinese" },
	{ "co", "Corsican" },
	{ "cop", "Coptic" },
	{ "cps", "Capiznon" },
	{ "cr", "Cree" },
	{ "crh", "Crimean Tatar" },
	{ "crs", "Seselwa Creole French" },
	{ "cs", "Czech" },
	{ "csb", "Kashubian" },
	{ "cu", "Church Slavic" },
	{ "cv", "Chuvash" },
	{ "cy", "Welsh" },
	{ "da", "Danish" },
	{ "dak", "Dakota" },
	{ "dar", "Dargwa" },
	{ "dav", "Taita" },
	{ "de", "German" },
	{ "del", "Delaware" },
	{ "den", "Slave" },
	{ "dgr", "Dogrib" },
	{ "din", "Dinka" },
	{ "dje", "Zarma" },
	{ "doi", "Dogri" },
	{ "dsb", "Lower Sorbian" },
	{ "dtp", "Central Dusun" },
	{ "dua", "Duala" },
	{ "dum", "Middle Dutch" },
	{ "dv", "Dhivehi" },
	{ "dyo", "Jola-Fonyi" },
	{ "dyu", "Dyula" },
	{ "dz", "Dzongkha" },
	{ "dzg", "Dazaga" },
	{ "ebu", "Embu" },
	{ "ee", "Ewe" },
	{ "efi", "Efik" },
	{ "egl", "Emilian" },
	{ "egy", "Ancient Egyptian" },
	{ "eka", "Ekajuk" },
	{ "el", "Greek" },
	{ "elx", "Elamite" },
	{ "en", "English" },
	{ "enm", "Middle English" },
	{ "eo", "Esperanto" },
	{ "es", "Spanish" },
	{ "esu", "Central Yupik" },
	{ "et", "Estonian" },
	{ "eu", "Basque" },
	{ "ewo", "Ewondo" },
	{ "ext", "Extremaduran" },
	{ "fa", "Persian" },
	{ "fan", "Fang" },
	{ "fat", "Fanti" },
	{ "ff", "Fulah" },
	{ "fi", "Finnish" },
	{ "fil", "Filipino" },
	{ "fit", "Tornedalen Finnish" },
	{ "fj", "Fijian" },
	{ "fo", "Faroese" },
	{ "fon", "Fon" },
	{ "fr", "French" },
	{ "frc", "Cajun French" },
	{ "frm", "Middle French" },
	{ "fro", "Old French" },
	{ "frp", "Arpitan" },
	{ "frr", "Northern Frisian" },
	{ "frs", "Eastern Frisian" },
	{ "fur", "Friulian" },
	{ "fy", "Western Frisian" },
	{ "ga", "Irish" },
	{ "gaa", "Ga" },
	{ "gag", "Gagauz" },
	{ "gan", "Gan Chinese" },
	{ "gay", "Gayo" },
	{ "gba", "Gbaya" },
	{ "gbz", "Zoroastrian Dari" },
	{ "gd", "Scottish Gaelic" },
	{ "gez", "Geez" },
	{ "gil", "Gilbertese" },
	{ "gl", "Galician" },
	{ "glk", "Gilaki" },
	{ "gmh", "Middle High German" },
	{ "gn", "Guarani" },
	{ "goh", "Old High German" },
	{ "gom", "Goan Konkani" },
	{ "gon", "Gondi" },
	{ "gor", "Gorontalo" },
	{ "got", "Gothic" },
	{ "grb", "Grebo" },
	{ "grc", "Ancient Greek" },
	{ "gsw", "Swiss German" },
	{ "gu", "Gujarati" },
	{ "guc", "Wayuu" },
	{ "gur", "Frafra" },
	{ "guz", "Gusii" },
	{ "gv", "Manx" },
	{ "gwi", "Gwichʼin" },
	{ "ha", "Hausa" },
	{ "hai", "Haida" },
	{ "hak", "Hakka Chinese" },
	{ "haw", "Hawaiian" },
	{ "he", "Hebrew" },
	{ "hi", "Hindi" },
	{ "hif", "Fiji Hindi" },
	{ "hil", "Hiligaynon" },
	{ "hit", "Hittite" },
	{ "hmn", "Hmong" },
	{ "ho", "Hiri Motu" },
	{ "hne", "Chhattisgarhi" },
	{ "hr", "Croatian" },
	{ "hsb", "Upper Sorbian" },
	{ "hsn", "Xiang Chinese" },
	{ "ht", "Haitian" },
	{ "hu", "Hungarian" },
	{ "hup", "Hupa" },
	{ "hus", "Huastec" },
	{ "hy", "Armenian" },
	{ "hz", "Herero" },
	{ "ia", "Interlingua" },
	{ "iba", "Iban" },
	{ "ibb", "Ibibio" },
	{ "id", "Indonesian" },
	{ "ie", "Interlingue" },
	{ "ig", "Igbo" },
	{ "ii", "Sichuan Yi" },
	{ "ik", "Inupiaq" },
	{ "ilo", "Iloko" },
	{ "inh", "Ingush" },
	{ "io", "Ido" },
	{ "is", "Icelandic" },
	{ "it", "Italian" },
	{ "iu", "Inuktitut" },
	{ "izh", "Ingrian" },
	{ "ja", "Japanese" },
	{ "jam", "Jamaican Creole English" },
	{ "jbo", "Lojban" },
	{ "jgo", "Ngomba" },
	{ "jmc", "Machame" },
	{ "jpr", "Judeo-Persian" },
	{ "jrb", "Judeo-Arabic" },
	{ "jut", "Jutish" },
	{ "jv", "Javanese" },
	{ "ka", "Georgian" },
	{ "kaa", "Kara-Kalpak" },
	{ "kab", "Kabyle" },
	{ "kac", "Kachin" },
	{ "kaj", "Jju" },
	{ "kam", "Kamba" },
	{ "kaw", "Kawi" },
	{ "kbd", "Kabardian" },
	{ "kbl", "Kanembu" },
	{ "kcg", "Tyap" },
	{ "kde", "Makonde" },
	{ "kea", "Kabuverdianu" },
	{ "ken", "Kenyang" },
	{ "kfo", "Koro" },
	{ "kg", "Kongo" },
	{ "kgp", "Kaingang" },
	{ "kha", "Khasi" },
	{ "kho", "Khotanese" },
	{ "khq", "Koyra Chiini" },
	{ "khw", "Khowar" },
	{ "ki", "Kikuyu" },
	{ "kiu", "Kirmanjki" },
	{ "kj", "Kuanyama" },
	{ "kk", "Kazakh" },
	{ "kkj", "Kako" },
	{ "kl", "Kalaallisut" },
	{ "kln", "Kalenjin" },
	{ "km", "Central Khmer" },
	{ "kmb", "Kimbundu" },
	{ "kn", "Kannada" },
	{ "ko", "Korean" },
	{ "koi", "Komi-Permyak" },
	{ "kok", "Konkani" },
	{ "kos", "Kosraean" },
	{ "kpe", "Kpelle" },
	{ "kr", "Kanuri" },
	{ "krc", "Karachay-Balkar" },
	{ "kri", "Krio" },
	{ "krj", "Kinaray-a" },
	{ "krl", "Karelian" },
	{ "kru", "Kurukh" },
	{ "ks", "Kashmiri" },
	{ "ksb", "Shambala" },
	{ "ksf", "Bafia" },
	{ "ksh", "Colognian" },
	{ "ku", "Kurdish" },
	{ "kum", "Kumyk" },
	{ "kut", "Kutenai" },
	{ "kv", "Komi" },
	{ "kw", "Cornish" },
	{ "ky", "Kirghiz" },
	{ "lag", "Langi" },
	{ "la", "Latin" },
	{ "lad", "Ladino" },
	{ "lag", "Langi" },
	{ "lah", "Lahnda" },
	{ "lam", "Lamba" },
	{ "lb", "Luxembourgish" },
	{ "lez", "Lezghian" },
	{ "lfn", "Lingua Franca Nova" },
	{ "lg", "Ganda" },
	{ "li", "Limburgan" },
	{ "lij", "Ligurian" },
	{ "liv", "Livonian" },
	{ "lkt", "Lakota" },
	{ "lmo", "Lombard" },
	{ "ln", "Lingala" },
	{ "lo", "Lao" },
	{ "lol", "Mongo" },
	{ "lou", "Louisiana Creole" },
	{ "loz", "Lozi" },
	{ "lrc", "Northern Luri" },
	{ "lt", "Lithuanian" },
	{ "ltg", "Latgalian" },
	{ "lu", "Luba-Katanga" },
	{ "lua", "Luba-Lulua" },
	{ "lui", "Luiseno" },
	{ "lun", "Lunda" },
	{ "luo", "Luo" },
	{ "lus", "Mizo" },
	{ "luy", "Luyia" },
	{ "lv", "Latvian" },
	{ "lzh", "Literary Chinese" },
	{ "lzz", "Laz" },
	{ "mad", "Madurese" },
	{ "maf", "Mafa" },
	{ "mag", "Magahi" },
	{ "mai", "Maithili" },
	{ "mak", "Makasar" },
	{ "man", "Mandingo" },
	{ "mas", "Masai" },
	{ "mde", "Maba" },
	{ "mdf", "Moksha" },
	{ "mdr", "Mandar" },
	{ "men", "Mende" },
	{ "mer", "Meru" },
	{ "mfe", "Morisyen" },
	{ "mg", "Malagasy" },
	{ "mga", "Middle Irish" },
	{ "mgh", "Makhuwa-Meetto" },
	{ "mgo", "Metaʼ" },
	{ "mh", "Marshallese" },
	{ "mhr", "Eastern Mari" },
	{ "mi", "Māori" },
	{ "mic", "Mi'kmaq" },
	{ "min", "Minangkabau" },
	{ "miq", "Mískito" },
	{ "mjw", "Karbi" },
	{ "mk", "Macedonian" },
	{ "ml", "Malayalam" },
	{ "mn", "Mongolian" },
	{ "mnc", "Manchu" },
	{ "mni", "Manipuri" },
	{ "mnw", "Mon" },
	{ "mos", "Mossi" },
	{ "moh", "Mohawk" },
	{ "mr", "Marathi" },
	{ "mrj", "Western Mari" },
	{ "ms", "Malay" },
	{ "mt", "Maltese" },
	{ "mua", "Mundang" },
	{ "mus", "Muscogee" },
	{ "mwl", "Mirandese" },
	{ "mwr", "Marwari" },
	{ "mwv", "Mentawai" },
	{ "my", "Burmese" },
	{ "mye", "Myene" },
	{ "myv", "Erzya" },
	{ "mzn", "Mazanderani" },
	{ "na", "Nauru" },
	{ "nah", "Nahuatl" },
	{ "nan", "Min Nan Chinese" },
	{ "nap", "Neapolitan" },
	{ "naq", "Nama" },
	{ "nan", "Min Nan Chinese" },
	{ "nb", "Norwegian Bokmål" },
	{ "nd", "North Ndebele" },
	{ "nds", "Low German" },
	{ "ne", "Nepali" },
	{ "new", "Newari" },
	{ "nhn", "Central Nahuatl" },
	{ "ng", "Ndonga" },
	{ "nia", "Nias" },
	{ "niu", "Niuean" },
	{ "njo", "Ao Naga" },
	{ "nl", "Dutch" },
	{ "nmg", "Kwasio" },
	{ "nn", "Norwegian Nynorsk" },
	{ "nnh", "Ngiemboon" },
	{ "nog", "Nogai" },
	{ "non", "Old Norse" },
	{ "nov", "Novial" },
	{ "nqo", "N'ko" },
	{ "nr", "South Ndebele" },
	{ "nso", "Pedi" },
	{ "nus", "Nuer" },
	{ "nv", "Navajo" },
	{ "nwc", "Classical Newari" },
	{ "ny", "Nyanja" },
	{ "nym", "Nyamwezi" },
	{ "nyn", "Nyankole" },
	{ "nyo", "Nyoro" },
	{ "nzi", "Nzima" },
	{ "oc", "Occitan" },
	{ "oj", "Ojibwa" },
	{ "om", "Oromo" },
	{ "or", "Odia" },
	{ "os", "Ossetic" },
	{ "osa", "Osage" },
	{ "ota", "Ottoman Turkish" },
	{ "pa", "Panjabi" },
	{ "pag", "Pangasinan" },
	{ "pal", "Pahlavi" },
	{ "pam", "Pampanga" },
	{ "pap", "Papiamento" },
	{ "pau", "Palauan" },
	{ "pcd", "Picard" },
	{ "pcm", "Nigerian Pidgin" },
	{ "pdc", "Pennsylvania German" },
	{ "pdt", "Plautdietsch" },
	{ "peo", "Old Persian" },
	{ "pfl", "Palatine German" },
	{ "phn", "Phoenician" },
	{ "pi", "Pali" },
	{ "pl", "Polish" },
	{ "pms", "Piedmontese" },
	{ "pnt", "Pontic" },
	{ "pon", "Pohnpeian" },
	{ "pr", "Pirate" },
	{ "prg", "Prussian" },
	{ "pro", "Old Provençal" },
	{ "prs", "Dari" },
	{ "ps", "Pushto" },
	{ "pt", "Portuguese" },
	{ "qu", "Quechua" },
	{ "quc", "K'iche" },
	{ "qug", "Chimborazo Highland Quichua" },
	{ "quy", "Ayacucho Quechua" },
	{ "quz", "Cusco Quechua" },
	{ "raj", "Rajasthani" },
	{ "rap", "Rapanui" },
	{ "rar", "Rarotongan" },
	{ "rgn", "Romagnol" },
	{ "rif", "Riffian" },
	{ "rm", "Romansh" },
	{ "rn", "Rundi" },
	{ "ro", "Romanian" },
	{ "rof", "Rombo" },
	{ "rom", "Romany" },
	{ "rtm", "Rotuman" },
	{ "ru", "Russian" },
	{ "rue", "Rusyn" },
	{ "rug", "Roviana" },
	{ "rup", "Aromanian" },
	{ "rw", "Kinyarwanda" },
	{ "rwk", "Rwa" },
	{ "sa", "Sanskrit" },
	{ "sad", "Sandawe" },
	{ "sah", "Sakha" },
	{ "sam", "Samaritan Aramaic" },
	{ "saq", "Samburu" },
	{ "sas", "Sasak" },
	{ "sat", "Santali" },
	{ "saz", "Saurashtra" },
	{ "sba", "Ngambay" },
	{ "sbp", "Sangu" },
	{ "sc", "Sardinian" },
	{ "scn", "Sicilian" },
	{ "sco", "Scots" },
	{ "sd", "Sindhi" },
	{ "sdc", "Sassarese Sardinian" },
	{ "sdh", "Southern Kurdish" },
	{ "se", "Northern Sami" },
	{ "see", "Seneca" },
	{ "seh", "Sena" },
	{ "sei", "Seri" },
	{ "sel", "Selkup" },
	{ "ses", "Koyraboro Senni" },
	{ "sg", "Sango" },
	{ "sga", "Old Irish" },
	{ "sgs", "Samogitian" },
	{ "sh", "Serbo-Croatian" },
	{ "shi", "Tachelhit" },
	{ "shn", "Shan" },
	{ "shs", "Shuswap" },
	{ "shu", "Chadian Arabic" },
	{ "si", "Sinhala" },
	{ "sid", "Sidamo" },
	{ "sk", "Slovak" },
	{ "sl", "Slovenian" },
	{ "sli", "Lower Silesian" },
	{ "sly", "Selayar" },
	{ "sm", "Samoan" },
	{ "sma", "Southern Sami" },
	{ "smj", "Lule Sami" },
	{ "smn", "Inari Sami" },
	{ "sms", "Skolt Sami" },
	{ "sn", "Shona" },
	{ "snk", "Soninke" },
	{ "so", "Somali" },
	{ "sog", "Sogdien" },
	{ "son", "Songhai" },
	{ "sq", "Albanian" },
	{ "sr", "Serbian" },
	{ "srn", "Sranan Tongo" },
	{ "srr", "Serer" },
	{ "ss", "Swati" },
	{ "ssy", "Saho" },
	{ "st", "Southern Sotho" },
	{ "stq", "Saterland Frisian" },
	{ "su", "Sundanese" },
	{ "suk", "Sukuma" },
	{ "sus", "Susu" },
	{ "sux", "Sumerian" },
	{ "sv", "Swedish" },
	{ "sw", "Swahili" },
	{ "swb", "Comorian" },
	{ "swc", "Congo Swahili" },
	{ "syc", "Classical Syriac" },
	{ "syr", "Syriac" },
	{ "szl", "Silesian" },
	{ "ta", "Tamil" },
	{ "tcy", "Tulu" },
	{ "te", "Telugu" },
	{ "tem", "Timne" },
	{ "teo", "Teso" },
	{ "ter", "Tereno" },
	{ "tet", "Tetum" },
	{ "tg", "Tajik" },
	{ "th", "Thai" },
	{ "the", "Chitwania Tharu" },
	{ "ti", "Tigrinya" },
	{ "tig", "Tigre" },
	{ "tiv", "Tiv" },
	{ "tk", "Turkmen" },
	{ "tkl", "Tokelau" },
	{ "tkr", "Tsakhur" },
	{ "tl", "Tagalog" },
	{ "tlh", "Klingon" },
	{ "tli", "Tlingit" },
	{ "tly", "Talysh" },
	{ "tmh", "Tamashek" },
	{ "tn", "Tswana" },
	{ "to", "Tongan" },
	{ "tog", "Nyasa Tonga" },
	{ "tpi", "Tok Pisin" },
	{ "tr", "Turkish" },
	{ "tru", "Turoyo" },
	{ "trv", "Taroko" },
	{ "ts", "Tsonga" },
	{ "tsd", "Tsakonian" },
	{ "tsi", "Tsimshian" },
	{ "tt", "Tatar" },
	{ "ttt", "Muslim Tat" },
	{ "tum", "Tumbuka" },
	{ "tvl", "Tuvalu" },
	{ "tw", "Twi" },
	{ "twq", "Tasawaq" },
	{ "ty", "Tahitian" },
	{ "tyv", "Tuvinian" },
	{ "tzm", "Central Atlas Tamazight" },
	{ "udm", "Udmurt" },
	{ "ug", "Uyghur" },
	{ "uga", "Ugaritic" },
	{ "uk", "Ukrainian" },
	{ "umb", "Umbundu" },
	{ "unm", "Unami" },
	{ "ur", "Urdu" },
	{ "uz", "Uzbek" },
	{ "vai", "Vai" },
	{ "ve", "Venda" },
	{ "vec", "Venetian" },
	{ "vep", "Veps" },
	{ "vi", "Vietnamese" },
	{ "vls", "West Flemish" },
	{ "vmf", "Main-Franconian" },
	{ "vo", "Volapük" },
	{ "vot", "Votic" },
	{ "vro", "Võro" },
	{ "vun", "Vunjo" },
	{ "wa", "Walloon" },
	{ "wae", "Walser" },
	{ "wal", "Wolaytta" },
	{ "war", "Waray" },
	{ "was", "Washo" },
	{ "wbp", "Warlpiri" },
	{ "wo", "Wolof" },
	{ "wuu", "Wu Chinese" },
	{ "xal", "Kalmyk" },
	{ "xh", "Xhosa" },
	{ "xmf", "Mingrelian" },
	{ "xog", "Soga" },
	{ "yao", "Yao" },
	{ "yap", "Yapese" },
	{ "yav", "Yangben" },
	{ "ybb", "Yemba" },
	{ "yi", "Yiddish" },
	{ "yo", "Yoruba" },
	{ "yrl", "Nheengatu" },
	{ "yue", "Yue Chinese" },
	{ "yuw", "Papua New Guinea" },
	{ "za", "Zhuang" },
	{ "zap", "Zapotec" },
	{ "zbl", "Blissymbols" },
	{ "zea", "Zeelandic" },
	{ "zen", "Zenaga" },
	{ "zgh", "Standard Moroccan Tamazight" },
	{ "zh", "Chinese" },
	{ "zu", "Zulu" },
	{ "zun", "Zuni" },
	{ "zza", "Zaza" },
	{ nullptr, nullptr }
};

// Additional regional variants.
// Variant name, supported languages.

static const char *locale_variants[][2] = {
	{ "valencia", "ca" },
	{ "iqtelif", "tt" },
	{ "saaho", "aa" },
	{ "tradnl", "es" },
	{ nullptr, nullptr },
};

// Script names and codes (excludes typographic variants, special codes, reserved codes and aliases for combined scripts).
// Reference:
// - https://en.wikipedia.org/wiki/ISO_15924

static const char *script_list[][2] = {
	{ "Adlam", "Adlm" },
	{ "Afaka", "Afak" },
	{ "Caucasian Albanian", "Aghb" },
	{ "Ahom", "Ahom" },
	{ "Arabic", "Arab" },
	{ "Imperial Aramaic", "Armi" },
	{ "Armenian", "Armn" },
	{ "Avestan", "Avst" },
	{ "Balinese", "Bali" },
	{ "Bamum", "Bamu" },
	{ "Bassa Vah", "Bass" },
	{ "Batak", "Batk" },
	{ "Bengali", "Beng" },
	{ "Bhaiksuki", "Bhks" },
	{ "Blissymbols", "Blis" },
	{ "Bopomofo", "Bopo" },
	{ "Brahmi", "Brah" },
	{ "Braille", "Brai" },
	{ "Buginese", "Bugi" },
	{ "Buhid", "Buhd" },
	{ "Chakma", "Cakm" },
	{ "Unified Canadian Aboriginal", "Cans" },
	{ "Carian", "Cari" },
	{ "Cham", "Cham" },
	{ "Cherokee", "Cher" },
	{ "Chorasmian", "Chrs" },
	{ "Cirth", "Cirt" },
	{ "Coptic", "Copt" },
	{ "Cypro-Minoan", "Cpmn" },
	{ "Cypriot", "Cprt" },
	{ "Cyrillic", "Cyrl" },
	{ "Devanagari", "Deva" },
	{ "Dives Akuru", "Diak" },
	{ "Dogra", "Dogr" },
	{ "Deseret", "Dsrt" },
	{ "Duployan", "Dupl" },
	{ "Egyptian demotic", "Egyd" },
	{ "Egyptian hieratic", "Egyh" },
	{ "Egyptian hieroglyphs", "Egyp" },
	{ "Elbasan", "Elba" },
	{ "Elymaic", "Elym" },
	{ "Ethiopic", "Ethi" },
	{ "Khutsuri", "Geok" },
	{ "Georgian", "Geor" },
	{ "Glagolitic", "Glag" },
	{ "Gunjala Gondi", "Gong" },
	{ "Masaram Gondi", "Gonm" },
	{ "Gothic", "Goth" },
	{ "Grantha", "Gran" },
	{ "Greek", "Grek" },
	{ "Gujarati", "Gujr" },
	{ "Gurmukhi", "Guru" },
	{ "Hangul", "Hang" },
	{ "Han", "Hani" },
	{ "Hanunoo", "Hano" },
	{ "Simplified", "Hans" },
	{ "Traditional", "Hant" },
	{ "Hatran", "Hatr" },
	{ "Hebrew", "Hebr" },
	{ "Hiragana", "Hira" },
	{ "Anatolian Hieroglyphs", "Hluw" },
	{ "Pahawh Hmong", "Hmng" },
	{ "Nyiakeng Puachue Hmong", "Hmnp" },
	{ "Old Hungarian", "Hung" },
	{ "Indus", "Inds" },
	{ "Old Italic", "Ital" },
	{ "Javanese", "Java" },
	{ "Jurchen", "Jurc" },
	{ "Kayah Li", "Kali" },
	{ "Katakana", "Kana" },
	{ "Kharoshthi", "Khar" },
	{ "Khmer", "Khmr" },
	{ "Khojki", "Khoj" },
	{ "Khitan large script", "Kitl" },
	{ "Khitan small script", "Kits" },
	{ "Kannada", "Knda" },
	{ "Kpelle", "Kpel" },
	{ "Kaithi", "Kthi" },
	{ "Tai Tham", "Lana" },
	{ "Lao", "Laoo" },
	{ "Latin", "Latn" },
	{ "Leke", "Leke" },
	{ "Lepcha", "Lepc" },
	{ "Limbu", "Limb" },
	{ "Linear A", "Lina" },
	{ "Linear B", "Linb" },
	{ "Lisu", "Lisu" },
	{ "Loma", "Loma" },
	{ "Lycian", "Lyci" },
	{ "Lydian", "Lydi" },
	{ "Mahajani", "Mahj" },
	{ "Makasar", "Maka" },
	{ "Mandaic", "Mand" },
	{ "Manichaean", "Mani" },
	{ "Marchen", "Marc" },
	{ "Mayan Hieroglyphs", "Maya" },
	{ "Medefaidrin", "Medf" },
	{ "Mende Kikakui", "Mend" },
	{ "Meroitic Cursive", "Merc" },
	{ "Meroitic Hieroglyphs", "Mero" },
	{ "Malayalam", "Mlym" },
	{ "Modi", "Modi" },
	{ "Mongolian", "Mong" },
	{ "Moon", "Moon" },
	{ "Mro", "Mroo" },
	{ "Meitei Mayek", "Mtei" },
	{ "Multani", "Mult" },
	{ "Myanmar (Burmese)", "Mymr" },
	{ "Nandinagari", "Nand" },
	{ "Old North Arabian", "Narb" },
	{ "Nabataean", "Nbat" },
	{ "Newa", "Newa" },
	{ "Naxi Dongba", "Nkdb" },
	{ "Nakhi Geba", "Nkgb" },
	{ "N'ko", "Nkoo" },
	{ "Nüshu", "Nshu" },
	{ "Ogham", "Ogam" },
	{ "Ol Chiki", "Olck" },
	{ "Old Turkic", "Orkh" },
	{ "Oriya", "Orya" },
	{ "Osage", "Osge" },
	{ "Osmanya", "Osma" },
	{ "Old Uyghur", "Ougr" },
	{ "Palmyrene", "Palm" },
	{ "Pau Cin Hau", "Pauc" },
	{ "Proto-Cuneiform", "Pcun" },
	{ "Proto-Elamite", "Pelm" },
	{ "Old Permic", "Perm" },
	{ "Phags-pa", "Phag" },
	{ "Inscriptional Pahlavi", "Phli" },
	{ "Psalter Pahlavi", "Phlp" },
	{ "Book Pahlavi", "Phlv" },
	{ "Phoenician", "Phnx" },
	{ "Klingon", "Piqd" },
	{ "Miao", "Plrd" },
	{ "Inscriptional Parthian", "Prti" },
	{ "Proto-Sinaitic", "Psin" },
	{ "Ranjana", "Ranj" },
	{ "Rejang", "Rjng" },
	{ "Hanifi Rohingya", "Rohg" },
	{ "Rongorongo", "Roro" },
	{ "Runic", "Runr" },
	{ "Samaritan", "Samr" },
	{ "Sarati", "Sara" },
	{ "Old South Arabian", "Sarb" },
	{ "Saurashtra", "Saur" },
	{ "SignWriting", "Sgnw" },
	{ "Shavian", "Shaw" },
	{ "Sharada", "Shrd" },
	{ "Shuishu", "Shui" },
	{ "Siddham", "Sidd" },
	{ "Khudawadi", "Sind" },
	{ "Sinhala", "Sinh" },
	{ "Sogdian", "Sogd" },
	{ "Old Sogdian", "Sogo" },
	{ "Sora Sompeng", "Sora" },
	{ "Soyombo", "Soyo" },
	{ "Sundanese", "Sund" },
	{ "Syloti Nagri", "Sylo" },
	{ "Syriac", "Syrc" },
	{ "Tagbanwa", "Tagb" },
	{ "Takri", "Takr" },
	{ "Tai Le", "Tale" },
	{ "New Tai Lue", "Talu" },
	{ "Tamil", "Taml" },
	{ "Tangut", "Tang" },
	{ "Tai Viet", "Tavt" },
	{ "Telugu", "Telu" },
	{ "Tengwar", "Teng" },
	{ "Tifinagh", "Tfng" },
	{ "Tagalog", "Tglg" },
	{ "Thaana", "Thaa" },
	{ "Thai", "Thai" },
	{ "Tibetan", "Tibt" },
	{ "Tirhuta", "Tirh" },
	{ "Tangsa", "Tnsa" },
	{ "Toto", "Toto" },
	{ "Ugaritic", "Ugar" },
	{ "Vai", "Vaii" },
	{ "Visible Speech", "Visp" },
	{ "Vithkuqi", "Vith" },
	{ "Warang Citi", "Wara" },
	{ "Wancho", "Wcho" },
	{ "Woleai", "Wole" },
	{ "Old Persian", "Xpeo" },
	{ "Cuneiform", "Xsux" },
	{ "Yezidi", "Yezi" },
	{ "Yi", "Yiii" },
	{ "Zanabazar Square", "Zanb" },
	{ nullptr, nullptr }
};

#endif // LOCALES_H
