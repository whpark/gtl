#include "pch.h"

#include "LayeredMatViewDlg.h"

#include "gtl/shape/shape.h"

#include <opencv2/imgproc.hpp>

namespace {

cv::Mat MakeCheckerboard() {
	cv::Mat image(240, 320, CV_8UC3);
	constexpr int tile = 32;
	for (int y{}; y < image.rows; ++y) {
		for (int x{}; x < image.cols; ++x) {
			auto const light = ((x / tile) + (y / tile)) % 2 == 0;
			image.at<cv::Vec3b>(y, x) = light ? cv::Vec3b{210, 210, 210} : cv::Vec3b{70, 70, 70};
		}
	}
	cv::putText(image, "image 0", {20, 45}, cv::FONT_HERSHEY_SIMPLEX, 1.0, {40, 40, 255}, 2, cv::LINE_AA);
	return image;
}

cv::Mat MakeColorKeyImage(cv::Vec3b key, cv::Scalar color, std::string_view label) {
	cv::Mat image(130, 210, CV_8UC3, cv::Scalar(key[0], key[1], key[2]));
	cv::rectangle(image, {10, 10}, {200, 120}, color, 5, cv::LINE_AA);
	cv::circle(image, {105, 65}, 48, color, cv::FILLED, cv::LINE_AA);
	cv::putText(image, label.data(), {28, 73}, cv::FONT_HERSHEY_SIMPLEX, .55, {255, 255, 255}, 2, cv::LINE_AA);
	return image;
}

cv::Mat MakeAlphaImage() {
	cv::Mat image(150, 220, CV_8UC4, cv::Scalar(0, 0, 0, 0));
	// OpenCV stores CV_8UC4 as BGRA. The fourth channel is used directly by xLayeredMatView.
	cv::rectangle(image, {10, 10}, {210, 140}, {40, 220, 255, 90}, cv::FILLED, cv::LINE_AA);
	cv::circle(image, {110, 75}, 58, {255, 80, 40, 170}, cv::FILLED, cv::LINE_AA);
	cv::putText(image, "BGRA alpha", {45, 82}, cv::FONT_HERSHEY_SIMPLEX, .55, {255, 255, 255, 230}, 2, cv::LINE_AA);
	return image;
}

std::shared_ptr<gtl::shape::xDrawing> MakeDrawing() {
	using namespace gtl::shape;

	auto drawing = std::make_shared<xDrawing>();
	auto* layer = new xLayer(L"xLayeredMatView test overlay");

	auto addLine = [&](point_t from, point_t to, color_t color) {
		auto* line = new xLine;
		line->m_pt0 = from;
		line->m_pt1 = to;
		line->m_color = color;
		line->m_lineWeight = 2;
		layer->m_shapes.push_back(line);
	};

	auto const cyan = gtl::ColorRGBA(0, 255, 255);
	addLine({0, 0, 0}, {320, 0, 0}, cyan);
	addLine({320, 0, 0}, {320, 240, 0}, cyan);
	addLine({320, 240, 0}, {0, 240, 0}, cyan);
	addLine({0, 240, 0}, {0, 0, 0}, cyan);
	addLine({0, 0, 0}, {320, 240, 0}, gtl::ColorRGBA(255, 80, 80));
	addLine({320, 0, 0}, {0, 240, 0}, gtl::ColorRGBA(255, 80, 80));

	auto* circle = new xCircle;
	circle->m_ptCenter = {160, 120, 0};
	circle->m_radius = 85;
	circle->m_color = gtl::ColorRGBA(80, 255, 80);
	circle->m_lineWeight = 2;
	layer->m_shapes.push_back(circle);

	drawing->m_layers.push_back(layer);
	return drawing;
}

} // namespace

xLayeredMatViewDlg::xLayeredMatViewDlg(QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);
	Populate();
	auto option = ui.display->GetOption();
	option.bPanningLock = false;
	ui.display->SetOption(option);

	connect(ui.btnAdd, &QPushButton::clicked, this, &xLayeredMatViewDlg::AddImage);
	connect(ui.btnDelete, &QPushButton::clicked, this, [this] {
		if (!ui.display->Empty())
			ui.display->Delete(ui.display->Count() - 1);
	});
	connect(ui.btnFront, &QPushButton::clicked, this, [this] {
		if (!ui.display->Empty())
			ui.display->BringToFront(0);
	});
	connect(ui.btnBack, &QPushButton::clicked, this, [this] {
		if (!ui.display->Empty())
			ui.display->SendToBack(ui.display->Count() - 1);
	});
}

xLayeredMatViewDlg::~xLayeredMatViewDlg() = default;

void xLayeredMatViewDlg::Populate() {
	cv::Mat img1 = MakeCheckerboard();
	ui.display->Add(img1, {-260, -150});

	// Each CV_8UC3 layer has its own BGR transparent color.
	cv::Vec3b const brownKey{70, 35, 20};
	cv::Vec3b const magentaKey{255, 0, 255};
	ui.display->Add(MakeColorKeyImage(brownKey, {60, 200, 255}, "BGR key 1"), {-205, -105}, brownKey);
	ui.display->Add(MakeColorKeyImage(magentaKey, {80, 230, 80}, "BGR key 2"), {-35, -15}, magentaKey);

	// CV_8UC4 needs no color key; its per-pixel alpha channel is applied automatically.
	ui.display->Add(MakeAlphaImage(), {-115, -55});
	ui.display->Add(MakeDrawing(), {-260, -150 + img1.rows});
	ui.display->FitToWindow();
}

void xLayeredMatViewDlg::AddImage() {
	cv::Mat image(90, 140, CV_8UC3, cv::Scalar(40 + (m_addedImageCount * 35) % 180, 80, 180));
	cv::putText(image, std::format("added {}", m_addedImageCount), {12, 52}, cv::FONT_HERSHEY_SIMPLEX, .55, {255, 255, 255}, 1, cv::LINE_AA);
	auto const offset = static_cast<double>(m_addedImageCount * 24);
	ui.display->Add(image, {-50 + offset, -30 + offset});
	++m_addedImageCount;
}
