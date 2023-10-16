#include "pch.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/util.h"
#include "gtl/win/EnvironmentVariable.h"
#include "test_qt.h"

#include "glaze/glaze.hpp"

#include <QSettings>

QSettings reg("Biscuit-lab.com", "gtl::test_qt");

struct sJson1 {
	std::string str2{"test 스트링2"};
	GLZ_LOCAL_META(sJson1, str2);
};
struct sJsonTest {
	std::string str{"test 스트링"};
	int i{3}, j{4};
	double d{5.6};
	bool b{true};
	std::array<int, 10> arr{{1,2,3,4,5,6,7,8,9,10}};
	sJson1 s1;

	GLZ_LOCAL_META(sJsonTest, str, i, j, d, b, arr, s1);
};

gtl::qt::test_qt::test_qt(QWidget *parent)
    : QMainWindow(parent), m_completer(this), m_modelGlaze(this)
{
    ui.setupUi(this);
	//ui.groupBox->Collapse(true);
	//ui.groupBox->PrepareAnimation(300ms);

    //m_dlgMatViewGV = std::make_unique<gtl::qt::xMatViewGVDlg>(this);
    //m_dlgMatViewGV->show();
	m_completer.Init({});
	ui.edtPath->setCompleter(&m_completer);
	auto strPath = reg.value("misc/LastImagePath").toString();
	ui.edtPath->setText(strPath);

	ui.treeView->setModel(&m_modelGlaze);

	{
		glz::json_t j{};
		sJsonTest test;
		std::string str;
		//auto j = glz::write_json(test);
		glz::write<glz::opts{}>(test, str);
		glz::read_json(j, str);
		m_modelGlaze.SetJson(j);
	}

	//m_ctrlMatView = std::make_unique<gtl::qt::xMatView>(this);
	//m_ctrlMatView->move({4, 100});
	//m_ctrlMatView->resize({1200, 500});
	//m_ctrlMatView->show();

	m_dlgMatView = std::make_unique<gtl::qt::xMatViewDlg>(this);
	//m_dlgMatView->show();
	m_dlgMatView->GetView().m_fnSyncSetting = [this](bool bStore, std::string_view cookie, xMatView::S_OPTION& option) -> bool {
		if (bStore) {
			std::string buffer = glz::write_json(option);
			reg.setValue("misc/viewOption", ToQString(buffer));
		}
		else {
			auto str = reg.value("misc/viewOption").toString();
			if (str.isEmpty())
				return false;
			auto buffer = ToString(str);
			glz::read_json(option, buffer);
		}
		return true;
	};
	m_dlgMatView->GetView().LoadOption();

	// Color Bar
	//cv::Mat img = cv::Mat::zeros(1080*2, 1920*2, CV_8UC3);
	cv::Mat img = cv::Mat::zeros(1800, 600, CV_8UC3);
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

		if (CheckGPU(true)) {
			gtl::ResizeImage(img, img, 2.0);
		}
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
	ui.view->SetImage(img, false);
	//ui.view->SetImage({});

	connect(ui.btnOpenImage, &QPushButton::clicked, this, &this_t::OnLoadImage);
	connect(ui.edtPath, &QLineEdit::returnPressed, this, &this_t::OnLoadImage);
	connect(ui.btnSetEnvVar, &QPushButton::clicked, this, &this_t::OnSetEnvVar);

	gtl::qt::LoadWindowPosition(reg, "test_qt", this);
}

gtl::qt::test_qt::~test_qt() {
	gtl::qt::SaveWindowPosition(reg, "test_qt", this);
}

void gtl::qt::test_qt::OnLoadImage() {
	//ui.groupBox->PrepareAnimation(1s);

	std::filesystem::path path = ui.edtPath->text().toStdWString();
	if (path.empty())
		return;

	if (!gtl::IsImageExtension(path))
		return ;

	xWaitCursor wc;
	cv::Mat img;
	bool bLoadBitmapMatTRIED{};
	if (path.extension() == L".bmp") {
		auto [result, fileHeader, header] = gtl::LoadBitmapHeader(path);
		if (result) {
			auto [w, h] = std::visit([](auto& arg) { return std::pair<int32_t, int32_t>(arg.width, arg.height); }, header);
			if (w < 0) w = -w;
			if (h < 0) h = -h;
			if ((uint64_t)w * h > 32767 * 32767) {
				gtl::xSize2i pelsPerMeter;
				bLoadBitmapMatTRIED = true;
				img = gtl::LoadBitmapMat(path, pelsPerMeter);
			}
		}
	}
	if (img.empty())
		img = gtl::LoadImageMat(path);
	if (img.empty() and (path.extension() == L".bmp") and !bLoadBitmapMatTRIED) {
		gtl::xSize2i pelsPerMeter;
		img = gtl::LoadBitmapMat(path, pelsPerMeter);
	}
	if (img.empty())
		return ;
	if (img.channels() == 3) {
		cv::cvtColor(img, img, cv::ColorConversionCodes::COLOR_BGR2RGB);
	} else if (img.channels() == 4) {
		cv::cvtColor(img, img, cv::ColorConversionCodes::COLOR_BGRA2RGBA);
	}
	auto str = ToQString(path);
	if (!m_dlgMatView->GetView().SetImage(img, true, xMatView::eZOOM::fit2window))
		return;
	reg.setValue("misc/LastImagePath", ToQString(path));
	m_dlgMatView->show();

}

void gtl::qt::test_qt::OnSetEnvVar() {
	using namespace gtl::win;
	{
		xEnvironmentVariable var(xEnvironmentVariable::eSCOPE::CURRENT_USER);
		if (!var.Set(L"OPENCV_IO_MAX_IMAGE_PIXELS", std::format(L"{}", 0x01ull << 40)))
			return;
		auto r = var.GetAll();
		var.Broadcast();
	}
	{
		xEnvironmentVariable var(xEnvironmentVariable::eSCOPE::LOCAL_MACHINE);
		if (!var.Set(L"OPENCV_IO_MAX_IMAGE_PIXELS", std::format(L"{}", 0x01ull << 40)))
			return;
		auto r = var.GetAll();
		var.Broadcast();
	}
}
