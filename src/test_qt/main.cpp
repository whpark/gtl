#include "pch.h"
#include "test_qt.h"

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	test_qt w;
	w.show();
	return a.exec();
}
