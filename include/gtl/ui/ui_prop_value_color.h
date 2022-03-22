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

	static inline string_t const AliceBlue					= GText("AliceBlue"s);						//	F0F8FF	240,248,255
	static inline string_t const AntiqueWhite				= GText("AntiqueWhite"s);					//	FAEBD7	250,235,215
	static inline string_t const Aqua						= GText("Aqua"s);							//	00FFFF	0,255,255
	static inline string_t const Aquamarine					= GText("Aquamarine"s);						//	7FFFD4	127,255,212
	static inline string_t const Azure						= GText("Azure"s);							//	F0FFFF	240,255,255
	static inline string_t const Beige						= GText("Beige"s);							//	F5F5DC	245,245,220
	static inline string_t const Bisque						= GText("Bisque"s);							//	FFE4C4	255,228,196
	static inline string_t const Black						= GText("Black"s);							//	000000	0,0,0
	static inline string_t const BlanchedAlmond				= GText("BlanchedAlmond"s);					//	FFEBCD	255,235,205
	static inline string_t const Blue						= GText("Blue"s);							//	0000FF	0,0,255
	static inline string_t const BlueViolet					= GText("BlueViolet"s);						//	8A2BE2	138,43,226
	static inline string_t const Brown						= GText("Brown"s);							//	A52A2A	165,42,42
	static inline string_t const BurlyWood					= GText("BurlyWood"s);						//	DEB887	222,184,135
	static inline string_t const CadetBlue					= GText("CadetBlue"s);						//	5F9EA0	95,158,160
	static inline string_t const Chartreuse					= GText("Chartreuse"s);						//	7FFF00	127,255,0
	static inline string_t const Chocolate					= GText("Chocolate"s);						//	D2691E	210,105,30
	static inline string_t const Coral						= GText("Coral"s);							//	FF7F50	255,127,80
	static inline string_t const CornflowerBlue				= GText("CornflowerBlue"s);					//	6495ED	100,149,237
	static inline string_t const Cornsilk					= GText("Cornsilk"s);						//	FFF8DC	255,248,220
	static inline string_t const Crimson					= GText("Crimson"s);						//	DC143C	220,20,60
	static inline string_t const Cyan						= GText("Cyan"s);							//	00FFFF	0,255,255
	static inline string_t const DarkBlue					= GText("DarkBlue"s);						//	00008B	0,0,139
	static inline string_t const DarkCyan					= GText("DarkCyan"s);						//	008B8B	0,139,139
	static inline string_t const DarkGoldenRod				= GText("DarkGoldenRod"s);					//	B8860B	184,134,11
	static inline string_t const DarkGray					= GText("DarkGray"s);						//	A9A9A9	169,169,169
	static inline string_t const DarkGrey					= GText("DarkGrey"s);						//	A9A9A9	169,169,169
	static inline string_t const DarkGreen					= GText("DarkGreen"s);						//	006400	0,100,0
	static inline string_t const DarkKhaki					= GText("DarkKhaki"s);						//	BDB76B	189,183,107
	static inline string_t const DarkMagenta				= GText("DarkMagenta"s);					//	8B008B	139,0,139
	static inline string_t const DarkOliveGreen				= GText("DarkOliveGreen"s);					//	556B2F	85,107,47
	static inline string_t const DarkOrange					= GText("DarkOrange"s);						//	FF8C00	255,140,0
	static inline string_t const DarkOrchid					= GText("DarkOrchid"s);						//	9932CC	153,50,204
	static inline string_t const DarkRed					= GText("DarkRed"s);						//	8B0000	139,0,0
	static inline string_t const DarkSalmon					= GText("DarkSalmon"s);						//	E9967A	233,150,122
	static inline string_t const DarkSeaGreen				= GText("DarkSeaGreen"s);					//	8FBC8F	143,188,143
	static inline string_t const DarkSlateBlue				= GText("DarkSlateBlue"s);					//	483D8B	72,61,139
	static inline string_t const DarkSlateGray				= GText("DarkSlateGray"s);					//	2F4F4F	47,79,79
	static inline string_t const DarkSlateGrey				= GText("DarkSlateGrey"s);					//	2F4F4F	47,79,79
	static inline string_t const DarkTurquoise				= GText("DarkTurquoise"s);					//	00CED1	0,206,209
	static inline string_t const DarkViolet					= GText("DarkViolet"s);						//	9400D3	148,0,211
	static inline string_t const DeepPink					= GText("DeepPink"s);						//	FF1493	255,20,147
	static inline string_t const DeepSkyBlue				= GText("DeepSkyBlue"s);					//	00BFFF	0,191,255
	static inline string_t const DimGray					= GText("DimGray"s);						//	696969	105,105,105
	static inline string_t const DimGrey					= GText("DimGrey"s);						//	696969	105,105,105
	static inline string_t const DodgerBlue					= GText("DodgerBlue"s);						//	1E90FF	30,144,255
	static inline string_t const FireBrick					= GText("FireBrick"s);						//	B22222	178,34,34
	static inline string_t const FloralWhite				= GText("FloralWhite"s);					//	FFFAF0	255,250,240
	static inline string_t const ForestGreen				= GText("ForestGreen"s);					//	228B22	34,139,34
	static inline string_t const Fuchsia					= GText("Fuchsia"s);						//	FF00FF	255,0,255
	static inline string_t const Gainsboro					= GText("Gainsboro"s);						//	DCDCDC	220,220,220
	static inline string_t const GhostWhite					= GText("GhostWhite"s);						//	F8F8FF	248,248,255
	static inline string_t const Gold						= GText("Gold"s);							//	FFD700	255,215,0
	static inline string_t const GoldenRod					= GText("GoldenRod"s);						//	DAA520	218,165,32
	static inline string_t const Gray						= GText("Gray"s);							//	808080	128,128,128
	static inline string_t const Grey						= GText("Grey"s);							//	808080	128,128,128
	static inline string_t const Green						= GText("Green"s);							//	008000	0,128,0
	static inline string_t const GreenYellow				= GText("GreenYellow"s);					//	ADFF2F	173,255,47
	static inline string_t const HoneyDew					= GText("HoneyDew"s);						//	F0FFF0	240,255,240
	static inline string_t const HotPink					= GText("HotPink"s);						//	FF69B4	255,105,180
	static inline string_t const IndianRed 					= GText("IndianRed "s);						//	CD5C5C	205,92,92
	static inline string_t const Indigo 					= GText("Indigo "s);						//	4B0082	75,0,130
	static inline string_t const Ivory						= GText("Ivory"s);							//	FFFFF0	255,255,240
	static inline string_t const Khaki						= GText("Khaki"s);							//	F0E68C	240,230,140
	static inline string_t const Lavender					= GText("Lavender"s);						//	E6E6FA	230,230,250
	static inline string_t const LavenderBlush				= GText("LavenderBlush"s);					//	FFF0F5	255,240,245
	static inline string_t const LawnGreen					= GText("LawnGreen"s);						//	7CFC00	124,252,0
	static inline string_t const LemonChiffon				= GText("LemonChiffon"s);					//	FFFACD	255,250,205
	static inline string_t const LightBlue					= GText("LightBlue"s);						//	ADD8E6	173,216,230
	static inline string_t const LightCoral					= GText("LightCoral"s);						//	F08080	240,128,128
	static inline string_t const LightCyan					= GText("LightCyan"s);						//	E0FFFF	224,255,255
	static inline string_t const LightGoldenRodYellow		= GText("LightGoldenRodYellow"s);			//	FAFAD2	250,250,210
	static inline string_t const LightGray					= GText("LightGray"s);						//	D3D3D3	211,211,211
	static inline string_t const LightGrey					= GText("LightGrey"s);						//	D3D3D3	211,211,211
	static inline string_t const LightGreen					= GText("LightGreen"s);						//	90EE90	144,238,144
	static inline string_t const LightPink					= GText("LightPink"s);						//	FFB6C1	255,182,193
	static inline string_t const LightSalmon				= GText("LightSalmon"s);					//	FFA07A	255,160,122
	static inline string_t const LightSeaGreen				= GText("LightSeaGreen"s);					//	20B2AA	32,178,170
	static inline string_t const LightSkyBlue				= GText("LightSkyBlue"s);					//	87CEFA	135,206,250
	static inline string_t const LightSlateGray				= GText("LightSlateGray"s);					//	778899	119,136,153
	static inline string_t const LightSlateGrey				= GText("LightSlateGrey"s);					//	778899	119,136,153
	static inline string_t const LightSteelBlue				= GText("LightSteelBlue"s);					//	B0C4DE	176,196,222
	static inline string_t const LightYellow				= GText("LightYellow"s);					//	FFFFE0	255,255,224
	static inline string_t const Lime						= GText("Lime"s);							//	00FF00	0,255,0
	static inline string_t const LimeGreen					= GText("LimeGreen"s);						//	32CD32	50,205,50
	static inline string_t const Linen						= GText("Linen"s);							//	FAF0E6	250,240,230
	static inline string_t const Magenta					= GText("Magenta"s);						//	FF00FF	255,0,255
	static inline string_t const Maroon						= GText("Maroon"s);							//	800000	128,0,0
	static inline string_t const MediumAquaMarine			= GText("MediumAquaMarine"s);				//	66CDAA	102,205,170
	static inline string_t const MediumBlue					= GText("MediumBlue"s);						//	0000CD	0,0,205
	static inline string_t const MediumOrchid				= GText("MediumOrchid"s);					//	BA55D3	186,85,211
	static inline string_t const MediumPurple				= GText("MediumPurple"s);					//	9370DB	147,112,219
	static inline string_t const MediumSeaGreen				= GText("MediumSeaGreen"s);					//	3CB371	60,179,113
	static inline string_t const MediumSlateBlue			= GText("MediumSlateBlue"s);				//	7B68EE	123,104,238
	static inline string_t const MediumSpringGreen			= GText("MediumSpringGreen"s);				//	00FA9A	0,250,154
	static inline string_t const MediumTurquoise			= GText("MediumTurquoise"s);				//	48D1CC	72,209,204
	static inline string_t const MediumVioletRed			= GText("MediumVioletRed"s);				//	C71585	199,21,133
	static inline string_t const MidnightBlue				= GText("MidnightBlue"s);					//	191970	25,25,112
	static inline string_t const MintCream					= GText("MintCream"s);						//	F5FFFA	245,255,250
	static inline string_t const MistyRose					= GText("MistyRose"s);						//	FFE4E1	255,228,225
	static inline string_t const Moccasin					= GText("Moccasin"s);						//	FFE4B5	255,228,181
	static inline string_t const NavajoWhite				= GText("NavajoWhite"s);					//	FFDEAD	255,222,173
	static inline string_t const Navy						= GText("Navy"s);							//	000080	0,0,128
	static inline string_t const OldLace					= GText("OldLace"s);						//	FDF5E6	253,245,230
	static inline string_t const Olive						= GText("Olive"s);							//	808000	128,128,0
	static inline string_t const OliveDrab					= GText("OliveDrab"s);						//	6B8E23	107,142,35
	static inline string_t const Orange						= GText("Orange"s);							//	FFA500	255,165,0
	static inline string_t const OrangeRed					= GText("OrangeRed"s);						//	FF4500	255,69,0
	static inline string_t const Orchid						= GText("Orchid"s);							//	DA70D6	218,112,214
	static inline string_t const PaleGoldenRod				= GText("PaleGoldenRod"s);					//	EEE8AA	238,232,170
	static inline string_t const PaleGreen					= GText("PaleGreen"s);						//	98FB98	152,251,152
	static inline string_t const PaleTurquoise				= GText("PaleTurquoise"s);					//	AFEEEE	175,238,238
	static inline string_t const PaleVioletRed				= GText("PaleVioletRed"s);					//	DB7093	219,112,147
	static inline string_t const PapayaWhip					= GText("PapayaWhip"s);						//	FFEFD5	255,239,213
	static inline string_t const PeachPuff					= GText("PeachPuff"s);						//	FFDAB9	255,218,185
	static inline string_t const Peru						= GText("Peru"s);							//	CD853F	205,133,63
	static inline string_t const Pink						= GText("Pink"s);							//	FFC0CB	255,192,203
	static inline string_t const Plum						= GText("Plum"s);							//	DDA0DD	221,160,221
	static inline string_t const PowderBlue					= GText("PowderBlue"s);						//	B0E0E6	176,224,230
	static inline string_t const Purple						= GText("Purple"s);							//	800080	128,0,128
	static inline string_t const RebeccaPurple				= GText("RebeccaPurple"s);					//	663399	102,51,153
	static inline string_t const Red						= GText("Red"s);							//	FF0000	255,0,0
	static inline string_t const RosyBrown					= GText("RosyBrown"s);						//	BC8F8F	188,143,143
	static inline string_t const RoyalBlue					= GText("RoyalBlue"s);						//	4169E1	65,105,225
	static inline string_t const SaddleBrown				= GText("SaddleBrown"s);					//	8B4513	139,69,19
	static inline string_t const Salmon						= GText("Salmon"s);							//	FA8072	250,128,114
	static inline string_t const SandyBrown					= GText("SandyBrown"s);						//	F4A460	244,164,96
	static inline string_t const SeaGreen					= GText("SeaGreen"s);						//	2E8B57	46,139,87
	static inline string_t const SeaShell					= GText("SeaShell"s);						//	FFF5EE	255,245,238
	static inline string_t const Sienna						= GText("Sienna"s);							//	A0522D	160,82,45
	static inline string_t const Silver						= GText("Silver"s);							//	C0C0C0	192,192,192
	static inline string_t const SkyBlue					= GText("SkyBlue"s);						//	87CEEB	135,206,235
	static inline string_t const SlateBlue					= GText("SlateBlue"s);						//	6A5ACD	106,90,205
	static inline string_t const SlateGray					= GText("SlateGray"s);						//	708090	112,128,144
	static inline string_t const SlateGrey					= GText("SlateGrey"s);						//	708090	112,128,144
	static inline string_t const Snow						= GText("Snow"s);							//	FFFAFA	255,250,250
	static inline string_t const SpringGreen				= GText("SpringGreen"s);					//	00FF7F	0,255,127
	static inline string_t const SteelBlue					= GText("SteelBlue"s);						//	4682B4	70,130,180
	static inline string_t const Tan						= GText("Tan"s);							//	D2B48C	210,180,140
	static inline string_t const Teal						= GText("Teal"s);							//	008080	0,128,128
	static inline string_t const Thistle					= GText("Thistle"s);						//	D8BFD8	216,191,216
	static inline string_t const Tomato						= GText("Tomato"s);							//	FF6347	255,99,71
	static inline string_t const Turquoise					= GText("Turquoise"s);						//	40E0D0	64,224,208
	static inline string_t const Violet						= GText("Violet"s);							//	EE82EE	238,130,238
	static inline string_t const Wheat						= GText("Wheat"s);							//	F5DEB3	245,222,179
	static inline string_t const White						= GText("White"s);							//	FFFFFF	255,255,255
	static inline string_t const WhiteSmoke					= GText("WhiteSmoke"s);						//	F5F5F5	245,245,245
	static inline string_t const Yellow						= GText("Yellow"s);							//	FFFF00	255,255,0
	static inline string_t const YellowGreen				= GText("YellowGreen"s);					//	9ACD32	154,205,50 

};
