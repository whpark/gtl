//module;
//
//#include "gtl/gtl.h"
//#include "gtl.ui.h"
//
//export module gtl.ui:prop.value.color;
//import :predefine;
//
//using namespace std::literals;
//using namespace gtl::literals;
//using namespace gtl::ui::unit::literals;

#pragma once

#include "ui_predefine.h"

namespace gtl::ui::prop::value::color {

	static inline constexpr string_t const aliceBlue				= GText("AliceBlue"s);						//	F0F8FF	240,248,255
	static inline constexpr string_t const antiqueWhite				= GText("AntiqueWhite"s);					//	FAEBD7	250,235,215
	static inline constexpr string_t const aqua						= GText("Aqua"s);							//	00FFFF	0,255,255
	static inline constexpr string_t const aquamarine				= GText("Aquamarine"s);						//	7FFFD4	127,255,212
	static inline constexpr string_t const azure					= GText("Azure"s);							//	F0FFFF	240,255,255
	static inline constexpr string_t const beige					= GText("Beige"s);							//	F5F5DC	245,245,220
	static inline constexpr string_t const bisque					= GText("Bisque"s);							//	FFE4C4	255,228,196
	static inline constexpr string_t const black					= GText("Black"s);							//	000000	0,0,0
	static inline constexpr string_t const blanchedAlmond			= GText("BlanchedAlmond"s);					//	FFEBCD	255,235,205
	static inline constexpr string_t const blue						= GText("Blue"s);							//	0000FF	0,0,255
	static inline constexpr string_t const blueViolet				= GText("BlueViolet"s);						//	8A2BE2	138,43,226
	static inline constexpr string_t const brown					= GText("Brown"s);							//	A52A2A	165,42,42
	static inline constexpr string_t const burlyWood				= GText("BurlyWood"s);						//	DEB887	222,184,135
	static inline constexpr string_t const cadetBlue				= GText("CadetBlue"s);						//	5F9EA0	95,158,160
	static inline constexpr string_t const chartreuse				= GText("Chartreuse"s);						//	7FFF00	127,255,0
	static inline constexpr string_t const chocolate				= GText("Chocolate"s);						//	D2691E	210,105,30
	static inline constexpr string_t const coral					= GText("Coral"s);							//	FF7F50	255,127,80
	static inline constexpr string_t const cornflowerBlue			= GText("CornflowerBlue"s);					//	6495ED	100,149,237
	static inline constexpr string_t const cornsilk					= GText("Cornsilk"s);						//	FFF8DC	255,248,220
	static inline constexpr string_t const crimson					= GText("Crimson"s);						//	DC143C	220,20,60
	static inline constexpr string_t const cyan						= GText("Cyan"s);							//	00FFFF	0,255,255
	static inline constexpr string_t const darkBlue					= GText("DarkBlue"s);						//	00008B	0,0,139
	static inline constexpr string_t const darkCyan					= GText("DarkCyan"s);						//	008B8B	0,139,139
	static inline constexpr string_t const darkGoldenRod			= GText("DarkGoldenRod"s);					//	B8860B	184,134,11
	static inline constexpr string_t const darkGray					= GText("DarkGray"s);						//	A9A9A9	169,169,169
	static inline constexpr string_t const darkGrey					= GText("DarkGrey"s);						//	A9A9A9	169,169,169
	static inline constexpr string_t const darkGreen				= GText("DarkGreen"s);						//	006400	0,100,0
	static inline constexpr string_t const darkKhaki				= GText("DarkKhaki"s);						//	BDB76B	189,183,107
	static inline constexpr string_t const darkMagenta				= GText("DarkMagenta"s);					//	8B008B	139,0,139
	static inline constexpr string_t const darkOliveGreen			= GText("DarkOliveGreen"s);					//	556B2F	85,107,47
	static inline constexpr string_t const darkOrange				= GText("DarkOrange"s);						//	FF8C00	255,140,0
	static inline constexpr string_t const darkOrchid				= GText("DarkOrchid"s);						//	9932CC	153,50,204
	static inline constexpr string_t const darkRed					= GText("DarkRed"s);						//	8B0000	139,0,0
	static inline constexpr string_t const darkSalmon				= GText("DarkSalmon"s);						//	E9967A	233,150,122
	static inline constexpr string_t const darkSeaGreen				= GText("DarkSeaGreen"s);					//	8FBC8F	143,188,143
	static inline constexpr string_t const darkSlateBlue			= GText("DarkSlateBlue"s);					//	483D8B	72,61,139
	static inline constexpr string_t const darkSlateGray			= GText("DarkSlateGray"s);					//	2F4F4F	47,79,79
	static inline constexpr string_t const darkSlateGrey			= GText("DarkSlateGrey"s);					//	2F4F4F	47,79,79
	static inline constexpr string_t const darkTurquoise			= GText("DarkTurquoise"s);					//	00CED1	0,206,209
	static inline constexpr string_t const darkViolet				= GText("DarkViolet"s);						//	9400D3	148,0,211
	static inline constexpr string_t const deepPink					= GText("DeepPink"s);						//	FF1493	255,20,147
	static inline constexpr string_t const deepSkyBlue				= GText("DeepSkyBlue"s);					//	00BFFF	0,191,255
	static inline constexpr string_t const dimGray					= GText("DimGray"s);						//	696969	105,105,105
	static inline constexpr string_t const dimGrey					= GText("DimGrey"s);						//	696969	105,105,105
	static inline constexpr string_t const dodgerBlue				= GText("DodgerBlue"s);						//	1E90FF	30,144,255
	static inline constexpr string_t const fireBrick				= GText("FireBrick"s);						//	B22222	178,34,34
	static inline constexpr string_t const floralWhite				= GText("FloralWhite"s);					//	FFFAF0	255,250,240
	static inline constexpr string_t const forestGreen				= GText("ForestGreen"s);					//	228B22	34,139,34
	static inline constexpr string_t const fuchsia					= GText("Fuchsia"s);						//	FF00FF	255,0,255
	static inline constexpr string_t const gainsboro				= GText("Gainsboro"s);						//	DCDCDC	220,220,220
	static inline constexpr string_t const ghostWhite				= GText("GhostWhite"s);						//	F8F8FF	248,248,255
	static inline constexpr string_t const gold						= GText("Gold"s);							//	FFD700	255,215,0
	static inline constexpr string_t const goldenRod				= GText("GoldenRod"s);						//	DAA520	218,165,32
	static inline constexpr string_t const gray						= GText("Gray"s);							//	808080	128,128,128
	static inline constexpr string_t const grey						= GText("Grey"s);							//	808080	128,128,128
	static inline constexpr string_t const green					= GText("Green"s);							//	008000	0,128,0
	static inline constexpr string_t const greenYellow				= GText("GreenYellow"s);					//	ADFF2F	173,255,47
	static inline constexpr string_t const honeyDew					= GText("HoneyDew"s);						//	F0FFF0	240,255,240
	static inline constexpr string_t const hotPink					= GText("HotPink"s);						//	FF69B4	255,105,180
	static inline constexpr string_t const indianRed 				= GText("IndianRed "s);						//	CD5C5C	205,92,92
	static inline constexpr string_t const indigo 					= GText("Indigo "s);						//	4B0082	75,0,130
	static inline constexpr string_t const ivory					= GText("Ivory"s);							//	FFFFF0	255,255,240
	static inline constexpr string_t const khaki					= GText("Khaki"s);							//	F0E68C	240,230,140
	static inline constexpr string_t const lavender					= GText("Lavender"s);						//	E6E6FA	230,230,250
	static inline constexpr string_t const lavenderBlush			= GText("LavenderBlush"s);					//	FFF0F5	255,240,245
	static inline constexpr string_t const lawnGreen				= GText("LawnGreen"s);						//	7CFC00	124,252,0
	static inline constexpr string_t const lemonChiffon				= GText("LemonChiffon"s);					//	FFFACD	255,250,205
	static inline constexpr string_t const lightBlue				= GText("LightBlue"s);						//	ADD8E6	173,216,230
	static inline constexpr string_t const lightCoral				= GText("LightCoral"s);						//	F08080	240,128,128
	static inline constexpr string_t const lightCyan				= GText("LightCyan"s);						//	E0FFFF	224,255,255
	static inline constexpr string_t const lightGoldenRodYellow		= GText("LightGoldenRodYellow"s);			//	FAFAD2	250,250,210
	static inline constexpr string_t const lightGray				= GText("LightGray"s);						//	D3D3D3	211,211,211
	static inline constexpr string_t const lightGrey				= GText("LightGrey"s);						//	D3D3D3	211,211,211
	static inline constexpr string_t const lightGreen				= GText("LightGreen"s);						//	90EE90	144,238,144
	static inline constexpr string_t const lightPink				= GText("LightPink"s);						//	FFB6C1	255,182,193
	static inline constexpr string_t const lightSalmon				= GText("LightSalmon"s);					//	FFA07A	255,160,122
	static inline constexpr string_t const lightSeaGreen			= GText("LightSeaGreen"s);					//	20B2AA	32,178,170
	static inline constexpr string_t const lightSkyBlue				= GText("LightSkyBlue"s);					//	87CEFA	135,206,250
	static inline constexpr string_t const lightSlateGray			= GText("LightSlateGray"s);					//	778899	119,136,153
	static inline constexpr string_t const lightSlateGrey			= GText("LightSlateGrey"s);					//	778899	119,136,153
	static inline constexpr string_t const lightSteelBlue			= GText("LightSteelBlue"s);					//	B0C4DE	176,196,222
	static inline constexpr string_t const lightYellow				= GText("LightYellow"s);					//	FFFFE0	255,255,224
	static inline constexpr string_t const lime						= GText("Lime"s);							//	00FF00	0,255,0
	static inline constexpr string_t const limeGreen				= GText("LimeGreen"s);						//	32CD32	50,205,50
	static inline constexpr string_t const linen					= GText("Linen"s);							//	FAF0E6	250,240,230
	static inline constexpr string_t const magenta					= GText("Magenta"s);						//	FF00FF	255,0,255
	static inline constexpr string_t const maroon					= GText("Maroon"s);							//	800000	128,0,0
	static inline constexpr string_t const mediumAquaMarine			= GText("MediumAquaMarine"s);				//	66CDAA	102,205,170
	static inline constexpr string_t const mediumBlue				= GText("MediumBlue"s);						//	0000CD	0,0,205
	static inline constexpr string_t const mediumOrchid				= GText("MediumOrchid"s);					//	BA55D3	186,85,211
	static inline constexpr string_t const mediumPurple				= GText("MediumPurple"s);					//	9370DB	147,112,219
	static inline constexpr string_t const mediumSeaGreen			= GText("MediumSeaGreen"s);					//	3CB371	60,179,113
	static inline constexpr string_t const mediumSlateBlue			= GText("MediumSlateBlue"s);				//	7B68EE	123,104,238
	static inline constexpr string_t const mediumSpringGreen		= GText("MediumSpringGreen"s);				//	00FA9A	0,250,154
	static inline constexpr string_t const mediumTurquoise			= GText("MediumTurquoise"s);				//	48D1CC	72,209,204
	static inline constexpr string_t const mediumVioletRed			= GText("MediumVioletRed"s);				//	C71585	199,21,133
	static inline constexpr string_t const midnightBlue				= GText("MidnightBlue"s);					//	191970	25,25,112
	static inline constexpr string_t const mintCream				= GText("MintCream"s);						//	F5FFFA	245,255,250
	static inline constexpr string_t const mistyRose				= GText("MistyRose"s);						//	FFE4E1	255,228,225
	static inline constexpr string_t const moccasin					= GText("Moccasin"s);						//	FFE4B5	255,228,181
	static inline constexpr string_t const navajoWhite				= GText("NavajoWhite"s);					//	FFDEAD	255,222,173
	static inline constexpr string_t const navy						= GText("Navy"s);							//	000080	0,0,128
	static inline constexpr string_t const oldLace					= GText("OldLace"s);						//	FDF5E6	253,245,230
	static inline constexpr string_t const olive					= GText("Olive"s);							//	808000	128,128,0
	static inline constexpr string_t const oliveDrab				= GText("OliveDrab"s);						//	6B8E23	107,142,35
	static inline constexpr string_t const orange					= GText("Orange"s);							//	FFA500	255,165,0
	static inline constexpr string_t const orangeRed				= GText("OrangeRed"s);						//	FF4500	255,69,0
	static inline constexpr string_t const orchid					= GText("Orchid"s);							//	DA70D6	218,112,214
	static inline constexpr string_t const paleGoldenRod			= GText("PaleGoldenRod"s);					//	EEE8AA	238,232,170
	static inline constexpr string_t const paleGreen				= GText("PaleGreen"s);						//	98FB98	152,251,152
	static inline constexpr string_t const paleTurquoise			= GText("PaleTurquoise"s);					//	AFEEEE	175,238,238
	static inline constexpr string_t const paleVioletRed			= GText("PaleVioletRed"s);					//	DB7093	219,112,147
	static inline constexpr string_t const papayaWhip				= GText("PapayaWhip"s);						//	FFEFD5	255,239,213
	static inline constexpr string_t const peachPuff				= GText("PeachPuff"s);						//	FFDAB9	255,218,185
	static inline constexpr string_t const peru						= GText("Peru"s);							//	CD853F	205,133,63
	static inline constexpr string_t const pink						= GText("Pink"s);							//	FFC0CB	255,192,203
	static inline constexpr string_t const plum						= GText("Plum"s);							//	DDA0DD	221,160,221
	static inline constexpr string_t const powderBlue				= GText("PowderBlue"s);						//	B0E0E6	176,224,230
	static inline constexpr string_t const purple					= GText("Purple"s);							//	800080	128,0,128
	static inline constexpr string_t const rebeccaPurple			= GText("RebeccaPurple"s);					//	663399	102,51,153
	static inline constexpr string_t const red						= GText("Red"s);							//	FF0000	255,0,0
	static inline constexpr string_t const rosyBrown				= GText("RosyBrown"s);						//	BC8F8F	188,143,143
	static inline constexpr string_t const royalBlue				= GText("RoyalBlue"s);						//	4169E1	65,105,225
	static inline constexpr string_t const saddleBrown				= GText("SaddleBrown"s);					//	8B4513	139,69,19
	static inline constexpr string_t const salmon					= GText("Salmon"s);							//	FA8072	250,128,114
	static inline constexpr string_t const sandyBrown				= GText("SandyBrown"s);						//	F4A460	244,164,96
	static inline constexpr string_t const seaGreen					= GText("SeaGreen"s);						//	2E8B57	46,139,87
	static inline constexpr string_t const seaShell					= GText("SeaShell"s);						//	FFF5EE	255,245,238
	static inline constexpr string_t const sienna					= GText("Sienna"s);							//	A0522D	160,82,45
	static inline constexpr string_t const silver					= GText("Silver"s);							//	C0C0C0	192,192,192
	static inline constexpr string_t const skyBlue					= GText("SkyBlue"s);						//	87CEEB	135,206,235
	static inline constexpr string_t const slateBlue				= GText("SlateBlue"s);						//	6A5ACD	106,90,205
	static inline constexpr string_t const slateGray				= GText("SlateGray"s);						//	708090	112,128,144
	static inline constexpr string_t const slateGrey				= GText("SlateGrey"s);						//	708090	112,128,144
	static inline constexpr string_t const snow						= GText("Snow"s);							//	FFFAFA	255,250,250
	static inline constexpr string_t const springGreen				= GText("SpringGreen"s);					//	00FF7F	0,255,127
	static inline constexpr string_t const steelBlue				= GText("SteelBlue"s);						//	4682B4	70,130,180
	static inline constexpr string_t const tan						= GText("Tan"s);							//	D2B48C	210,180,140
	static inline constexpr string_t const teal						= GText("Teal"s);							//	008080	0,128,128
	static inline constexpr string_t const thistle					= GText("Thistle"s);						//	D8BFD8	216,191,216
	static inline constexpr string_t const tomato					= GText("Tomato"s);							//	FF6347	255,99,71
	static inline constexpr string_t const turquoise				= GText("Turquoise"s);						//	40E0D0	64,224,208
	static inline constexpr string_t const violet					= GText("Violet"s);							//	EE82EE	238,130,238
	static inline constexpr string_t const wheat					= GText("Wheat"s);							//	F5DEB3	245,222,179
	static inline constexpr string_t const white					= GText("White"s);							//	FFFFFF	255,255,255
	static inline constexpr string_t const whiteSmoke				= GText("WhiteSmoke"s);						//	F5F5F5	245,245,245
	static inline constexpr string_t const yellow					= GText("Yellow"s);							//	FFFF00	255,255,0
	static inline constexpr string_t const yellowGreen				= GText("YellowGreen"s);					//	9ACD32	154,205,50 

};
