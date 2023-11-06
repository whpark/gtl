#pragma once

//////////////////////////////////////////////////////////////////////
//
// sequence_map.h: Sequence Map.
//
// dependency : glaze
// 
// PWH
// 2023-11-06
//
//////////////////////////////////////////////////////////////////////

#include "glaze/glaze.hpp"
#include "sequence.h"

namespace gtl::seq::inline v01 {

	//-------------------------------------------------------------------------
	/// @brief sequence map (unit tree, sequence function map) manager
	template < typename tJson = glz::json_t >
	class TSequenceMap {
	public:
		using this_t = TSequenceMap;
		using unit_id_t = std::string;
		using json_t = tJson;
		using sequence_t = xSequence;

	public:
		using this_t = TSequenceMap;
		struct sParam {
			json_t in, out;
		};
		using param_t = std::shared_ptr<sParam>;
		using handler_t = std::function<xSequence(param_t)>;
		using map_t = std::map<seq_id_t, handler_t>;

	protected:
		unit_id_t m_unit;
		this_t* m_top{};
		xSequence* m_driver{};
		this_t* m_parent{};
		std::set<this_t*> m_children;

		map_t m_mapFuncs;
	public:
		// constructors and destructor
		TSequenceMap(unit_id_t unit, this_t& parent) : m_unit(std::move(unit)), m_parent(&parent), m_top(parent.m_top), m_driver(parent.m_driver) {
			m_parent->Register(this);
		}
		TSequenceMap(unit_id_t unit, xSequence& driver) : m_unit(std::move(unit)), m_driver(&driver), m_top(this) {
		}
		~TSequenceMap() {
			if (auto* parent = std::exchange(m_parent, nullptr))
				parent->Unregister(this);
		}
		TSequenceMap(TSequenceMap const&) = delete;
		TSequenceMap& operator = (TSequenceMap const&) = delete;
		TSequenceMap(TSequenceMap&& b) {
			if (this == &b)
				return;

			if (auto* parent = std::exchange(b.m_parent, nullptr)) {
				parent->Unregister(&b);
				m_parent = parent;
				m_top = parent->m_top;
			}
			m_unit = std::exchange(b.m_unit, {});
			m_top = std::exchange(b.m_top, nullptr);
			m_driver = std::exchange(b.m_driver, nullptr);
			m_children.swap(b.m_children);

			if (m_parent)
				m_parent->Register(this);
		}
		TSequenceMap& operator = (TSequenceMap&&) = delete;	// if has some children, no way to remove children from parent

		//-----------------------------------
		void Register(this_t* child) {
			if (child) {
				m_children.insert(child);
			}
		}
		void Unregister(this_t* child) {
			if (child) {
				m_children.erase(child);
			}
		}

		//-----------------------------------
		void Connect(seq_id_t const& id, handler_t handler) {
			m_mapFuncs[id] = handler;
		}
		void Disconnect(seq_id_t const& id) {
			if (auto iter = m_mapFuncs.find(id); iter != m_mapFuncs.end())
				m_mapFuncs.erase(iter);
		}
		template < typename tSelf, typename self_handler_t = std::function<xSequence(tSelf* self, std::shared_ptr<typename base_t::sParam>)> >
		void Connect(seq_id_t const& id, self_handler_t handler, tSelf* self) {
			Connect(id, std::bind(handler, self, std::placeholders::_1));
		}

		//-----------------------------------
		// Find Handler
		inline handler_t FindHandler(seq_id_t const& sequence) const {
			if (auto iter = m_mapFuncs.find(sequence); iter != m_mapFuncs.end())
				return iter->second;
			return nullptr;
		}

		//-----------------------------------
		// Find Map
		inline auto FindUnitDFS(this auto&& self, seq_id_t const& unit) -> decltype(&self) {
			if (self.m_unit == unit)
				return &self;
			for (auto* child : self.m_children) {
				if (auto* unitTarget = child->FindUnitDFS(unit))
					return unitTarget;
			}
			return nullptr;
		}

		inline xSequence& CreateRootSequence(seq_id_t name, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			return CreateChildSequence(*m_driver, std::move(name), std::move(params));
		}
		inline xSequence& CreateRootSequence(unit_id_t const& unit, seq_id_t name, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			if (auto* unitTarget = m_top->FindUnitDFS(unit)) {
				unitTarget->CreateChildSequence(*m_driver, std::move(name), std::move(params));
			}
			return nullptr;
		}

		inline xSequence& CreateChildSequence(xSequence& parent, seq_id_t name, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			if (auto handler = FindHandler(name)) {
				return parent.CreateChildSequence<std::shared_ptr<sParam>>(std::move(name), handler, std::move(params));
			}
			throw std::exception(std::format("no handler: {}", name).c_str());
		}
		inline xSequence& CreateChildSequence(seq_id_t name, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			xSequence* parent = xSequence::GetCurrentSequence();
			if (!parent)
				throw std::exception("CreateChildSequence() must be called from sequence function");
			return CreateChildSequence(*parent, std::move(name), std::move(params));
		}
		xSequence* CreateChildSequence(unit_id_t const& unit, seq_id_t name, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			if (auto* unitTarget = m_top->FindUnitDFS(unit)) {
				unitTarget->CreateChildSequence(std::move(name), std::move(params));
			}
			return nullptr;
		}

		size_t BroadcastSequence(xSequence& parent, seq_id_t name, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			size_t count{};
			if (auto handler = FindHandler(name)) {
				parent.CreateChildSequence<std::shared_ptr<sParam>>(name, handler, std::move(params));
				count++;
			}
			for (auto* child : m_children) {
				count += child->BroadcastSequence(parent, name, params);
			}
			return count;
		}

	};


#if 0
	//-------------------------------------------------------------------------
	/// @brief sequence wrapper, with sequence map
	template < typename tSelf >
	class TSequenceWrapper {
	public:
		using this_t = TSequenceWrapper;
		using self_t = tSelf;
		using seq_unit_t = std::string;

		struct sParam {
			std::string in, out;
		};

		template < typename ... targs >
		using tseq_handler_t = std::function<xSequence(self_t*, targs&& ...)>;
		using seq_handler_t = tseq_handler_t<std::shared_ptr<sParam>>;

	protected:
		seq_unit_t m_name;
		xSequence& m_driver;

	public:
		//-----------------------------------
		TSequenceWrapper(xSequence& driver, seq_unit_t name) : m_driver(driver), m_name{ std::move(name) } {}
		~TSequenceWrapper() {}
		TSequenceWrapper(TSequenceWrapper const&) = delete;
		TSequenceWrapper& operator = (TSequenceWrapper const&) = delete;
		TSequenceWrapper(TSequenceWrapper&& b) = default;
		TSequenceWrapper& operator = (TSequenceWrapper&& b) = default;

		//-----------------------------------
		// Helper functions
		auto const& GetName() const { return m_name; }

		// Create Child Sequence
		inline xSequence& CreateChildSequence(xSequence& seqParent, seq_unit_t name, seq_handler_t funcSequence, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			tSelf* self = static_cast<tSelf*>(this);
			return seqParent.CreateChildSequence<tSelf*, std::shared_ptr<sParam>>(std::move(name), funcSequence, self, std::move(params));
		}
		inline xSequence& CreateChildSequence(seq_unit_t name, seq_handler_t funcSequence, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			auto* seqParent = m_driver.GetCurrentSequence();
			if (!seqParent)
				throw std::exception("CreateChildSequence() must be called from sequence function");
			return CreateChildSequence(*seqParent, std::move(name), funcSequence, std::move(params));
		}
	};
#endif

};
