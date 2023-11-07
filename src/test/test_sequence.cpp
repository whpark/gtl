#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/string.h"
#include "gtl/sequence.h"
#include "gtl/sequence_map.h"

using namespace std::literals;
using namespace gtl::literals;

namespace seq = gtl::seq::v01;

gtl::seq::xSequence g_driver("g_driver"s);

TEST(gtl_sequence, normal) {

	class Task1 : public seq::TSequenceMap<std::string> {
	public:
		using this_t = Task1;
		using base_t = seq::TSequenceMap<std::string>;

		Task1() : base_t("Task1", g_driver) {
		}
	};

}
