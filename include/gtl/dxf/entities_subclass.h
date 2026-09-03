#pragma once

#include "gtl/dxf/entities_reader.h"

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::dxf::entities {

	enum class eAC_DB {
		none,

		SymbolTable,
		RegAppTableRecord,
		BlockTableRecord,
		DimStyleTableRecord,
		LayerTableRecord,
		LineTypeTableRecord,
		TextStyleTableRecord,
		UCSTableRecord,
		ViewTableRecord,
		ViewportTableRecord,

		BlockBegin,
		BlockEnd,

		Entity,
		Face,
		ModelerGeometry,
		_3dSolid,
		ProxyEntity,
		Circle,
		Arc,
		Text,
		AttributeDefinition,
		Xrecord,
		MText,
		Attribute,
		Dimension,
		AlignedDimension,
		RotatedDimension,
		RadialDimension,
		DiametricDimension,
		_3PointAngularDimension,
		OrdinateDimension,
		Ellipse,
		Hatch,
		Helix,
		RasterImage,
		BlockReference,
		Leader,
		Light,
		Line,
		Polyline,
		SubDMesh,
		MLine,
		OleFrame,
		Ole2Frame,
		Point,
		_2dPolyline,
		_3dPolyline,
		Ray,
		Section,
		Shape,
		Trace,
		Spline,
		Sun,
		Surface,
		ExtrudedSurface,
		LoftedSurface,
		RevolvedSurface,
		SweptSurface,
		Table,
		Fcf,
		UnderlayReference,
		Vertex,
		_2dVertex,
		_3dPolylineVertex,
		Viewport,
		Xline,
	};

	//-----------------------------------------------------------------------------------------------------------------------------

	enum class eSPACE : gcv_t< 67> { model, paper };
	enum class eCOLOR : gcv_t< 62> { byBlock = 0, byLayer = 256, };

	enum fSTANDARD_FLAGS : gcv_t< 70> {
		fSTANDARD__SHAPE			= 0x01,					// If set, this entry describes a shape
		fSTANDARD__VERTICAL_TEXT	= 0x04,					// Vertical text
		fSTANDARD__EXTERNALLY_DEPENDENT_ON_XREF = 0x16,		// If set, table entry is externally dependent on an xref
		fSTANDARD__OK = 0x32,								// If both this bit and bit 16 are set, the externally dependent xref has been successfully resolved
		fSTANDARD__REFERENCED = 0x64,						// If set, the table entry was referenced by at least one entity in the drawing the last time
															// the drawing was edited. (This flag is for the benefit of AutoCADcommands. It can be ignored
															// by most programs that read DXF files and need not be set by programs that write DXF files)
	};
	enum class eUNIT : gcv_t<1070> {
		unitless = 0, inches = 1, feet = 2, miles = 3,
		milimeters = 4, centimeters = 5, meters = 6, kilometers = 7,
		microinches = 8, mils/*a thousandth of an inch*/ = 9, yards = 10,
		angstroms = 11, nanometers = 12, microns = 13, decimeters = 14, decameters = 15, hectometers = 16, gigameters = 17,
		astronomical_units = 18, light_years = 19, parsecs = 20, us_survey_feet = 21, us_survey_inch = 22, us_survey_yard = 23, us_survey_mile = 24
	};
	enum fLAYER_FLAGS : gcv_t<70> {
		fLAYER__FROZEN = 1,
		fLAYER__FROZEN_BY_DEFAULT_IN_NEW_VIEWPORTS = 2,
		fLAYER__LOCKED = 4,
		fLAYER__DEPENDENT_ON_XREF = 16,
		fLAYER__DEPENDENT_RESOLVED = 32,
		fLAYER__REFERENCED = 64,
	};
	enum fCOMPLEX_LINETYPE_FLAGS : gcv_t< 74> {
		fCOMPLEX_LINETYPE__ABSOLUTE_ROTATION = 1,	// code 50
		fCOMPLEX_LINETYPE__STRING = 2,
		fCOMPLEX_LINETYPE__SHAPE = 4,
	};

	enum fHIDDEN : gcv_t< 70> { fHIDDEN__FIRST = 1, fHIDDEN__SECOND = 2, fHIDDEN__THIRD = 4, fHIDDEN__FOURTH = 8 };
	// Text
	enum fTEXT_GENERATION_FLAGS : gcv_t< 71> { fTEXT_GENERATION__BACKWARD = 2, fTEXT_GENERATION__UPSIDE_DOWN = 4 };
	enum class eHORIZONTAL_TEXT_JUSTIFICATION : gcv_t< 72> { left, center, right, aligned, middle, fit };
	enum class eVERTICAL_ALIGNMENT : gcv_t< 73> { baseline, bottom, middle, top};

	// MText
	enum class eTEXT_ATTACHMENT_POINT : gcv_t< 71> {
		top_left = 1, top_center, top_right,
		middle_left, middle_center, middle_right,
		bottom_left, bottom_center, bottom_right
	};
	enum class eDRAWING_DIRECTION : gcv_t< 72> { left_to_right = 1, top_to_bottom = 3, by_style = 5 };
	enum class eLINE_SPACING_STYLE : gcv_t< 73> { at_least = 1, exact = 2 };
	enum class eBACKGROUND_FILL : gcv_t< 90> { off = 0, color = 1, window = 2 };

	enum fATTRIBUTE_FLAGS : gcv_t< 70> { fATTRIBUTE__HIDDEN = 0x01, fATTRIBUTE__CONSTANT = 0x02, fATTRIBUTE__VERIFICATION_REQUIRED = 0x04, fATTRIBUTE__PRESET = 0x08 };
	enum fMTEXT_FLAGS : gcv_t< 70> { fMTEXT_MULTILINE = 0x02, fMTEXT_CONSTANT_MULTILINE = 0x04 };

	enum class eHATCH_STYLE : gcv_t< 75> { odd_parity, outermost_area_only, through_entire_area };
	enum class eHATCH_PATTERN_TYPE : gcv_t< 76> { user_defined = 0, predefined, custom };

	enum class eHANDEDNESS : gcv_t<290> { left, right };
	enum class eHELIX_CONSTRAIN : gcv_t<280> { turn_height, turns, height };

	enum fIMAGE_DISPLAY_FLAGS : gcv_t< 70> {
		fIMAGE_DISPLAY__SHOW_IMAGE = 1,
		fIMAGE_DISPLAY__SHOW_IMAGE_WHEN_NOT_ALIGNED_WITH_SCREEN = 2,
		fIMAGE_DISPLAY__USE_CLIPPING_BOUNDARY = 4,
		fIMAGE_DISPLAY__TRANSPARENT = 8
	};
	enum class eCLIPPING_BOUNDARY : gcv_t< 71> { rectangular = 1, polygonal = 2 };

	enum class eLEADER_CREATION : gcv_t< 73> { text_annotation, tolerance_annotation, block_reference_annotation, no_annotation };
	enum class eLEADER_HOOKLINE_DIRECTION : gcv_t< 74> { opposite_horizontal, same_horizontal };

	enum class eLIGHT_TYPE : gcv_t< 70> { distant, point, spot };
	enum class eATTENUATION_TYPE : gcv_t< 72> { none, inverse_linear, inverse_square };
	enum class eSHADOW_TYPE : gcv_t< 73> { ray_traced_shadow, shadow_maps };

	enum class eLW_POLYLINE_FLAG : gcv_t< 70> { open, closed, plinegen=128} ;
	enum fPOLYLINE_FLAGS : gcv_t< 70> {
		fPOLYLINE__CLOSED = 1, fPOLYLINE__CURVE_FIT = 2, fPOLYLINE__SPLINE_FIT = 4,
		fPOLYLINE__3D = 8, fPOLYLINE_3D_POLYGON_MESH = 16, fPOLYLINE__POLYGON_MESH_CLOSED_IN_N_DIRECTION = 32, fPOLYLINE__POLYFACE = 64,
		fPOLYLINE__CONTINUOUS = 128,
	};
	enum class ePOLYLINE_TYPE : gcv_t< 75> { no_smooth_surface = 0, quadratic_BSpline = 5, cubic_BSpline = 6, bezier_surface = 8 };

	enum class eMESH_PROPERTY_TYPE : gcv_t< 90> { color, material, transparency, material_mapper };

	enum class eMLINE_JUSTIFICATION : gcv_t< 70> { top, zero, bottom };
	enum fMLINE_FLAGS : gcv_t< 71> { fMLINE__HAS_AT_LEAST_ONE_VERTEX, fMLINE__CLOSED, fMLINE__SUPPRESS_START_CAPS, fMLINE__SUPPRESS_END_CAPS };

	enum class eOLE_OBJECT_TYPE : gcv_t< 71> { link = 1, embedded = 2, static_ = 3 };
	enum class eOLE_MODE : gcv_t< 71> { link = 1, embedded = 2, static_ = 3 };

	enum fSPLINE_FLAGS : gcv_t< 70> { fSPLINE__CLOSED = 1, fSPLINE__PERIODIC = 2, fSPLINE__RATIONAL = 4, fSPLINE__PLANAR = 8, fSPLINE__LINEAR = 16 };

	enum class eSUN_SHADOW_TYPE : gcv_t< 70> { ray_traced_shadows = 0, shadow_maps = 1 };

	enum class eSWEEP_ALIGNMENT : gcv_t< 70> { nont = 0, align_sweep_entity_to_path, translate_sweep_entity_to_path, translate_path_to_sweep_entity };

	enum fUNDERLAY_FLAGS : gcv_t<280> { fUNDERLAY__CLIP = 1, fUNDERLAY__UNDERLAY = 2, fUNDERLAY__MONOCHROME= 4, fUNDERLAY__ADJUST_FOR_BACKGROUND = 8, fUNDERLAY__CLIP_INSIDE = 16 };

	enum fVERTEX_FLAGS : gcv_t< 70> {
		fVERTEX__EXTRA = 1, fVERTEX__CURVE_FIT_TANGENT = 2, fVERTEX__NOT_USED = 4, fVERTEX__SPLINE = 8,
		fVERTEX__SPLINE_FRAME_CONTROL_POINT = 16, fVERTEX__3D_POLYLINE_VERTEX = 32, fVERTEX__3D_POLYGON_MESH = 64,
		fVERTEX__POLYFACE_MESH_VERTEX = 128,
	};

	enum fVIEWPORT_FLAGS : gcv_t< 90> {
		fVIEWPORT__ENABLES_PERSPECTIVE_MODE					= 0x01,
		fVIEWPORT__ENABLES_FRONT_CLIPPING					= 0x02,
		fVIEWPORT__ENABLES_BACK_CLIPPING					= 0x04,
		fVIEWPORT__ENABLES_UCS_FOLLOW						= 0x08,
		fVIEWPORT__ENABLES_FRONT_CLIP_NOT_AT_EYE			= 0x10,
		fVIEWPORT__ENABLES_UCS_ICON_VISIBILITY				= 0x20,
		fVIEWPORT__ENABLES_UCS_ICON_AT_ORIGIN				= 0x40,
		fVIEWPORT__ENABLES_FAST_ZOOM						= 0x80,
		fVIEWPORT__ENABLES_SNAP_MODE						= 0x100,
		fVIEWPORT__ENABLES_GRID_MODE						= 0x200,
		fVIEWPORT__ENABLES_ISOMETRIC_SNAP_STYLE				= 0x400,
		fVIEWPORT__ENABLES_HIDE_PLOT_MODE					= 0x800,
		fVIEWPORT__KISO_PAIR_TOP							= 0x1000,		// IF SET AND KISOPAIRRIGHT IS NOT SET, THEN ISOPAIR TOP IS ENABLED
																			// IF BOTH KISOPAIRTOP AND KISOPAIRRIGHT ARE SET, THEN ISOPAIR LEFT IS ENABLED
		fVIEWPORT__KISO_PAIR_RIGHT							= 0x2000,		// IF SET AND KISOPAIRTOP IS NOT SET, THEN ISOPAIR RIGHT IS ENABLED
		fVIEWPORT__ENABLES_VIEWPORT_ZOOM_LOCKING			= 0x4000,
		fVIEWPORT__CURRENTLY_ALWAYS_ENABLED					= 0x8000,
		fVIEWPORT__ENABLES_NON_RECTANGULAR_CLIPPING			= 0x10000,
		fVIEWPORT__TURNS_THE_VIEWPORT_OFF					= 0x20000,
		fVIEWPORT__ENABLES_THE_DISPLAY_OF_THE_GRID			= 0x40000,		// BEYOND THE DRAWING LIMITS
		fVIEWPORT__ENABLE_ADAPTIVE_GRID_DISPLAY				= 0x80000,
		fVIEWPORT__ENABLES_SUBDIVISION						= 0x100000,		// OF THE GRID BELOW THE SET GRID SPACING WHEN THE GRID DISPLAY IS ADAPTIVE
		fVIEWPORT__ENABLES_GRID_FOLLOWS_WORKPLANE_SWITCHING = 0x200000,
	};

	enum class eVIEWPORT_RENDER : gcv_t<281> { classic2d = 0, wireframe, hidden_line, flat_shaded, gouraud_shaded, flat_shaded_with_wireframe, gouraud_shaded_with_wireframe };

	enum class eUCS_PER_VIEWPORT : gcv_t< 71> { UCS_not_change = 0, own_UCS };
	enum class eORTHOGRAPHIC_TYPE : gcv_t< 79> { not_orthographic = 0, top = 1, bottom = 2, front = 3, back = 4, left = 5, right = 6 };
	enum class eLIGHTING_TYPE : gcv_t<282> { one_distant_light, two_distant_lights };
	enum class eSHADE_PLOT : gcv_t<170> { as_displayed = 0, wireframe = 1, hidden = 2, rendered = 3 };

	enum fBLOCK_FLAGS : gcv_t< 70> {
		fBLOCK__ANONYMOUS = 0x01,				// this is an anonymous block generated by hatching, associative dimensioning, other internal operations, or an application
		fBLOCK__NON_CONSTANT = 0x02,			// this block has non-constant attribute definitions
		// (this bit is not set if the block has any attribute definitions that are constant, or has no attribute definitions at all)
		fBLOCK__EXTERNAL_REF = 0x04,			// this block is an external reference (xref)
		fBLOCK__XREF_OVERLAY = 0x08,			// this block is an xref overlay
		fBLOCK__EXTERNALLY_DEPENDENT = 0x16,	// this block is externally dependent
		fBLOCK__EXTERNAL_REFENCE = 0x32,		// this is a resolved external reference, or dependent of an external reference (ignored on input)
		fBLOCK__REFERENCED_EXTERNAL_REFERENCE = 0x64	// this definition is a referenced eternal reference (ignored on input)
	};

	enum fBOUNDARY_PATH_FLAGS : gcv_t< 92> {
		fBOUNDARY_PATH__DEFAULT		= 0x00,
		fBOUNDARY_PATH__EXTERNAL	= 0x01,
		fBOUNDARY_PATH__POLYLINE	= 0x02,
		fBOUNDARY_PATH__DERIVED		= 0x04,
		fBOUNDARY_PATH__TEXTBOX		= 0x08,
		fBOUNDARY_PATH__OUTERMOST	= 0x10,
	};
	enum class eBOUNDARY_EDGE : gcv_t< 72> {
		line = 1, circular_arc = 2, elliptical_arc = 3, spline = 4,
	};

	//=============================================================================================================================
	// TABLE SECTION

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbSymbolTable {
		using this_t = sAcDbSymbolTable;
		constexpr static inline xStringLiteral marker{"AcDbSymbolTable"};
		gcv< 70> maxEntries{};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbDimStyleTable {
		using this_t = sAcDbDimStyleTable;
		constexpr static inline xStringLiteral marker{"AcDbDimStyleTable"};	// NOT in spec.
		gcv< 71> a;									// NOT Documented
		gcv<340> b;									// NOT documented

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbBlockTableRecord {
		using this_t = sAcDbBlockTableRecord;
		constexpr static inline xStringLiteral marker{"AcDbBlockTableRecord"};
		gcv<  2> name;
		gcv<340> pLayoutObject;	// Hard-pointer ID/handle to associated LAYOUT object
		gcv< 70> blockInsertionUnit;
		gcv<280> blockExplodability;
		gcv<281> blockScalability;
		gcv<310> bitmapPreview;
		gcv<1001> xName{"ACAD"s};
		gcv<1002> xBegin{"{"s};
		gcv<1070> version;
		gcv<1070, ::gtl::dxf::entities::eUNIT> unit;
		gcv<1002> xEnd{"}"s};

		std::vector<sGroup> unknowns_;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbDimStyleTableRecord {
		using this_t = sAcDbDimStyleTableRecord;
		constexpr static inline xStringLiteral marker{"AcDbDimStyleTableRecord"};
		gcv<  2> name;
		gcv< 70, fSTANDARD_FLAGS> fStandard;
		gcv<  3> post;
		gcv<  4> apost;
		gcv<  5> blk;	// obsolete, now object ID
		gcv<  6> blk1;	// obsolete, now object ID
		gcv<  7> blk2;	// obsolete, now object ID
		gcv< 40> scale;
		gcv< 41> asz;
		gcv< 42> exo;
		gcv< 43> dli;
		gcv< 44> exe;
		gcv< 45> rnd;
		gcv< 46> dle;
		gcv< 47> tp;
		gcv< 48> tm;
		gcv<140> txt;
		gcv<141> cen;
		gcv<142> tsz;
		gcv<143> altf;
		gcv<144> lfac;
		gcv<145> tvp;
		gcv<146> tfac;
		gcv<147> gap;
		gcv<148> altrnd;
		gcv< 71> tol;
		gcv< 72> lim;
		gcv< 73> tih;
		gcv< 74> toh;
		gcv< 75> se1;
		gcv< 76> se2;
		gcv< 77> tad;
		gcv< 78> zin;
		gcv< 79> azin;
		gcv<170> alt;
		gcv<171> altd;
		gcv<172> tofl;
		gcv<173> sah;
		gcv<174> tix;
		gcv<175> soxd;
		gcv<176> clrd;
		gcv<177> clre;
		gcv<178> clrt;
		gcv<179> adec;
		gcv<270> unit;	// (obsolete, now use dimlunit and dimfrac)
		gcv<271> dec;
		gcv<272> tdec;
		gcv<273> altu;
		gcv<274> alttd;
		gcv<275> aunit;
		gcv<276> frac;
		gcv<277> lunit;
		gcv<278> dsep;
		gcv<279> tmove;
		gcv<280> just;
		gcv<281> sd1;
		gcv<282> sd2;
		gcv<283> tolj;
		gcv<284> tzin;
		gcv<285> altz;
		gcv<286> alttz;
		gcv<287> fit;	// (obsolete, now use dimatfit and dimtmove)
		gcv<288> upt;
		gcv<289> atfit;
		gcv<340> htxsty;	// (handle of referenced style)
		gcv<341> hldrblk; // (handle of referenced block)
		gcv<342> hblk;	// (handle of referenced block)
		gcv<343> hblk1;	// (handle of referenced block)
		gcv<344> hblk2;	// (handle of referenced block)
		gcv<371> lwd;	// (lineweight enum value)
		gcv<372> lwe;	// (lineweight enum value)

		std::vector<sGroup> unknowns_;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbLayerTableRecord {
		using this_t = sAcDbLayerTableRecord;
		constexpr static inline xStringLiteral marker{"AcDbLayerTableRecord"};
		gcv<  2> name;
		gcv< 70, entities::fLAYER_FLAGS> flags;
		gcv< 62> iColor;				// if negative, layer is off
		gcv<  6> linetype;
		gcv<290> bDoNotPlot;
		gcv<370> eLineWeight;			// (lineweight enum value)
		gcv<390> pPlotStyleName;		// hard-pointer ID/handle of PlotStyleName object
		gcv<347> pMaterial;				// hard-pointer ID/handle of Material object
		gcv<420, color_bgra_t> color24;	// (color value, RGB) NOT in the documents

		std::vector<sGroup> unknowns_;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbLinetypeTableRecord {
		using this_t = sAcDbLinetypeTableRecord;
		constexpr static inline xStringLiteral marker{"AcDbLinetypeTableRecord"};
		gcv<  2> name;
		gcv< 70, fSTANDARD_FLAGS> fStandard;
		gcv<  3> linetypeDescription;
		gcv< 72> align{'A'};	// always 'A'. obsolete?
		gcv< 73> nLineTypeElement;
		gcv< 40> lenTotalPattern;
		struct sDetail {
			gcv< 49> lenDashDotSpace;
			gcv< 74, entities::fCOMPLEX_LINETYPE_FLAGS> fLineTypes;
			gcv< 75> iShapes;		// valid fLineType is set to shape (fCOMPLEX_LINETYPE__SHAPE)
			gcv<340> pStyleObject;	// one per element if code 74 > 0
			gcv< 46> scale;
			gcv< 50> angle;
			gcv< 44> offsetX;
			gcv< 45> offsetY;
			gcv<  9> text;
		};
		TListVariable<sDetail, &this_t::nLineTypeElement> details;

		std::vector<sGroup> unknowns_;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbTextStyleTableRecord {
		using this_t = sAcDbTextStyleTableRecord;
		constexpr static inline xStringLiteral marker{"AcDbTextStyleTableRecord"};
		gcv<  2> name;
		gcv< 70, fSTANDARD_FLAGS> fStandard;			// 1 : shape, 4: vertical text, 16, 32, 64 - same as above
		gcv< 40> height;
		gcv< 41> widthFactor;
		gcv< 50> obliqueAngle;
		gcv< 71> fGeneration;							// 2 : mirrored in X, 4: mirrored in Y
		gcv< 42> lastHeightUsed;
		gcv<  3> namePrimaryFontFile;
		gcv<  4> nameBigfontFile;
		gcv<1071> fullname;	// a long value which contains a truetype font's pitch and family, character set, and italic and bold flags

		std::vector<sGroup> unknowns_;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};
	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbUCSTableRecord {
		using this_t = sAcDbUCSTableRecord;
		constexpr static inline xStringLiteral marker{"AcDbUCSTableRecord"};
		gcv<  2> name;
		gcv< 70, fSTANDARD_FLAGS> fStandard;
		mgcv<point_t, 10, 20, 30> pt;		// WCS
		mgcv<point_t, 11, 21, 31> xAxis;	// X axis direction vector
		mgcv<point_t, 12, 22, 32> yAxis;	// Y axis direction vector
		gcv< 79> reserved;					// always 0
		gcv<146> elevation;
		gcv<346> hUCS;						// ID/handle of UCS if this is an orthographic. This code is not present if the 79 code is 0.
		// if this code is not present and code 79 is non-zero, then base UCS is assumed to be WORLD
		gcv< 71, entities::eORTHOGRAPHIC_TYPE> eOrthographicType;
		mgcv<point_t, 13, 23, 33> ptOriginRelToUCS;	// Origin for this orthographic type relative to this UCS.

		std::vector<sGroup> unknowns_;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};
	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbViewTableRecord {
		using this_t = sAcDbViewTableRecord;
		constexpr static inline xStringLiteral marker{"AcDbViewTableRecord"};
		gcv<  2> name;
		gcv< 70, fSTANDARD_FLAGS> fStandard;										// 1: paper space view, 16: ...
		gcv< 40> height;										// in DCS
		mgcv<point_t, 10, 20> ptCenter;
		gcv< 41> width;											// in DCS
		mgcv<point_t, 11, 21, 31> vcViewDirectionFromTarget;	// in DCS
		mgcv<point_t, 12, 22, 32> ptTarget;						// target point (in WCS)
		gcv< 42> lensLength;					// lens length of the view
		gcv< 43> clippingPlaneFront;
		gcv< 44> clippingPlaneBack;
		gcv< 50> angleTwist;
		gcv< 71> eViewMode;						// VIEWMODE system variable
		gcv<281, entities::eVIEWPORT_RENDER> eRenderMode;
		gcv< 72> bUCSAssociated;
		gcv< 73> bPlottableCamera;
		gcv<332> pBackgroupObject;				// Soft-pointer ID/handle to background object (optional)
		gcv<334> pLiveSectionObject;			// Soft-pointer ID/handle to live section object (optional)
		gcv<348> pVisualStyleObject;			// Soft-pointer ID/handle to visual style object (optional)
		gcv<361> sunHardOwnerShiptID;

		// appears only if code 72 is set to 1
		mgcv<point_t, 110, 120, 130> ptUCS;
		mgcv<point_t, 111, 121, 131> ptUCS_X;	// X-axis
		mgcv<point_t, 112, 122, 132> ptUCS_Y;	// Y-axis
		gcv< 79, entities::eORTHOGRAPHIC_TYPE> eOrthographicType;
		gcv<146> elevation;						// UCS elevation
		gcv<345> pUCSTableRecord;				// ID/handle of AcDbUCSTableRecord if UCS is a named UCS. If not present, then UCS is unnamed
		gcv<346> pBaseUCSTableRecord;			// ID/handle of AcDbUCSTableREcord of base UCS if UCS is orthographic (79 code is non-zero).
												// If not present and 79 code is non-zero, then base UCS is taken to be WORLD

		std::vector<sGroup> unknowns_;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};
	struct sAcDbViewportTableRecord {
		using this_t = sAcDbViewportTableRecord;
		constexpr static inline xStringLiteral marker{"AcDbViewportTableRecord"};
		gcv<  2> name;
		gcv< 70, fSTANDARD_FLAGS> fStandard;				// 16, 32, 64
		mgcv<point_t, 10, 20> ptLowerLeft;
		mgcv<point_t, 11, 21> ptUpperRight;
		mgcv<point_t, 12, 22> ptCenterDCS;
		mgcv<point_t, 13, 23> ptSnapBaseDCS;
		mgcv<point_t, 14, 24> ptSnapSpacing;
		mgcv<point_t, 15, 25> ptGridSpacing;
		mgcv<point_t, 16, 26, 36> vcViewDirectionFromTargetPointWCS;
		mgcv<point_t, 17, 27, 37> ptViewTargetWCS;
		gcv< 42> lengthLens;
		gcv< 43> clippingPlaneFront;
		gcv< 44> clippingPlaneBack;
		gcv< 45> heightView;
		gcv< 50> angleSnapRotation;
		gcv< 51> angleViewTwist;
		gcv< 72> circleSides;
		lgcv<331, 0> pFrozenLayerObjects;
		lgcv<441, 0> pFrozenLayerObjects2;
		gcv<  1> namePlotStyleSheet;
		gcv<281, entities::eVIEWPORT_RENDER> eRenderMode;
		gcv< 71> eViewMode;				// VIEWMODE system variable
		gcv< 74> UCSICON;
		mgcv<point_t, 110, 120, 130> ptUCSOrigin;
		mgcv<point_t, 111, 121, 131> vcUCS_X;
		mgcv<point_t, 112, 122, 132> vcUCS_Y;
		gcv<345> pUCSTableRecord;		// ID/handle of AcDbUCSTableRecord if UCS is a named UCS. If not present, then UCS is unnamed
		gcv<346> pBaseUCSTableRecord;	// ID/handle of AcDbUCSTableREcord of base UCS if UCS is orthographic (79 code is non-zero).
		// If not present and 79 code is non-zero, then base UCS is taken to be WORLD
		gcv< 79, entities::eORTHOGRAPHIC_TYPE> eOrthographicType;
		gcv<146> elevation;				// UCS elevation
		gcv<170> shadePlotSetting;
		gcv< 61> majorGridLines;
		gcv<332> pBackgroundObject;		// Soft-pointer ID/handle to background object (optional)
		gcv<333> pShadePlotObject;		// Soft-pointer ID/handle to shade plot object (optional)
		gcv<348> pVisualStyleObject;	// Hard-pointer ID/handle to visual style object (optional)
		gcv<292> defaultLightingOnFlag;
		gcv<282, entities::eLIGHTING_TYPE> eDefaultLightingType;
		gcv<141> brightness;
		gcv<142> contrast;
		gcv< 63> ambientColorIndex;		// only output when non-black
		gcv<421, color_bgra_t> ambientColorValue;		// only output when non-black
		gcv<431> ambientColorName;		// only output when non-black

		std::vector<sGroup> unknowns_;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbRegAppTableRecord {
		using this_t = sAcDbRegAppTableRecord;
		constexpr static inline xStringLiteral marker{"AcDbRegAppTableRecord"};
		gcv<  2> name;
		gcv< 70, fSTANDARD_FLAGS> fStandard;	// standard flag values (bit-coded values):
												// 16: if set, table entry is externally dependent on an xref
												// 32: if both this bit and bit 16 are set, the externally dependent xref has been successfully resolved
												// 64: if set, the table entry was referenced by at least one entity in the drawing the last time the drawing was edited.

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//=============================================================================================================================
	// BLOCK SECTION

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbBlockBegin {
		using this_t = sAcDbBlockBegin;
		constexpr static inline xStringLiteral marker{"AcDbBlockBegin"};
		gcv<  2> name;
		gcv< 70, entities::fBLOCK_FLAGS> flags{};
		mgcv<point_t, 10, 20, 30> ptBase;
		gcv<  3> name2;
		gcv<  1> xrefPath;
		gcv<  4> description;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbBlockEnd {
		using this_t = sAcDbBlockEnd;

		constexpr static inline xStringLiteral marker{"AcDbBlockEnd"};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};


	//=============================================================================================================================
	// ENTITY SECTION

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbEntity {
	public:
		using this_t = sAcDbEntity;
	public:
		//gcv<  5> handle{};
		//gcv<330> hOwnerBlock;

		constexpr static inline xStringLiteral marker{"AcDbEntity"};
		gcv< 67, eSPACE> space{};					// 0 for model, 1 for paper
		gcv<410> layoutTabName;
		gcv<  8> layer;
		gcv<  6> lineTypeName;
		gcv<347> ptrMaterial;
		gcv< 62, eCOLOR> color{ eCOLOR::byLayer };	// 0 for ByBlock, 256 for ByLayer, negative value indicates layer is off.
		gcv<370> lineWeight;						// Stored and moved around as a 16-bit integer (?)
		gcv< 48> lineTypeScale{ 1.0 };
		gcv< 60> bHidden;							// 0: visible, 1: invisible
		gcv< 92> nGraphicsData;
		lgcv<310, 0> graphicsData;
		gcv<420, color_bgra_t> color24;				// A 24-bit color value that should be dealt with in terms of bytes with values of 0 to 255.
													// The lowest byte is the blue value, the middle byte is the green value, and the third byte is the red value.
													// The top byte is always 0.
													// The group code cannot be used by custom entities for their own data
													// because the group code is reserved for AcDbEntity, class-level color data and AcDbEntity, class-level transparency data
		gcv<430> colorName;
		gcv<440> transparency{};
		gcv<390> ptrPlotStyleObject{};
		gcv<284> shadowMode{};						// 0 : Casts and received shadows, 1 : Casts shadows, 2 : Receives shadows, 3 : Ignores shadows
		gcv<160> dummy{};

		//point_t extrusion{0., 0., 1.};
		//gcv< 39> thickness{};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDb3DFace {
		using this_t = sAcDb3DFace;
		constexpr static inline xStringLiteral marker{"AcDbFace"};
		mgcv<point_t, 10, 20, 30> pt1;
		mgcv<point_t, 11, 21, 31> pt2;
		mgcv<point_t, 12, 22, 32> pt3;
		mgcv<point_t, 13, 23, 33> pt4;
		gcv< 70, fHIDDEN> flags{};	// 70:

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbModelerGeometry {
	public:
		using this_t = sAcDbModelerGeometry;
	public:
		constexpr static inline xStringLiteral marker{"AcDbModelerGeometry"};
		gcv< 70> version{ 1 };
		sgcv<1, 3> proprietaryData;		// GroupCode 1, 3
		gcv<290> a;	// not in documents
		gcv<  2> b;	// not in documents

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDb3DSolid {
	public:
		using this_t = sAcDb3DSolid;
	public:
		constexpr static inline xStringLiteral marker{"AcDb3dSolid"};				// Subclass marker (AcDb3dSolid)
		gcv<350> pHistoryObject{};		// Soft-owner ID/handle to history object

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbProxyEntity {
		using this_t = sAcDbProxyEntity;
		constexpr static inline xStringLiteral marker{"AcDbProxyEntity"};
		gcv< 90> proxyEntityClassID{ 498 };
		gcv< 91> applicationEntityClassID{ 500 };
		gcv< 92> sizeGraphicsData{};
		gcv<310> graphicsData{};
		gcv< 93> sizeEntityData_inBits{};
		gcv<310> entityData{};
		gcv<330> objectID0{};
		gcv<340> objectID1{};
		gcv<350> objectID2{};
		gcv<360> objectID3{};
		gcv< 94> endOfObjectIDSection{};
		gcv< 95> sizeProxyData{};
		gcv< 70> dwg_or_dxf{};	// Original custom object data format (0 = DWG, 1 = DXF)

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbArc {
	public:
		using this_t = sAcDbArc;
	public:
		constexpr static inline xStringLiteral marker{"AcDbArc"};
		gcv< 50> startAngle{};
		gcv< 51> endAngle{};
		gcv_extrusion extrusion{{0., 0., 1.}};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbPoint {
	public:
		using this_t = sAcDbPoint;
	public:
		constexpr static inline xStringLiteral marker{"AcDbPoint"};
		mgcv<point_t, 10, 20, 30> pt{};
		gcv< 39> thickness{};
		gcv_extrusion extrusion{{0., 0., 1.}};
		gcv< 50> angle{};	// angle of x axis for UCS inf effect when the point was drawn (optional, default = 0); used when PDMODE is nonzero

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbLine {
	public:
		using this_t = sAcDbLine;
	public:
		constexpr static inline xStringLiteral marker{"AcDbLine"};
		mgcv<point_t, 10, 20, 30> pt0;
		mgcv<point_t, 11, 21, 31> pt1;
		gcv< 39> thickness;
		gcv_extrusion extrusion{{0., 0., 1.}};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbText {
	public:
		using this_t = sAcDbText;
	public:
		constexpr static inline xStringLiteral marker{"AcDbText"};	// Subclass marker (AcDbText)
		gcv< 39> thickness{};
		mgcv<point_t, 10, 20, 30> ptAlign0;	// alignment point
		gcv< 40> heightText{};
		sgcv< 1, 3> text;
		gcv< 50> rotation{};
		gcv< 41> scaleX{1.};
		gcv< 51> angleOblique{};
		gcv<  7> textStyleName{"STANDARD"s};
		gcv< 71, fTEXT_GENERATION_FLAGS> fFlags{};	// 71:
		gcv< 72, eHORIZONTAL_TEXT_JUSTIFICATION> eHorzJustification{};	// 72:
		mgcv<point_t, 11, 21, 31> ptAlign1{};	// second alignment point
		gcv_extrusion extrusion{{0., 0., 1.}};

		gcv<100> marker2{"AcDbText"};			// Exception. (works as end marker for AcDbText)
		gcv< 73, eVERTICAL_ALIGNMENT> vertical_alignment{};	// 73:

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbMText {
	public:
		using this_t = sAcDbMText;
	public:
		constexpr static inline xStringLiteral marker{"AcDbMText"};
		mgcv<point_t, 10, 20, 30> pt;				// insertion point
		gcv< 40> height;							// nominal text height
		gcv< 41> widthReferenceRectangle;
		gcv< 46> heightAnnotation;  // updated name
		gcv< 71, eTEXT_ATTACHMENT_POINT> eAttachmentPoint{eTEXT_ATTACHMENT_POINT::top_left};		// 71
		gcv< 72, eDRAWING_DIRECTION> eDrawingDirection{eDRAWING_DIRECTION::left_to_right};			// 72
		sgcv< 1, 3> text;								// group code 1, 3
		gcv<  7> textStyleName{"STANDARD"s};
		gcv_extrusion extrusion{{0., 0., 1.}};
		mgcv<point_t, 11, 21, 31> direction{};
		gcv< 42> widthChar{};
		gcv< 43> heightChar{};
		gcv< 50, rad_t> rotation{};						// in radian.
		gcv< 73, eLINE_SPACING_STYLE> eLineSpacingStyle{eLINE_SPACING_STYLE::exact};		// 73:
		gcv< 44> lineSpacingFactor{ 1. };													// 0.25 ~ 4.0
		gcv< 90, eBACKGROUND_FILL> eBackgroundFillColor{eBACKGROUND_FILL::off};			// 90:
		gcv< 63> iBackgroundColor;
		gcv<420, color_bgra_t> crBackgroundFillColorValue0{};
		gcv<421, color_bgra_t> crBackgroundFillColorValue1{};
		gcv<422, color_bgra_t> crBackgroundFillColorValue2{};
		gcv<423, color_bgra_t> crBackgroundFillColorValue3{};
		gcv<424, color_bgra_t> crBackgroundFillColorValue4{};
		gcv<425, color_bgra_t> crBackgroundFillColorValue5{};
		gcv<426, color_bgra_t> crBackgroundFillColorValue6{};
		gcv<427, color_bgra_t> crBackgroundFillColorValue7{};
		gcv<428, color_bgra_t> crBackgroundFillColorValue8{};
		gcv<429, color_bgra_t> crBackgroundFillColorValue9{};
		gcv<430> strBackgroundFillColorName0{};
		gcv<431> strBackgroundFillColorName1{};
		gcv<432> strBackgroundFillColorName2{};
		gcv<433> strBackgroundFillColorName3{};
		gcv<434> strBackgroundFillColorName4{};
		gcv<435> strBackgroundFillColorName5{};
		gcv<436> strBackgroundFillColorName6{};
		gcv<437> strBackgroundFillColorName7{};
		gcv<438> strBackgroundFillColorName8{};
		gcv<439> strBackgroundFillColorName9{};
		gcv< 45> scaleFillBox{1.};
		gcv< 63> iBackgroundFillColor{};				// valid when groupcode 90 is 1
		gcv<441> transparency_na{};						// not implemented
		gcv< 75> columnType{};							// 0 : no columns, 1 : one column, 2 : continuous columns
		gcv< 76> columnCount{};
		gcv< 78> columnFlowReversed{};
		gcv< 79> columnAutoHeight{};
		gcv< 48> columnWidth{};
		gcv< 49> columnGutter{};
		gcv< 50> columnHeights{};						// this code is followed by column count (int16), and then the number of column heights

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbAttributeDefinition {
	public:
		using this_t = sAcDbAttributeDefinition;
	public:
		constexpr static inline xStringLiteral marker{"AcDbAttributeDefinition"};
		gcv<280> versionNumber;	// 0 = 2010
		gcv<  3> prompt{};
		gcv<  2> tag{};
		gcv< 70, fATTRIBUTE_FLAGS> flags{};	// 70:
		gcv< 73> fieldLength{};
		gcv< 74> verticalTextJustification{};
		gcv<280> lockPosition{};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbAttribute {
	public:
		using this_t = sAcDbAttribute;
	public:
		constexpr static inline xStringLiteral marker{"AcDbAttribute"};
		gcv<280> version_number;					// 0 = 2010
		gcv<  2> tag{};
		gcv< 70, fATTRIBUTE_FLAGS> flags{};
		gcv< 73> fieldLength{};
		gcv< 50> angle{};
		gcv< 41> scaleX{ 1. };
		gcv< 51> angleOblique{0.};
		gcv<  7> textStyleName{ "STANDARD"s };
		gcv< 71> textGenerationFlags{};
		gcv< 72, eHORIZONTAL_TEXT_JUSTIFICATION> eHorzJustification{};
		gcv< 74> verticalTextJustification{};		// see groupcode 73 in TEXT
		mgcv<point_t, 11, 21, 31> ptAlignmentOCS{};	// alignment point for attribute. Present only if 72 or 74 group is present and nonzero
		gcv_extrusion extrusion{{0., 0., 1.}};
		gcv<280> lockPosition{};					// 0 : unlocked, 1 : locked

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbXRecord {
	public:
		using this_t = sAcDbXRecord;
	public:
		constexpr static inline xStringLiteral marker{"AcDbXrecord"};

		gcv<280> bDuplicateRecordCloning{};			// 1 : keep existing
		gcv< 70, fMTEXT_FLAGS> fMText{};
		gcv< 70> bReallyLocked{};					// 0 : unlocked, 1 : locked
		gcv< 70> nSecondaryAttributes{};
		lgcv<340, &this_t::nSecondaryAttributes> hardPointerIDofSecondaryAttributes{};
		mgcv<point_t, 10, 20, 30> ptAlignment;
		gcv< 40> scaleCurrentAnnotation{ 1. };		// no default value specified
		gcv<  2> attribute;
		std::vector<sGroup> params;

		DEFINE_SPACESHIP_OPERATOR(this_t);

		bool PreRead(group_iter_t& iter) {
			if (!iter)
				return false;
			auto const eCode = iter->eCode;
			if (eCode == 5 or eCode == 105 or eCode < 1 or eCode > 369)
				return false;
			params.push_back(*iter);
			iter++;
			return true;
		}
	};

	////-----------------------------------------------------------------------------------------------------------------------------
	//struct sAcDbBody {
	//public:
	//	using this_t = sAcDbBody;
	//public:
	//	constexpr static inline xStringLiteral marker{"AcDbModelerGeometry"};
	//	gcv< 70> version{1};
	//	sgcv< 1, 3> proprietaryData;		// GroupCode 1, 3

	//	DEFINE_SPACESHIP_OPERATOR(this_t);
	//};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbCircle {
	public:
		using this_t = sAcDbCircle;
	public:
		constexpr static inline xStringLiteral marker{"AcDbCircle"};
		gcv< 39> thickness{};
		mgcv<point_t, 10, 20, 30> pt;
		gcv< 40> radius;
		gcv_extrusion extrusion{{0., 0., 1.}};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbNavisworksModel {
		using this_t = sAcDbNavisworksModel;
		constexpr static inline xStringLiteral marker{"AcDbNavisworksModel"};
		gcv<340> pNavisworksModelObject;	// Soft-pointer ID/handle to AcDbNavisworksModelDef object
		lgcv<40, 16> transform;				// Model transform in wcs coordinates (4x4 matrix)
		gcv< 40> insertionUnitFactor;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	// todo: AcDbDimension

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbEllipse {
	public:
		using this_t = sAcDbEllipse;
	public:
		constexpr static inline xStringLiteral marker{"AcDbEllipse"};
		mgcv<point_t, 10, 20, 30> pt;
		mgcv<point_t, 11, 21, 31> vcMajor;	// relative to the center
		gcv_extrusion extrusion{{0., 0., 1.}};
		gcv< 40> ratio;	// Ratio of minor axis to major axis
		gcv< 41, rad_t> angle0;	// in radian, 0.0 for a full ellipse
		gcv< 42, rad_t> angle1;	// in radian, 2pi for a full ellipse

		DEFINE_SPACESHIP_OPERATOR(this_t);

	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbHatch {
	public:
		using this_t = sAcDbHatch;

		struct sPolylineBoundary {
			using this_t = sPolylineBoundary;
			gcv< 72> bBulge;
			gcv< 73> bClosed;
			gcv< 93> nVertices;
			struct sVertex {
				using this_t = sVertex;
				mgcv<point_t, 10, 20> pt;	// in OCS
				gcv< 42> bulge;
				DEFINE_SPACESHIP_OPERATOR(this_t);
			};
			TListVariable<sVertex, &this_t::nVertices> pt;
			DEFINE_SPACESHIP_OPERATOR(this_t);
		};
		struct sLineEdge {
			using this_t = sLineEdge;
			mgcv<point_t, 10, 20> pt0;	// in OCS
			mgcv<point_t, 11, 21> pt1;	// in OCS
			DEFINE_SPACESHIP_OPERATOR(this_t);
		};
		struct sArcEdge {
			using this_t = sArcEdge;
			mgcv<point_t, 10, 20> ptCenter;		// in OCS
			gcv< 40> radius;					// in OCS
			gcv< 50, rad_t> angleStart;			// in radians
			gcv< 51, rad_t> angleEnd;			// in radians
			gcv< 73> bCounterClockwise;
			DEFINE_SPACESHIP_OPERATOR(this_t);
		};
		struct sEllipseEdge {
			using this_t = sEllipseEdge;
			mgcv<point_t, 10, 20> ptCenter;		// in OCS
			mgcv<point_t, 11, 21> vcMajorAxis;	// in OCS
			gcv< 40> lengthMinorAxis;			// Ratio of minor axis to major axis
			gcv< 50, rad_t> angleStart;
			gcv< 51, rad_t> angleEnd;
			gcv< 73> bCounterClockwise;
			DEFINE_SPACESHIP_OPERATOR(this_t);
		};
		struct sSplineEdge {
			using this_t = sSplineEdge;
			gcv< 94> degree;
			gcv< 73> rational;
			gcv< 74> periodic;
			gcv< 95> nKnot;
			gcv< 96> nControlPoints;
			lgcv< 40, &this_t::nKnot> knots;
			TListVariable<mgcv<point_t, 10, 20>, &this_t::nControlPoints> controlPoints;	// in OCS
			lgcv< 42, &this_t::nControlPoints> weights;
			gcv< 97> nFitData;
			TListVariable<mgcv<point_t, 11, 21>, &this_t::nFitData> fitData;	// in OCS
			mgcv<point_t, 12, 22> vcStartTangent;	// in OCS
			mgcv<point_t, 13, 23> vcEndTangent;		// in OCS
			DEFINE_SPACESHIP_OPERATOR(this_t);
		};
		using edge_t = std::variant<sLineEdge, sArcEdge, sEllipseEdge, sSplineEdge>;

		struct sHatchBoundaryPath {
			using this_t = sHatchBoundaryPath;
			gcv< 92, fBOUNDARY_PATH_FLAGS> fFlags;
			std::optional<sPolylineBoundary> polylineBoundary;					// if fFlags & fBOUNDARY_PATH__POLYLINE
			gcv< 93> nEdges;									// only if boundary is not a polyline
			TListVariable<gcv< 72, eBOUNDARY_EDGE>, &this_t::nEdges> eEdgeTypes;
			std::vector<edge_t> edges;
			gcv< 97> nSourceBoundaryObject;
			lgcv<330, &this_t::nSourceBoundaryObject> pSourceBoundaryObject;    // Reference to source boundary objects (multiple entries)
			DEFINE_SPACESHIP_OPERATOR(this_t);

			bool PreRead(group_iter_t& iter) {
				if ((fFlags() & fBOUNDARY_PATH__POLYLINE) and !polylineBoundary) {
					polylineBoundary.emplace();
					return ReadFieldMembers(*polylineBoundary, iter);
				}
				if (eEdgeTypes.IsGroupCodeMatch(*iter)) {
					if (!eEdgeTypes.SetFromGroup(*this, iter))
						return false;
				#ifdef _DEBUG
					auto distance = iter.Distance();
				#endif
					switch (eEdgeTypes().back().value) {
					case eBOUNDARY_EDGE::line:
						edges.emplace_back(sLineEdge{});
						return ReadFieldMembers(std::get<sLineEdge>(edges.back()), iter);
					case eBOUNDARY_EDGE::circular_arc:
						edges.emplace_back(sArcEdge{});
						return ReadFieldMembers(std::get<sArcEdge>(edges.back()), iter);
					case eBOUNDARY_EDGE::elliptical_arc:
						edges.emplace_back(sEllipseEdge{});
						return ReadFieldMembers(std::get<sEllipseEdge>(edges.back()), iter);
					case eBOUNDARY_EDGE::spline:
						edges.emplace_back(sSplineEdge{});
						return ReadFieldMembers(std::get<sSplineEdge>(edges.back()), iter);
					}
				}

				return false;
			}
		};

		struct sPattern {
			using this_t = sPattern;
			gcv< 53> anglePatternLine;
			mgcv<point_t, 43, 44> pt;
			mgcv<point_t, 45, 46> vc;
			gcv< 79> nLength;
			lgcv< 49, &this_t::nLength> patternLineLengths;	// Lengths of pattern lines
			DEFINE_SPACESHIP_OPERATOR(this_t);
		};


	public:
		constexpr static inline xStringLiteral marker{"AcDbHatch"};

		mgcv<point_t, 10, 20, 30> ptElevation;				// in OCS, x = y = 0, z : elevation
		gcv_extrusion extrusion{{0., 0., 1.}};
		gcv<  2> pattern_name;
		gcv< 70> bSolidPattern{};			// 0:pattern fill; 1=solid fill; for MPolygon, the version of MPolygon
		gcv< 63> patternFillColorAci;		// for MPolygon
		gcv< 71> bAssociative;				// boolean
		//== boundary path
		gcv< 91> nBoundaryPath;
		TListVariable<sHatchBoundaryPath, &this_t::nBoundaryPath> boundaryPaths;
		gcv< 75> hatchStyle;
		gcv< 76> patternType;
		gcv< 52> patternAngle;
		gcv< 41> patternScale_or_Spacing;	// pattern fill only
		gcv< 73> bBoundaryAnnotation;		// boolean
		gcv< 77> bDoublePattern;			// boolean
		//== pattern definition lines
		gcv< 78> nPatternDefinitionLines;
		TListVariable<sPattern, &this_t::nPatternDefinitionLines> patternDefinitionLines;
		gcv< 47> pixelSize;
		gcv< 98> nSeedPoint;
		gcv< 11> offsetVector;				// for MPolygon
		gcv< 99> nDegenerateBoundaryPath;
		lgcv<10, &this_t::nSeedPoint> seedsX;
		lgcv<20, &this_t::nSeedPoint> seedsY;
		gcv<450> solidHatch_or_Gradient;
		gcv<451> reserved;
		gcv<452> bSingleColorGradient;		// 0:two-color gradient, 1:single-color gradient;
		gcv<453> nColors;					// 0:Solid hatch, 2:gradient
		gcv<460, rad_t> angle;				// in radians
		gcv<461> gradientDefinition;		// 0.0:non shifted, 1.0:shifted
		gcv<462> colorTint;
		gcv<463> reserved2;
		gcv<470> String{"LINEAR"};			// default : "LINEAR"

		lgcv<463, 0> a;							// not in documents
		lgcv< 63, 0> b;							// not in documents
		lgcv<421, 0> c;							// not in documents

		//std::vector<sGroup> unknowns_;		// 92, 93

		DEFINE_SPACESHIP_OPERATOR(this_t);

	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbHelix {
	public:
		using this_t = sAcDbHelix;
	public:
		constexpr static inline xStringLiteral marker{"AcDbHelix"};
		gcv< 90> releaseMajor;
		gcv< 91> releaseMaintenance;
		mgcv<point_t, 10, 20, 30> ptAxisBase;
		mgcv<point_t, 11, 21, 31> ptStart;
		mgcv<point_t, 12, 22, 32> vAxis;
		gcv< 40> radius;
		gcv< 41> nTurns;
		gcv<290, eHANDEDNESS> eHandedness;	// 0: right handed, 1: left handed
		gcv<280, eHELIX_CONSTRAIN> eConstrainType;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbImage {
	public:
		using this_t = sAcDbImage;
	public:
		constexpr static inline xStringLiteral marker{"AcDbRasterImage"};
		gcv< 90> classVersion;
		mgcv<point_t, 10, 20, 30> pt;
		mgcv<point_t, 11, 21, 31> vcU;
		mgcv<point_t, 12, 22, 32> vcV;
		gcv< 13> imageWidth; // vcU
		gcv< 23> imageHeight; // vcV
		gcv<340> hImage; // hard reference to image data
		gcv< 70, fIMAGE_DISPLAY_FLAGS> fProperty{}; // 70:
		gcv<280> bClipping; // 0 : off, 1 : on
		gcv<281> brightness{50}; // 0-100, Default:50
		gcv<282> contrast{50}; // 0-100, Default:50
		gcv<283> fade{0}; // 0-100, Default:0
		gcv<360> hReactor; // hard reference to image reactor
		gcv< 71, eCLIPPING_BOUNDARY> eClippingBoundary{};
		gcv< 91> numberOfClippingBoundaryVertices{};
		gcv< 14> imageWidthClippingBoundary; // vcU
		gcv< 24> imageHeightClippingBoundary; // vcV
		gcv<290> bClipInside; // 0: clip outside, 1: clip inside

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbBlockReference {
		using this_t = sAcDbBlockReference;
		constexpr static inline xStringLiteral marker{"AcDbBlockReference"};
		gcv< 66> bAttributeEntitiesFollow;
		gcv<  2> blockName;					// name of the block
		mgcv<point_t, 10, 20, 30> ptInsertionPoint;	// insertion point in OCS
		mgcv<point_t, 41, 42, 43> scale{{1., 1., 1.}};
		gcv< 50> angleRotation{0.};	// deg? rad?
		gcv< 70> cols{1};
		gcv< 71> rows{1};
		gcv< 44> colSpacing{0.};
		gcv< 45> rowSpacing{0.};
		gcv_extrusion extrusion{{0., 0., 1.}};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbLeader {
		using this_t = sAcDbLeader;
		constexpr static inline xStringLiteral marker{"AcDbLeader"};
		gcv<  3> dimensionStyleName;
		gcv< 71> bArrowhead;
		gcv< 72> bSplinePathType;	// 0: Strait line segments, 1: Spline
		gcv< 73, eLEADER_CREATION> eLeaderCreation;
		gcv< 74, eLEADER_HOOKLINE_DIRECTION> bHooklineDirection;
		gcv< 75> bHookline;
		gcv< 40> height;
		gcv< 41> width;
		gcv< 76> nVertex;
		lgcv<10, &this_t::nVertex> verticesX;
		lgcv<20, &this_t::nVertex> verticesY;
		lgcv<30, &this_t::nVertex> verticesZ;
		gcv< 77> color;	// if leader's DIMCLRD = BYBLOCK
		gcv<340> hReference;	// Hard reference to associated annotation (mtext, tolerance, or insert entity)
		mgcv<point_t, 210, 220, 230> vcNormal;
		mgcv<point_t, 211, 221, 231> vcHorizontal;
		mgcv<point_t, 212, 222, 232> ptOffsetFromInsertionPoint;
		mgcv<point_t, 213, 223, 233> ptOffsetFromAnnotationPlacement;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbLight {
		using this_t = sAcDbLight;
		constexpr static inline xStringLiteral marker{"AcDbLight"};
		gcv< 90> version;
		gcv<  1> name;
		gcv< 70, eLIGHT_TYPE> eLightType;
		gcv<290> status;
		gcv<291> plotGlyph;
		gcv< 40> intensity;
		mgcv<point_t, 10, 20, 30> pt;
		gcv_extrusion extrusion{{0., 0., 1.}};
		gcv< 72, eATTENUATION_TYPE> eAttenuation;
		gcv<292> bUseAttenuationLimit;
		gcv< 41> attenuationStartLimit;
		gcv< 42> attenuationEndLimit;
		gcv< 50> angleHotSpot;
		gcv< 51> angleFalloff;
		gcv<293> bCastShadows;
		gcv< 73, eSHADOW_TYPE> eShadowType;
		gcv< 91> shadowMapSize;
		gcv<280> shadowMapSoftness;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbLWPolyline {
		using this_t = sAcDbLWPolyline;
		struct sVertex {
			using this_t = sVertex;
			mgcv<point_t, 10, 20, 30> pt;
			gcv< 91> id;
			gcv< 40> widthStarting;
			gcv< 41> widthEnding;
			gcv< 42> bulge;
			DEFINE_SPACESHIP_OPERATOR(this_t);
		};
		constexpr static inline xStringLiteral marker{"AcDbPolyline"};
		gcv< 90> nVertex;
		gcv< 70, eLW_POLYLINE_FLAG> ePolylineFlag;
		gcv< 43> constantWidth{0.};
		gcv< 38> elevation{0.};
		gcv< 39> thickness{0.};
		TListVariable<sVertex, &this_t::nVertex> vertices;
		gcv_extrusion extrusion{{0., 0., 1.}};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbMesh {
		using this_t = sAcDbMesh;
		constexpr static inline xStringLiteral marker{"AcDbSubDMesh"};
		gcv< 71> version;
		gcv< 72> bBlendCrease;	// 0: off, 1: on
		gcv< 91> nSubdivisionLevel;
		gcv< 92> nVertexCountOfLevel0;
		TListVariable<mgcv<point_t, 10, 20, 30>, &this_t::nVertexCountOfLevel0> vertexPosition;
		gcv< 93> sizeOfFaceListOfLevel0;
		lgcv< 90, &this_t::sizeOfFaceListOfLevel0> faceListItem;
		gcv< 94> edgeCountOfLevel0;
		lgcv< 90, &this_t::edgeCountOfLevel0> vertexIndexOfEachEdge;
		gcv< 95> edgeCreaseCountOfLevel0;
		lgcv<140, &this_t::edgeCountOfLevel0> edgeCreateValue;
		gcv< 90> nSubEntityOverridden;
		gcv< 91> markerSubEntity;
		gcv< 92> nCountOfPropertyWasOverridden;
		TListVariable<gcv< 90, eMESH_PROPERTY_TYPE>, &this_t::nCountOfPropertyWasOverridden> eMeshPropertyType;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbMLine {
		using this_t = sAcDbMLine;
	public:
		constexpr static inline xStringLiteral marker{"AcDbMLine"};
		gcv<  2> styleName;									// MLINE style name
		gcv<340> hStyle;									// Hard pointer ID of MLINESTYLE object
		gcv< 40> scale{1.0};								// Scale factor
		gcv< 70, eMLINE_JUSTIFICATION> justification{};		// MLINE justification (0=Top, 1=Zero, 2=Bottom)
		gcv< 71, fMLINE_FLAGS> fFlags;
		gcv< 72> nVertex;
		gcv< 73> nMLineStyleElements;
		mgcv<point_t, 10, 20, 30> ptStart;
		gcv_extrusion extrusion{{0., 0., 1.}};
		lgcv<11, &this_t::nVertex> verticesX;
		lgcv<21, &this_t::nVertex> verticesY;
		lgcv<31, &this_t::nVertex> verticesZ;
		lgcv<12, &this_t::nVertex> vcsX;
		lgcv<22, &this_t::nVertex> vcsY;
		lgcv<32, &this_t::nVertex> vcsZ;
		lgcv<13, &this_t::nVertex> vcsMiterX;
		lgcv<23, &this_t::nVertex> vcsMiterY;
		lgcv<33, &this_t::nVertex> vcsMiterZ;

		template < int eCode_Count_, int eCode_Parameter_ >
		struct TElements {
			using this_t = TElements;
			constexpr static inline auto eCode_Count = eCode_Count_;
			constexpr static inline auto eCode_Parameter = eCode_Parameter_;
			gcv<eCode_Count> nCount;
			std::vector<gcv_t<eCode_Parameter>> params;
			DEFINE_SPACESHIP_OPERATOR(this_t);
		};

		TElements< 74, 41> param;
		TElements< 75, 42> areaFill;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbOleFrame {
	public:
		using this_t = sAcDbOleFrame;
	public:
		constexpr static inline xStringLiteral marker{"AcDbOleFrame"};
		gcv< 70> version;
		gcv< 90> dataLength;
		gcv<310> data;
		gcv<  1> strEnd{"OLE"};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbOle2Frame {
	public:
		using this_t = sAcDbOle2Frame;
	public:
		constexpr static inline xStringLiteral marker{"AcDbOle2Frame"};
		gcv< 70> version;
		gcv<  3> nData;
		mgcv<point_t, 10, 20, 30> ptUpperLeft;	// WCS
		mgcv<point_t, 11, 21, 31> ptLowerRight;	// WCS
		gcv< 71, eOLE_OBJECT_TYPE> eOLEObjectType;	// 1 = Link; 2 = Embedded; 3 = Static
		gcv< 72, eOLE_MODE> eOLEMode;	// 0 = Model space, 1 = Paper space
		gcv< 90> dataLength;
		lgcv<310, 0> data;	// binary data, multiple lines
		gcv<  1> strEnd{"OLE"};	// end of OLE data

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbPolyline {
		using this_t = sAcDbPolyline;
		bool b3d{};
		bool bPolyFaceMesh{};
		constexpr static inline xStringLiteral marker{""};
		gcv< 66> obsolete;
		mgcv<point_t, 10, 20, 30> elevation;	// z: Elevation. x, y: always 0. dummy point.
		gcv< 39> thickness;
		gcv< 70, fPOLYLINE_FLAGS> fFlags;
		gcv< 40> widthStart;
		gcv< 41> widthEnd;
		gcv< 71> nMVertex;
		gcv< 72> nNVetex;
		gcv< 73> smoothSurfaceMDensity;
		gcv< 74> smoothSurfaceNDensity;
		gcv< 75, ePOLYLINE_TYPE> eType;
		gcv_extrusion extrusion{{0., 0., 1.}};

		DEFINE_SPACESHIP_OPERATOR(this_t);

		bool PreRead(group_iter_t& iter) {
			if (iter->eCode != eGROUP_CODE::subclass) {
				return false;
			}
			static sGroup const group2dPolyline{eGROUP_CODE::subclass, "AcDb2dPolyline"s};
			static sGroup const group3dPolyline{eGROUP_CODE::subclass, "AcDb3dPolyline"s};
			static sGroup const groupPolyFaceMesh{eGROUP_CODE::subclass, "AcDbPolyFaceMesh"s};
			if (*iter == group2dPolyline) {
				iter++;
				b3d = false;
				return true;
			}
			else if (*iter == group3dPolyline) {
				iter++;
				b3d = true;
				return true;
			}
			else if (*iter == groupPolyFaceMesh) {
				iter++;
				bPolyFaceMesh = true;
				return true;
			}
			return false;
		}
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbRay {
	public:
		using this_t = sAcDbRay;
	public:
		constexpr static inline xStringLiteral marker{"AcDbRay"};
		mgcv<point_t, 10, 20, 30> pt0;
		mgcv<point_t, 11, 21, 31> pt1;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	////-----------------------------------------------------------------------------------------------------------------------------
	//struct sAcDbRegion {
	//public:
	//	using this_t = sAcDbRegion;
	//public:
	//	constexpr static inline xStringLiteral marker{"AcDbModelerGeometry"};
	//	gcv< 70> version;
	//	sgcv< 1, 3> proprietaryData;

	//	DEFINE_SPACESHIP_OPERATOR(this_t);
	//};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbSection {
		using this_t = sAcDbSection;
		constexpr static inline xStringLiteral marker{"AcDbSection"};
		gcv< 90> state;
		gcv< 91> flags;
		gcv<  1> name;
		mgcv<point_t, 10, 20, 30> vcVertical;
		gcv< 40> heightTop;
		gcv< 41> heightBottom;
		gcv< 70> indicatorTransparency;
		gcv< 63> indicatorColorIndex;
		gcv<411> indicatorColor;
		gcv< 92> nVertex;
		lgcv<11, &this_t::nVertex> verticesX;
		lgcv<21, &this_t::nVertex> verticesY;
		lgcv<31, &this_t::nVertex> verticesZ;
		gcv< 93> nVertexBackLine;
		lgcv<12, &this_t::nVertexBackLine> verticesBackLineX;
		lgcv<22, &this_t::nVertexBackLine> verticesBackLineY;
		lgcv<32, &this_t::nVertexBackLine> verticesBackLineZ;
		gcv<360> hGeometrySettingsObject;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbShape {
		using this_t = sAcDbShape;
		constexpr static inline xStringLiteral marker{"AcDbShape"};
		gcv< 39> thickness;
		mgcv<point_t, 10, 20, 30> insertionPoint;	// WCS
		gcv< 40> size;	// Size of the shape
		gcv<  2> name;
		gcv< 50> rotation;	// rad_t? deg_t?
		gcv< 41> scaleX;
		gcv< 51> scaleY;
		gcv_extrusion extrusion{{0., 0., 1.}};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbSpline {
		using this_t = sAcDbSpline;
		constexpr static inline xStringLiteral marker{"AcDbSpline"};
		mgcv<point_t, 210, 220, 230> vcNormal;	// in OCS (omitted if the spline is nonplanar)
		gcv< 70, fSPLINE_FLAGS> flags;
		gcv< 71> degree;
		gcv< 72> nKnot;
		gcv< 73> nControlPoint;
		gcv< 74> nFitPoint{0};
		gcv< 42> toleranceKnot{0.0000001};
		gcv< 43> toleranceControlPoint{0.0000001};
		gcv< 44> toleranceFit{0.0000000001};
		mgcv<point_t, 12, 22, 32> vcStartTangent;	// optional
		mgcv<point_t, 13, 23, 33> vcEndTangent;		// optional
		lgcv<40, &this_t::nKnot> knots;	// one entry per knot
		lgcv<41, 0> weight;	// optional (if not 1.); w/ multiple group pairs (if all are not 1)
		TListVariable<mgcv<point_t, 10, 20, 30>, &this_t::nControlPoint> controlPoints;	// in OCS
		TListVariable<mgcv<point_t, 11, 21, 31>, &this_t::nFitPoint> fitPoints;	// in OCS

		bool PreRead(group_iter_t& iter) {
			if (iter->eCode == 20) {	// vcNormal
				int i{};
			}
			return false;
		}

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbSun {
		using this_t = sAcDbSun;
		constexpr static inline xStringLiteral marker{"AcDbSun"};
		gcv< 90> version;
		gcv<290> status;
		gcv< 63> color;			// index
		gcv< 40> intensity;
		gcv<291> shadows;
		gcv< 91> julianDay;
		gcv< 92> time;	// in seconds past midnight
		gcv<292> bDaylightSavingsTime;
		gcv< 70, eSUN_SHADOW_TYPE> bShadowMaps;
		gcv< 71> shadowMapSize;
		gcv<280> shadowSoftness;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbExtrudedSurface {
		using this_t = sAcDbExtrudedSurface;

		constexpr static inline xStringLiteral marker2{"AcDbExtrudedSurface"};
		gcv< 90> classID;
		gcv< 90> nData;
		lgcv<310, 0> data;
		mgcv<point_t, 10, 20, 30> vcSweep;
		lgcv<40, 16> transformExtrudedEntity;	// Transform Matrix of extruded entity (16 reals, row major);
		gcv<42, rad_t> draftAngle;	// in radians
		gcv<43> draftStartDistance;
		gcv<44> draftEndDistance;
		gcv<45> twistAngle;
		gcv<48> scale;
		gcv<49, rad_t> alignAngle;	// in radians
		lgcv<46, 16> transformSweepEntity;	// Transform Matrix of sweep entity (16 reals, row major);
		lgcv<47, 16> transformPathEntity;	// Transform Matrix of path entity (16 reals, row major);
		gcv<290> fSolidFlag;
		gcv< 70, eSWEEP_ALIGNMENT> eSweepAlignment;
		gcv<292> fAlignStart;
		gcv<293> fBank;
		gcv<294> fBasePointSet;
		gcv<295> fSweepEntityTransformComputed;
		gcv<296> fPathEntityTransformComputed;
		mgcv<point_t, 11, 21, 31> vcControllingTwist;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	struct sAcDbLoftedSurface {
		using this_t = sAcDbLoftedSurface;
		constexpr static inline xStringLiteral marker{"AcDbLoftedSurface"};
		lgcv<40, 16> transformLoftEntity;	// Transform Matrix of loft entity (16 reals, row major)
		// entity data for cross sections
		// entity data for guide curves
		// entity data for path curces
		gcv< 70> planeNormalLoftingType;
		gcv< 41, rad_t> startDraftAngle;	// in radians
		gcv< 42, rad_t> endDraftAngle;		// in radians
		gcv< 43> startDraftMagnitude;
		gcv< 44> endDraftMagnitude;
		gcv<290> fArcLengthParameterization;
		gcv<291> fNoTwist;
		gcv<292> fAlignDirection;
		gcv<293> fCreateSimpleSurfaces;
		gcv<294> fCreateClosedSurface;
		gcv<295> fSolid;
		gcv<296> fCreateRuledSurface;
		gcv<297> fVirtualGuide;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	struct sAcDbRevolvedSurface {
		using this_t = sAcDbRevolvedSurface;
		constexpr static inline xStringLiteral marker{"AcDbRevolvedSurface"};
		gcv< 90> revolveEntityID;
		gcv< 90> nData;
		lgcv<310, 0> data;
		mgcv<point_t, 10, 20, 30> ptAxis;
		mgcv<point_t, 11, 21, 31> vcAxis;
		gcv< 40, rad_t> revolveAngle;	// in radians
		gcv< 41, rad_t> startAngle;	// in radians
		lgcv< 42, 16> transformRevolvedEntity; 	// Transform Matrix of revolved entity (16 reals, row major);
		gcv< 43, rad_t> draftAngle;
		gcv< 44> startDraftDistance;
		gcv< 45> endDraftDistance;
		gcv< 46, rad_t> twistAngle;
		gcv<290> fSolid;
		gcv<291> fCloseToAxis;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	struct sAcDbSweptSurface {
		using this_t = sAcDbSweptSurface;
		constexpr static inline xStringLiteral marker{"AcDbSweptSurface"};
		gcv< 90> sweepEntityID;
		gcv< 90> nData;
		lgcv<310, 0> data;
		gcv< 90> pathEntityID;
		gcv< 90> nProprietaryData;
		gcv<310> proprietaryData;
		lgcv<40, 16> transformSweepEntity;	// Transform Matrix of swept entity (16 reals, row major);
		lgcv<41, 16> transformPathEntity;	// Transform Matrix of path entity (16 reals, row major);
		gcv< 42, rad_t> draftAngle;
		gcv< 43> draftStartDistance;
		gcv< 44> draftEndDistance;
		gcv< 45> twistAngle;
		gcv< 48> scale;
		gcv< 49, rad_t> alignAngle;
		lgcv<46, 16> transformSweepEntity2;	// Transform Matrix of swept entity (16 reals, row major)
		lgcv<47, 16> transformPathEntity2;	// Transform Matrix of path entity (16 reals, row major)
		gcv<290> fSolidFlag;
		gcv< 70, eSWEEP_ALIGNMENT> eSweepAlignment;
		gcv<292> fAlignStart;
		gcv<293> fBank;
		gcv<294> fBasePointSet;
		gcv<295> fSweepEntityTransformComputed;
		gcv<296> fPathEntityTransformComputed;
		mgcv<point_t, 11, 21, 31> vcControllingTwist;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	struct sAcDbSurface {
		using this_t = sAcDbSurface;
		constexpr static inline xStringLiteral marker{"AcDbSurface"};
		gcv< 71> nU_isolines;
		gcv< 72> nV_isolines;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbTolerance {
		using this_t = sAcDbTolerance;
		constexpr static inline xStringLiteral marker{"AcDbFcf"};
		gcv<  3> dimensionStyleName;
		mgcv<point_t, 10, 20, 30> ptInsert;	// WCS
		gcv<  1> representation;
		gcv_extrusion extrusion{{0., 0., 1.}};
		mgcv<point_t, 11, 21, 31> vcXAxis; // WCS

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbTrace {
		using this_t = sAcDbTrace;
		constexpr static inline xStringLiteral marker{"AcDbTrace"};
		mgcv<point_t, 10, 20, 30> pt1;	// OCS, first corner	(for solid, WCS)
		mgcv<point_t, 11, 21, 31> pt2;	// OCS, second corner	(for solid, WCS)
		mgcv<point_t, 12, 22, 32> pt3;	// OCS, third corner	(for solid, WCS)
		mgcv<point_t, 13, 23, 33> pt4;	// OCS, 4th corner		(for solid, WCS)
		gcv< 39> thickness;
		gcv_extrusion extrusion{{0., 0., 1.}};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbUnderlay {
		using this_t = sAcDbUnderlay;
		//gcv<  0> objectName;	// DGNUNDERLAY, DWFUNDERLAY, PDFUNDERLAY
		constexpr static inline xStringLiteral marker{"AcDbUnderlayReference"};
		gcv<340> objectID;
		mgcv<point_t, 10, 20, 30> ptInsert;	// OCS/ECS
		mgcv<point_t, 41, 42, 43> scale;	// x, y, z scale factors
		gcv< 50, rad_t> rotation;	// in radians
		mgcv<point_t, 210, 220, 230> vcNormal;	// WCS
		gcv<280, fUNDERLAY_FLAGS> fFlags;
		gcv<281> contrast;	// 20~100
		gcv<282> fade;		// 0~80
		lgcv<11, 0> insertionPointX;	// OCS/ECS,	If only two, then they are the lower left and upper right corner points of a clip rectangle.
										//			If more than two, then they are the vertices of a clipping polygon
		lgcv<21, 0> insertionPointY;	// OCS/ECS,	If only two, then they are the lower left and upper right corner points of a clip rectangle.
										//			If more than two, then they are the vertices of a clipping polygon

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbVertex {
		using this_t = sAcDbVertex;
		constexpr static inline xStringLiteral marker{"AcDbVertex"};
		gcv<100> markerSpecific;	// AcDb2dVertex or AcDb3dPolylineVertex
		mgcv<point_t, 10, 20, 30> pt;	// WCS
		gcv< 40> widthStart;
		gcv< 41> widthEnd;
		gcv< 42> bulge;
		gcv< 70, fVERTEX_FLAGS> fFlags;
		gcv< 50> curveFitTangentDirection;
		gcv< 71> polyfaceMeshVertexIndex;
		gcv< 72> polyfaceMeshVertexIndex2;
		gcv< 73> polyfaceMeshVertexIndex3;
		gcv< 74> polyfaceMeshVertexIndex4;
		gcv< 91> vertexIdentifier;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbViewport {
		using this_t = sAcDbViewport;
		constexpr static inline xStringLiteral marker{"AcDbViewport"};
		mgcv<point_t, 10, 20, 30> ptCenterWCS;
		gcv< 40> width;		// in paper space units
		gcv< 41> height;	// in paper space units
		gcv< 68> eViewportStatus;	// -1 : on (but is fully offscreen, or is one of the viewports that is not active because the $MAXDACTVP count is currently being exceeded.
		// 0 : off
		// > 0 : on and active. stacking order. 1 : active, 2: the next, and so forth
		gcv< 69> viewportID;
		mgcv<point_t, 12, 22> ptCenterDCS;
		mgcv<point_t, 13, 23> ptSnap;
		mgcv<point_t, 14, 24> snapSpacing;
		mgcv<point_t, 15, 25> gridSpacing;
		mgcv<point_t, 16, 26, 36> vcViewDirection;	// WCS
		mgcv<point_t, 17, 27, 37> ptViewTarget;		// WCS
		gcv< 42> lensLength;
		gcv< 43> frontClipPlaneZ;
		gcv< 44> backClipPlaneZ;
		gcv< 45> heightView;		// in model space units
		gcv< 50> snapAngle;
		gcv< 51> viewTwistAngle;
		gcv< 72> circleZoomPercent;
		lgcv<331, 0> frozenLayerObjectID_Handle;	// multiple entries may exist
		gcv< 90, fVIEWPORT_FLAGS> fStatusBits;
		gcv<340> hClippingBoundary;					// hard-pointer ID/Handle to entity that serves as the viewport's clipping boundary (only preset if viewport is non-rectanglular)
		gcv<  1> plotStyleSheetName;
		gcv<281, eVIEWPORT_RENDER> eRenderMode;
		gcv< 71, eUCS_PER_VIEWPORT> eUCSPerViewport;
		gcv< 74> bDisplayUCSIcon;
		mgcv<point_t, 110, 120, 130> ptUCS;
		mgcv<point_t, 111, 121, 131> vcXAxis;

		mgcv<point_t, 112, 122, 132> vcYAxis;
		gcv<345> hAcDbUCSTableRecord;
		gcv<346> hAcDbUCSTableRecordBaseUCS;
		gcv< 79, eORTHOGRAPHIC_TYPE> eOrthographicType;
		gcv<146> elevation;
		gcv<170, eSHADE_PLOT> eShadePlot;
		gcv< 61> frequencyMajorGridLines;	// Compared to minor grid lines;
		gcv<332> backgroundID;
		gcv<333> shadePlotID;
		gcv<348> visualStyleID;
		gcv<292> defaultLightingFlag;
		gcv<282, eLIGHTING_TYPE> eDefaultLightingType;
		gcv<141> viewBrightness;
		gcv<142> viewContrast;
		gcv< 63> ambientLightColor0;
		gcv<421> ambientLightColor1;
		gcv<431> ambientLightColor2;
		gcv<361> sunID;
		gcv<335> pViewportObject0;
		gcv<343> pViewportObject1;
		gcv<344> pViewportObject2;
		gcv< 91> pViewportObject3;

		gcv_extrusion extrusion{{0., 0., 1.}};

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbWipeout {
		using this_t = sAcDbWipeout;
		constexpr static inline xStringLiteral marker{""};	// "AcDbRasterImage" or "AcDbWipeout"
		gcv< 90> version;
		mgcv<point_t, 10, 20, 30> ptInsert;			// in WCS
		mgcv<point_t, 11, 21, 31> vcUSinglePixel;	// in WCS
		mgcv<point_t, 12, 22, 32> vcVSinglePixel;	// in WCS
		mgcv<point_t, 13, 23> imageSizeInPixel;		// U, V
		gcv<340> pImageDefObject;					// hard reference to imagedef object
		gcv< 70, fIMAGE_DISPLAY_FLAGS> fImageDisplay;
		gcv<280> bClipping;
		gcv<281> brightness{50};	// 0 ~ 100
		gcv<282> contrast{50};		// 0 ~ 100
		gcv<283> fade{0};			// 0 ~ 100
		gcv<290> a;	// not in documents
		gcv<360> pImageDefReactor;	// hard reference to imagedef_reactor object
		gcv< 71, eCLIPPING_BOUNDARY> eClippingBoundary{};
		gcv< 91> nClipBoundaryVertex;
		lgcv<14, &this_t::nClipBoundaryVertex> verticesClipBoundaryX;
		lgcv<24, &this_t::nClipBoundaryVertex> verticesClipBoundaryY;

		DEFINE_SPACESHIP_OPERATOR(this_t);

		bool PreRead(group_iter_t& iter) {
			if (iter->eCode != eGROUP_CODE::subclass) {
				return false;
			}
			static sGroup const groupRasterImage{eGROUP_CODE::subclass, "AcDbRasterImage"s};
			static sGroup const groupWipeOut{eGROUP_CODE::subclass, "AcDbWipeout"s};
			if (*iter == groupRasterImage) {
				iter++;
				return true;
			}
			else if (*iter == groupWipeOut) {
				iter++;
				return true;
			}
			return false;
		}
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	struct sAcDbXLine {
		using this_t = sAcDbXLine;
		constexpr static inline xStringLiteral marker{"AcDbXline"};
		mgcv<point_t, 10, 20, 30> pt0;
		mgcv<point_t, 11, 21, 31> pt1;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};

};

