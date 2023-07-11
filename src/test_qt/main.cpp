#include "pch.h"
#include "test_qt.h"

int main(int argc, char* argv[]) {
	QString str(" \t \r\n 0xffff");
	auto t = gtl::qt::ToArithmeticValue<int>(str);
	QString str2(" \t \r\n 0x1234567890ABC");
	auto t2 = gtl::qt::ToArithmeticValue<int64_t>(str2);

	QApplication a(argc, argv);
	test_qt w;
	w.show();
	return a.exec();
}
