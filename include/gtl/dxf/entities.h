#pragma once

#include "gtl/dxf/entities_subclass.h"

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::dxf::entities {

	//=============================================================================================================================
	enum class eENTITY {
		none,
		unknown,
		_3dface,
		_3dsolid,
		acad_proxy_entity,
		arc,
		attdef,
		attrib,
		body,
		circle,
		dimension,
		ellipse,
		hatch,
		helix,
		image,
		insert,
		leader,
		light,
		line,
		lw_polyline,
		mesh,
		mline,
		mleader_style,
		mleader,
		mtext,
		mtext_attdef,
		ole_frame,
		ole2_frame,
		point,
		polyline,
		ray,
		region,
		section,
		seqend,
		shape,
		solid,
		spline,
		sun,
		surface,
		table,
		text,
		tolerance,
		trace,
		underlay,
		vertex,
		viewport,
		wipeout,
		xline,
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	template < eGROUP_CODE code = eGROUP_CODE::control >
	struct TControlData {
		using this_t = TControlData;
		constexpr static inline auto enclosure{code};
		std::string name{};
		std::vector<sGroup> data{};

		DEFINE_SPACESHIP_OPERATOR(this_t)
	};

	using sControl = TControlData<eGROUP_CODE::control>;
	using sXData = TControlData<eGROUP_CODE::xdata>;

	//=============================================================================================================================

	//-------------------------------------------------------------------------
	struct sEntity {
		using this_t = sEntity;

		gcv<  5> m_handle{};
		gcv<330> m_hOwnerBlock;
		sAcDbEntity m_entity;
		sXData xdata;
		std::vector<sControl> controls;

		DEFINE_SPACESHIP_OPERATOR(this_t);
		//bool operator == (sEntity const& r) const {
		//	return m_handle == r.m_handle
		//		and m_hOwnerBlock == r.m_hOwnerBlock
		//		and m_entity == r.m_entity
		//		and xdata == r.xdata
		//		and controls == r.controls;
		//}
		//std::partial_ordering operator <=> (sEntity const& r) const {
		//	if (auto e = m_handle <=> r.m_handle; e != 0)
		//		return e;
		//	if (auto e = m_hOwnerBlock <=> r.m_hOwnerBlock; e != 0)
		//		return e;
		//	if (auto e = m_entity <=> r.m_entity; e != 0)
		//		return e;
		//	if (auto e = xdata <=> r.xdata; e != 0)
		//		return e;
		//	if (auto e = controls <=> r.controls; e != 0)
		//		return e;
		//	return std::partial_ordering::equivalent;
		//}
	};
	class xEntity : public sEntity {
	public:
		using root_t = xEntity;
		using this_t = xEntity;

	public:
		xEntity() = default;
	protected:
		xEntity(xEntity const&) = default;
		xEntity(xEntity&&) = default;
		xEntity& operator=(xEntity const&) = default;
		xEntity& operator=(xEntity&&) = default;
	public:
		virtual ~xEntity() = default;

		DEFINE_SPACESHIP_OPERATOR(this_t);

		virtual std::unique_ptr<xEntity> clone() const = 0;
		virtual bool IsEqual(xEntity const& other) const = 0;
		virtual bool CopyTo(xEntity& other) const = 0;
		virtual eENTITY GetEntityType() const = 0;

		virtual bool ReadSubclasses(group_iter_t& iter) = 0;

		bool Read(group_iter_t& iter) {
			auto const iter0 = iter;
			for (; iter; ) {
				if (iter->eCode == eGROUP_CODE::entity)
					return true;

				if (!ReadFieldMembers((sEntity&)*this, iter))
					return false;

				if (/*iter->eCode == eGROUP_CODE::subclass
					and */!ReadSubclasses(iter)
					)
				{
					break;
				}
			}
			return iter0 != iter;
		}

	public:
		static std::unique_ptr<xEntity> CreateEntity(string_t const& name);
	private:
		// map of entity factory. don't make static inline variable. (static variable initializing order is NOT stable)
		//inline static std::map<string_t, std::function<std::unique_ptr<xEntity>()>> m_mapEntityFactory;
		static auto& GetEntityFactory() {
			static std::map<string_t, std::function<std::unique_ptr<xEntity>()>> mapEntityFactory;
			return mapEntityFactory;
		}
	protected:
		class xRegisterEntity {
		public:
			xRegisterEntity(std::string const& name, std::function<std::unique_ptr<xEntity>()> fnCreate) {
				GetEntityFactory()[name] = std::move(fnCreate);
			}
		};

	};

	using entity_ptr_t = TCloneablePtr<xEntity>;
	using entities_t = std::deque<entity_ptr_t>;


	//============================================================================================================================
	template < eENTITY eEntity, xStringLiteral NAME, typename tSubclasses >
	class TEntityDerived final : public xEntity, public tSubclasses {
	public:
		using base_t = xEntity;
		using this_t = TEntityDerived;

	public:
		constexpr static inline eENTITY m_eEntity = eEntity;

	public:
		TEntityDerived() = default;
		TEntityDerived(TEntityDerived const&) = default;
		TEntityDerived(TEntityDerived&&) = default;
		TEntityDerived& operator=(TEntityDerived const&) = default;
		TEntityDerived& operator=(TEntityDerived&&) = default;
		virtual ~TEntityDerived() = default;
	public:
		DEFINE_SPACESHIP_OPERATOR(this_t);

		xEntity& BaseEntity() { return static_cast<xEntity&>(*this); }
		xEntity const& BaseEntity() const { return static_cast<xEntity const&>(*this); }
		tSubclasses& Subclasses() { return static_cast<tSubclasses&>(*this); }
		tSubclasses const& Subclasses() const { return static_cast<tSubclasses const&>(*this); }
		eENTITY GetEntityType() const override { return m_eEntity; }
		std::unique_ptr<this_t::root_t> clone() const override {
			return std::make_unique<this_t>(*(this_t*)this);
		}
		static std::unique_ptr<this_t::root_t> create() { return std::make_unique<this_t>(); }
		bool IsEqual(xEntity const& other) const override {
			//if (!base_t::IsEqual(other))
			//	return false;
			if (m_eEntity != other.GetEntityType())
				return false;
			if (auto const* p = dynamic_cast<this_t const*>(&other))
				return *this == *p;
			return false;
		}
		bool CopyTo(xEntity& other) const override {	// 'CopyTo' copies only to SAME type
			if (GetEntityType() != other.GetEntityType())
				return false;
			(this_t&)other = *this;
			return true;
		}

	private:
		static inline xRegisterEntity s_registerEntity{NAME.str, &this_t::create};
	public:
		bool ReadSubclasses(group_iter_t& iter) override {
			//if constexpr (requires (tSubclasses subclasses) { tSubclasses::PreRead; }) {
			//	if (tSubclasses::PreRead(iter))
			//		return true;
			//}
			if constexpr (CountStructMember<tSubclasses>() > 0) {
				return ReadFieldMembers(Subclasses(), iter);
			}
			return true;
		}

	};


	//============================================================================================================================
	// xUnknown
	struct sUnknown {
		using this_t = sUnknown;
		string_t m_name;
		std::vector<sGroup> m_groups;

		DEFINE_SPACESHIP_OPERATOR(this_t);

		bool PreRead(group_iter_t& iter) {
			bool bFound{};
			for (; iter and iter->eCode != eGROUP_CODE::entity; iter++) {
				bFound = true;
				m_groups.push_back(*iter);
			}
			return bFound;
		}

	};
	using xUnknown =	TEntityDerived<eENTITY::unknown, "UNKNOWN", sUnknown>;

	//=============================================================================================================================
	inline std::unique_ptr<xEntity> xEntity::CreateEntity(string_t const& name) {
		auto const& map = GetEntityFactory();
		if (auto iter = map.find(name); iter != map.end() and iter->second)
			return iter->second();
		auto entity = std::make_unique<xUnknown>();
		entity->m_name = name;
		return entity;
	}

}	// namespace gtl::dxf::entities

namespace gtl::dxf::entities {

	//-----------------------------------------------------------------------------------------------------------------------------
	template < typename tSubclass >
	struct TSubclass {
		using this_t = TSubclass;
		tSubclass m_field;

		DEFINE_SPACESHIP_OPERATOR(this_t)
	};

	//-----------------------------------------------------------------------------------------------------------------------------

	ENTITY_DERIVED(x3DFace,			eENTITY::_3dface, "3DFACE", TSubclass<sAcDb3DFace>);

	struct s3DSolid {
		using this_t = s3DSolid;

		sAcDbModelerGeometry m_geometry;
		sAcDb3DSolid m_field;
		DEFINE_SPACESHIP_OPERATOR(this_t);
	};
	ENTITY_DERIVED(x3DSolid,		eENTITY::_3dsolid, "3DSOLID", s3DSolid);

	ENTITY_DERIVED(xACADProxyEntity, eENTITY::acad_proxy_entity, "ACAD_PROXY_ENTITY", TSubclass<sAcDbProxyEntity>);

	struct sArc {
		using this_t = sArc;
		sAcDbCircle m_circle;	// circle is subclass of arc
		sAcDbArc m_field;

		DEFINE_SPACESHIP_OPERATOR(this_t);
	};
	ENTITY_DERIVED(xArc,			eENTITY::arc, "ARC", sArc);

	template < typename T >
	struct TAttrib {
		using this_t = TAttrib;
		sAcDbText m_text;
		T m_field;
		sAcDbXRecord m_xrec;
		bool m_bMText{};
		sAcDbEntity m_entityMText;
		sAcDbMText m_mtext;

		DEFINE_SPACESHIP_OPERATOR(this_t)

		bool PreRead(group_iter_t& iter) {
			static sGroup const groupMText{eGROUP_CODE::entity, "MTEXT"s};
			if (*iter != groupMText or m_bMText)
				return false;
			iter++;
			m_bMText = true;
			if (!ReadFieldMembers(m_entityMText, iter))
				return false;
			if (!ReadFieldMembers(m_mtext, iter))
				return false;
			return true;
		}
	};
	ENTITY_DERIVED(xAttDef,			eENTITY::attdef,		"ATTDEF",		TAttrib<sAcDbAttributeDefinition>);
	ENTITY_DERIVED(xAttrib,			eENTITY::attrib,		"ATTRIB",		TAttrib<sAcDbAttribute>);
	ENTITY_DERIVED(xBody,			eENTITY::body,			"BODY",			TSubclass<sAcDbModelerGeometry>);
	ENTITY_DERIVED(xCircle,			eENTITY::circle,		"CIRCLE",		TSubclass<sAcDbCircle>);
	ENTITY_DERIVED(xEllipse,		eENTITY::ellipse,		"ELLIPSE",		TSubclass<sAcDbEllipse>);
	ENTITY_DERIVED(xHatch,			eENTITY::hatch,			"HATCH",		TSubclass<sAcDbHatch>);
	ENTITY_DERIVED(xHelix,			eENTITY::helix,			"HELIX",		TSubclass<sAcDbHelix>);
	ENTITY_DERIVED(xImage,			eENTITY::image,			"IMAGE",		TSubclass<sAcDbImage>);
	ENTITY_DERIVED(xInsert,			eENTITY::insert,		"INSERT",		TSubclass<sAcDbBlockReference>);
	ENTITY_DERIVED(xLeader,			eENTITY::leader,		"LEADER",		TSubclass<sAcDbLeader>);
	ENTITY_DERIVED(xLight,			eENTITY::light,			"LIGHT",		TSubclass<sAcDbLight>);
	ENTITY_DERIVED(xLine,			eENTITY::line,			"LINE",			TSubclass<sAcDbLine>);
	ENTITY_DERIVED(xLWPolyline,		eENTITY::lw_polyline,	"LWPOLYLINE",	TSubclass<sAcDbLWPolyline>);
	ENTITY_DERIVED(xMesh,			eENTITY::mesh,			"MESH",			TSubclass<sAcDbMesh>);
	//ENTITY_DERIVED(xMLeader,			eENTITY::mleader,		"MLEADER",		TSubclass<sAcDbMLeader>);
	//ENTITY_DERIVED(xMLeaderStyle,		eENTITY::mleader_style,	"MLEADERSTYLE",	TSubclass<sAcDbMLeaderStyle>);
	ENTITY_DERIVED(xMText,			eENTITY::mtext,			"MTEXT",		TSubclass<sAcDbMText>);
	ENTITY_DERIVED(xOleFrame,		eENTITY::ole_frame,		"OLEFRAME",		TSubclass<sAcDbOleFrame>);
	ENTITY_DERIVED(xOle2Frame,		eENTITY::ole2_frame,	"OLE2FRAME",	TSubclass<sAcDbOle2Frame>);
	ENTITY_DERIVED(xPoint,			eENTITY::point,			"POINT",		TSubclass<sAcDbPoint>);
	ENTITY_DERIVED(xPolyline,		eENTITY::polyline,		"POLYLINE",		TSubclass<sAcDbPolyline>);
	ENTITY_DERIVED(xRay,			eENTITY::ray,			"XRAY",			TSubclass<sAcDbRay>);
	ENTITY_DERIVED(xRegion,			eENTITY::region,		"REGION",		TSubclass<sAcDbModelerGeometry>);
	ENTITY_DERIVED(xSection,		eENTITY::section,		"SECTION",		TSubclass<sAcDbSection>);
	ENTITY_DERIVED(xShape,			eENTITY::shape,			"SHAPE",		TSubclass<sAcDbShape>);
	ENTITY_DERIVED(xSolid,			eENTITY::solid,			"SOLID",		TSubclass<sAcDbTrace>);	// NOT Solid, but Trace
	ENTITY_DERIVED(xSpline,			eENTITY::spline,		"SPLINE",		TSubclass<sAcDbSpline>);
	ENTITY_DERIVED(xSun,			eENTITY::sun,			"SUN",			TSubclass<sAcDbSun>);

	struct sSurface {
		using this_t = sSurface;
		sAcDbModelerGeometry m_geometry;
		sAcDbSurface m_field;
		std::variant<sAcDbExtrudedSurface, sAcDbLoftedSurface, sAcDbRevolvedSurface, sAcDbSweptSurface> m_surface;

		bool operator == (this_t const&) const = default;
		auto operator <=> (this_t const&) const = default;

		bool PreRead(group_iter_t& iter) {
			if (!iter)
				return false;
			static sGroup const groupExtrudedSurface{eGROUP_CODE::subclass, "AcDbExtrudedSurface"s};
			static sGroup const groupLoftedSurface{eGROUP_CODE::subclass, "AcDbLoftedSurface"s};
			static sGroup const groupRevolvedSurface{eGROUP_CODE::subclass, "AcDbRevolvedSurface"s};
			static sGroup const groupSweptSurface{eGROUP_CODE::subclass, "AcDbSweptSurface"s};

			if (*iter == groupExtrudedSurface) {
				m_surface = sAcDbExtrudedSurface{};
				return ReadFieldMembers(std::get<sAcDbExtrudedSurface>(m_surface), iter);
			}
			if (*iter == groupLoftedSurface) {
				m_surface = sAcDbLoftedSurface{};
				return ReadFieldMembers(std::get<sAcDbLoftedSurface>(m_surface), iter);
			}
			if (*iter == groupRevolvedSurface) {
				m_surface = sAcDbRevolvedSurface{};
				ReadFieldMembers(std::get<sAcDbRevolvedSurface>(m_surface), iter);
				return true;
			}
			if (*iter == groupSweptSurface) {
				m_surface = sAcDbSweptSurface{};
				ReadFieldMembers(std::get<sAcDbSweptSurface>(m_surface), iter);
				return true;
			}
			return false;
		}
	};
	ENTITY_DERIVED(xSurface,		eENTITY::surface,		"SURFACE",		sSurface);

	//ENTITY_DERIVED(xTable,			eENTITY::table,			"TABLE",		TSubclass<sAcDbTable>);
	ENTITY_DERIVED(xText,			eENTITY::text,			"TEXT",			TSubclass<sAcDbText>);
	ENTITY_DERIVED(xTolerance,		eENTITY::tolerance,		"TOLERANCE",	TSubclass<sAcDbTolerance>);
	ENTITY_DERIVED(xTrace,			eENTITY::trace,			"TRACE",		TSubclass<sAcDbTrace>);
	ENTITY_DERIVED(xUnderlay,		eENTITY::underlay,		"UNDERLAY",		TSubclass<sAcDbUnderlay>);
	ENTITY_DERIVED(xDGNUnderlay,	eENTITY::underlay,		"DGNUNDERLAY",	TSubclass<sAcDbUnderlay>);
	ENTITY_DERIVED(xDWFUnderlay,	eENTITY::underlay,		"DWFUNDERLAY",	TSubclass<sAcDbUnderlay>);
	ENTITY_DERIVED(xPDFUnderlay,	eENTITY::underlay,		"PDFUNDERLAY",	TSubclass<sAcDbUnderlay>);
	ENTITY_DERIVED(xVertex,			eENTITY::vertex,		"VERTEX",		TSubclass<sAcDbVertex>);
	ENTITY_DERIVED(xViewport,		eENTITY::viewport,		"VIEWPORT",		TSubclass<sAcDbViewport>);
	ENTITY_DERIVED(xWipeout,		eENTITY::wipeout,		"WIPEOUT",		TSubclass<sAcDbWipeout>);
	ENTITY_DERIVED(xXLine,			eENTITY::xline,			"XLINE",		TSubclass<sAcDbXLine>);

};	// namespace gtl::dxf::entities
