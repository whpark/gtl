#include "pch.h"
#include "test_qt.h"

int main(int argc, char* argv[]) {
	QString str(" \t \r\n 0xffff");
	auto t = gtl::qt::ToArithmeticValue<int>(str);
	QString str2(" \t \r\n 0x1234567890ABC");
	auto t2 = gtl::qt::ToArithmeticValue<int64_t>(str2);


	QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	fmt.setVersion(4, 6);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	fmt.setRedBufferSize(8);
	fmt.setGreenBufferSize(8);
	fmt.setBlueBufferSize(8);
	fmt.setDepthBufferSize(8);
	fmt.setAlphaBufferSize(8);
	fmt.setStencilBufferSize(0);
	fmt.setStereo(false);
	fmt.setSamples(0); // we never need multisampling in the context since the FBO can support
	// multisamples independently

	QSurfaceFormat::setDefaultFormat(fmt);


	QApplication a(argc, argv);
	gtl::qt::test_qt w;
	w.show();
	return a.exec();
}
