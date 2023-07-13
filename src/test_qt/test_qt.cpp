#include "pch.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/util.h"
#include "test_qt.h"

#include <QSettings>

QSettings s("Biscuit-lab.com", "gtl::test_qt");

gtl::qt::test_qt::test_qt(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    //m_dlgMatViewGV = std::make_unique<gtl::qt::xMatViewGVDlg>(this);
    //m_dlgMatViewGV->show();

	gtl::qt::SaveWindowPosition(s, "test_qt", this);

	m_ctrlMatView = std::make_unique<gtl::qt::xMatView>(this);
	m_ctrlMatView->move({4, 100});
	m_ctrlMatView->show();

	m_dlgMatView = std::make_unique<gtl::qt::xMatViewDlg>(this);
	m_dlgMatView->show();

	// Color Bar
	//cv::Mat img = cv::Mat::zeros(1080*2, 1920*2, CV_8UC3);
	cv::Mat img = cv::Mat::zeros(800, 600, CV_8UC3);
	{
		auto cy = img.rows/3;
		auto cy2 = cy*2;
		auto v = (img.depth() == CV_8U) ? 255. : 1.;
		auto a = v / cy;
		for (int y{}; y < cy; y++) {
			double v = a*(y+1.);
			//if (y&1)
			//	continue;
			img.row(y) = cv::Scalar(v, 0, 0, 0);
			img.row(y+cy) = cv::Scalar(0, v, 0, 255);
			img.row(y+cy2) = cv::Scalar(0, 0, v, 255);
		}
		for (int x{}; x < img.cols; x++) {
			double v = (x+1.)/img.cols;
			img.col(x) *= (x+1.)/img.cols;
		}
		img.col(0) = cv::Scalar(v, v, v);
		img.col(img.cols-1) = cv::Scalar(v, v, v);
		img.row(0) = cv::Scalar(v, v, v);
		img.row(img.rows-1) = cv::Scalar(v, v, v);
	}
	// Color Bar
	//img = cv::Mat::zeros(1080*2, 1920*2, CV_32SC3);
	//{
	//	auto cy = img.rows/3;
	//	auto cy2 = cy*2;
	//	for (int y{}; y < cy; y++) {
	//		double v = y*(std::numeric_limits<int>::max()/cy);//(y+1.)/cy;
	//		img.row(y) = cv::Scalar(v, 0, 0);
	//		img.row(y+cy) = cv::Scalar(0, v, 0);
	//		img.row(y+cy2) = cv::Scalar(0, 0, v);
	//	}
	//}
	//m_option.bGLonly = false;
	//m_option.bSkia = true;

	m_dlgMatView->GetView().SetImage(img, false);
	//m_dlgMatViewGV->SetImage(img, false);
}

gtl::qt::test_qt::~test_qt() {
	gtl::qt::SaveWindowPosition(s, "test_qt", this);
}
