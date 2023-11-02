#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/string.h"
#include "gtl/sequence.h"

using namespace std::literals;
using namespace gtl::literals;

namespace seq = gtl::seq::v01;

TEST(gtl_sequence, normal) {
	class Task1 : public seq::TSequence<Task1> {
	public:
		using this_t = Task1;
		using base_t = seq::TSequence<Task1>;

		Task1() : base_t("Task1") {
		}
	};

}