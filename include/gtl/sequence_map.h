#pragma once

//////////////////////////////////////////////////////////////////////
//
// sequence_map.h: Sequence Map
//
// PWH
// 2023-11-06
//
//////////////////////////////////////////////////////////////////////

#include "sequence.h"

namespace gtl::seq::inline v01 {

	using unit_id_t = std::string;

	//-------------------------------------------------------------------------
	/// @brief sequence map manager
	template < typename tSelf >
	class xSequenceHandlerMap {
	public:
		struct sParam {
			std::string in, out;
		};

		using this_t = xSequenceHandlerMap;
		using handler_t = std::function<xSequence&(std::shared_ptr<sParam>)>;
		using map_t = std::map<seq_id_t, handler_t>;

	protected:
		unit_id_t m_unit;
		this_t* m_top{};
		this_t* m_parent{};
		std::set<this_t*> m_children;

		map_t m_mapFuncs;
	public:
		// constructors and destructor
		xSequenceHandlerMap(this_t* parent) : m_parent(parent) {
			if (m_parent) {
				m_parent->Register(this);
				m_top = parent->m_top;
			}
		}
		~xSequenceHandlerMap() {
			if (auto* parent = std::exchange(m_parent, nullptr))
				parent->Unregister(this);
		}
		xSequenceHandlerMap(xSequenceHandlerMap const&) = delete;
		xSequenceHandlerMap& operator = (xSequenceHandlerMap const&) = delete;
		xSequenceHandlerMap(xSequenceHandlerMap&& b) {
			if (this == &b)
				return ;

			if (auto* parent = std::exchange(b.m_parent, nullptr)) {
				parent->Unregister(&b);
				m_parent = parent;
				m_top = parent->m_top;
			}
			m_unit = std::exchange(b.m_unit, {});
			m_top = std::exchange(b.m_top, nullptr);
			m_children.swap(b.m_children);

			if (m_parent)
				m_parent->Register(this);
		}
		xSequenceHandlerMap& operator = (xSequenceHandlerMap&&) = delete;	// if has some children, no way to remove children from parent

		//-----------------------------------
		void Register(this_t* child) {
			if (child)
				m_children.insert(child);
		}
		void Unregister(this_t* child) {
			if (child)
				m_children.erase(child);
		}

		//-----------------------------------
		// Find Handler
		inline handler_t FindHandler(seq_id_t const& sequence) const {
			if (auto iter = m_mapFuncs.find(sequence); iter != m_mapFuncs.end())
				return iter->second;
			return nullptr;
		}
		handler_t FindHandlerDFS(seq_id_t const& sequence) const {
			if (auto handler = FindHandler(sequence))
				return handler;
			for (auto* child : m_children) {
				if (auto func = child->FindHandlerDFS(sequence))
					return func;
			}
			return nullptr;
		}

		//-----------------------------------
		// Find Map
		inline auto FindMapDFS(this auto&& self, seq_id_t const& unit) -> decltype(&self) {
			if (m_unit == unit)
				return const_cast<this_t*>(this);
			for (auto* child : self.m_children) {
				if (auto* map = child->FindMapDFS(unit))
					return map;
			}
			return nullptr;
		}

		inline xSequence& CreateChildSequence(xSequence& parent, seq_id_t name, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			if (auto handler = FindHandler(name)) {
				return parent.CreateChildSequence<std::shared_ptr<sParam>>(std::move(name), handler, std::move(params));
			}
			throw std::exception(std::format("no handler: {}", name).c_str());
		}

		xSequence& CreateChildSequence(unit_id_t unit, seq_id_t name, std::shared_ptr<sParam> params = std::make_shared<sParam>()) {
			if (auto* map = FindMapDFS(unit)) {
				map->CreateChildSequence(std::move(name), std::move(params));
			}
		}

	};


	//-------------------------------------------------------------------------
	/// @brief sequence wrapper, with sequence map
	template < typename tSelf >
	class TSequence {
	public:
		using this_t = TSequence;
		using self_t = tSelf;
		using seq_unit_t = std::string;

		struct sParam {
			std::string in, out;
		};

		template < typename ... targs >
		using tseq_handler_t = std::function<xSequence(self_t*, targs&& ...)>;
		using seq_handler_t = tseq_handler_t<std::shared_ptr<sParam>>;

	public:
		//struct sAwaitable {
		//	bool await_ready() { return false; }
		//	bool await_suspend(handle_t h) {
		//		return false;
		//	}
		//	void await_resume() { }
		//};
	protected:
		seq_unit_t m_name;
		xSequence& m_driver;

	public:
		//-----------------------------------
		TSequence(xSequence& driver, seq_unit_t name) : m_driver(driver), m_name{ std::move(name) } {}
		~TSequence() {}
		TSequence(TSequence const&) = delete;
		TSequence& operator = (TSequence const&) = delete;
		TSequence(TSequence&& b) = default;
		TSequence& operator = (TSequence&& b) = default;

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

};
