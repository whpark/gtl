#include "pch.h"

#include "gtl/qt/MatView/LayeredMatView.h"
#include "ui_LayeredMatView.h"
#include "gtl/mat_helper.h"
#include "gtl/shape/shape.h"
#include "../MatView/MatViewSettingsDlg.h"

#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPointF>
#include <QResizeEvent>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace {

QPointF ToQPoint(gtl::xPoint2d point) { return {point.x, point.y}; }
gtl::xPoint2d ToPoint(QPointF point) { return {point.x(), point.y()}; }

std::optional<cv::Rect> VisibleImageRoi(cv::Mat const& image, gtl::xPoint2d position, QRectF const& visibleWorld) {
	if (image.empty())
		return {};
	auto const visible = QRectF(position.x, position.y, image.cols, image.rows).intersected(visibleWorld);
	if (visible.isEmpty())
		return {};

	auto const x0 = std::clamp(static_cast<int>(std::floor(visible.left() - position.x)), 0, image.cols);
	auto const y0 = std::clamp(static_cast<int>(std::floor(visible.top() - position.y)), 0, image.rows);
	auto const x1 = std::clamp(static_cast<int>(std::ceil(visible.right() - position.x)), 0, image.cols);
	auto const y1 = std::clamp(static_cast<int>(std::ceil(visible.bottom() - position.y)), 0, image.rows);
	if (x1 <= x0 || y1 <= y0)
		return {};
	return cv::Rect(x0, y0, x1 - x0, y1 - y0);
}

class xLayeredMatViewCanvasMat final : public gtl::shape::xCanvasMat {
public:
	xLayeredMatViewCanvasMat(cv::Mat& image, gtl::xCoordTrans2d const& transform) : xCanvasMat(image, transform), m_scale(transform.m_scale) {
		m_target_interpolation_inverval = m_scale;
	}

	void PreDraw(gtl::shape::xShape const& shape) override {
		if (shape.GetShapeType() == gtl::shape::eSHAPE::drawing)
			m_drawingVisible = shape.m_bVisible;
		else if (shape.GetShapeType() == gtl::shape::eSHAPE::layer)
			m_layerVisible = m_drawingVisible && shape.m_bVisible;

		if (!m_drawingVisible || !m_layerVisible || !shape.m_bVisible)
			m_color = cv::Scalar(0, 0, 0, 0);
		else
			m_color = cv::Scalar(shape.m_color.b, shape.m_color.g, shape.m_color.r, shape.m_color.a ? shape.m_color.a : 255);
		m_line_width = std::max(1, shape.m_lineWeight);
	}

	void LineTo_Target(gtl::shape::point_t const& point) override {
		if (m_color[3] != 0)
			xCanvasMat::LineTo_Target(point);
	}

	void Text(gtl::shape::xText const& text) override {
		if (m_color[3] == 0)
			return;
		auto const point = Trans(text.m_pt0);
		auto const scale = std::max(.25, text.m_height * m_scale / 30.0);
		cv::putText(m_img, QString::fromStdWString(text.m_text).toStdString(), cv::Point(cvRound(point.x), cvRound(point.y)),
			cv::FONT_HERSHEY_SIMPLEX, scale, m_color, static_cast<int>(m_line_width), cv::LINE_AA);
	}

	void Text(gtl::shape::xMText const& text) override { Text(static_cast<gtl::shape::xText const&>(text)); }

private:
	double m_scale{1.0};
	bool m_drawingVisible{true};
	bool m_layerVisible{true};
};

int InterpolationFor(double zoom, gtl::qt::xLayeredMatView::S_OPTION const& option) {
	if (zoom >= 1.0) {
		switch (option.eZoomIn) {
		case gtl::qt::xLayeredMatView::eZOOM_IN::linear: return cv::INTER_LINEAR;
		case gtl::qt::xLayeredMatView::eZOOM_IN::bicubic: return cv::INTER_CUBIC;
		case gtl::qt::xLayeredMatView::eZOOM_IN::lanczos4: return cv::INTER_LANCZOS4;
		default: return cv::INTER_NEAREST;
		}
	}
	return option.eZoomOut == gtl::qt::xLayeredMatView::eZOOM_OUT::area ? cv::INTER_AREA : cv::INTER_NEAREST;
}

void AlphaComposite(cv::Mat& destination, cv::Mat const& source) {
	CV_Assert(destination.type() == CV_8UC4 && source.type() == CV_8UC4 && destination.size() == source.size());
	for (int y{}; y < destination.rows; ++y) {
		auto* dst = destination.ptr<cv::Vec4b>(y);
		auto const* src = source.ptr<cv::Vec4b>(y);
		for (int x{}; x < destination.cols; ++x) {
			auto const alpha = src[x][3] / 255.0;
			for (int channel{}; channel < 3; ++channel)
				dst[x][channel] = cv::saturate_cast<uchar>(src[x][channel] * alpha + dst[x][channel] * (1.0 - alpha));
			dst[x][3] = 255;
		}
	}
}

} // namespace

namespace gtl::qt {

gtl::xPoint2d xLayeredMatView::Item::Position() const noexcept {
	return std::visit([](auto const& item) { return item.position; }, content);
}

void xLayeredMatView::Item::SetPosition(gtl::xPoint2d position) noexcept {
	std::visit([position](auto& item) { item.position = position; }, content);
}

xLayeredMatView::xLayeredMatView(QWidget* parent) : QWidget(parent), m_ui(std::make_unique<Ui::LayeredMatViewClass>()) {
	m_ui->setupUi(this);
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	connect(m_ui->cmbZoomMode, &QComboBox::currentIndexChanged, this, [this](int index) {
		SetZoomMode(static_cast<eZOOM>(index));
	});
	connect(m_ui->spinZoom, &QDoubleSpinBox::valueChanged, this, [this](double value) {
		SetZoom(value * .01, ViewCenter());
	});
	connect(m_ui->btnZoomIn, &QPushButton::clicked, this, [this] { ZoomInOut(true, ViewCenter()); });
	connect(m_ui->btnZoomOut, &QPushButton::clicked, this, [this] { ZoomInOut(false, ViewCenter()); });
	connect(m_ui->btnZoomFit, &QPushButton::clicked, this, [this] { FitToWindow(); });
	connect(m_ui->btnSettings, &QPushButton::clicked, this, &xLayeredMatView::ShowSettingsDialog);
	connect(this, &xLayeredMatView::ViewChanged, this, [this] { SyncToolbar(); });
	SetOption(m_option);
	SyncToolbar();
}

xLayeredMatView::~xLayeredMatView() = default;

xLayeredMatView::index_t xLayeredMatView::Add(cv::Mat const& image, gtl::xPoint2d position, bool copy,
	std::optional<cv::Vec3b> transparentColor) {
	if (image.empty())
		return npos;
	if (image.type() != CV_8UC1 && image.type() != CV_8UC3 && image.type() != CV_8UC4)
		return npos;
	if (transparentColor && image.type() != CV_8UC3)
		return npos;

	Image item{copy ? image.clone() : image, {}, position, transparentColor};
	m_items.push_back(Item{std::move(item)});
	NotifyItemsChanged();
	return m_items.size() - 1;
}

xLayeredMatView::index_t xLayeredMatView::Add(cv::Mat const& image, gtl::xPoint2d position,
	cv::Vec3b transparentColor, bool copy) {
	return Add(image, position, copy, transparentColor);
}

xLayeredMatView::index_t xLayeredMatView::Add(gtl::shape::xDrawing const& drawing, gtl::xPoint2d position) {
	return Add(std::make_shared<gtl::shape::xDrawing>(drawing), position);
}

xLayeredMatView::index_t xLayeredMatView::Add(std::shared_ptr<gtl::shape::xDrawing> drawing, gtl::xPoint2d position) {
	if (!drawing)
		return npos;
	m_items.push_back(Item{Drawing{std::move(drawing), position}});
	NotifyItemsChanged();
	return m_items.size() - 1;
}

bool xLayeredMatView::Delete(index_t index) {
	if (index >= m_items.size())
		return false;
	m_items.erase(m_items.begin() + index);
	NotifyItemsChanged();
	return true;
}

void xLayeredMatView::Clear() {
	if (m_items.empty())
		return;
	m_items.clear();
	m_fitOnFirstContent = true;
	NotifyItemsChanged();
}

bool xLayeredMatView::SetPosition(index_t index, gtl::xPoint2d position) {
	if (index >= m_items.size())
		return false;
	m_items[index].SetPosition(position);
	NotifyItemsChanged();
	return true;
}

bool xLayeredMatView::SetVisible(index_t index, bool visible) {
	if (index >= m_items.size())
		return false;
	m_items[index].visible = visible;
	NotifyItemsChanged();
	return true;
}

bool xLayeredMatView::SetPalette(index_t index, cv::Mat const& palette, bool updateView) {
	if (index >= m_items.size())
		return false;
	auto* image = std::get_if<Image>(&m_items[index].content);
	if (!image)
		return false;
	if (!palette.empty() && (palette.type() != CV_8UC1 && palette.type() != CV_8UC3))
		return false;
	if (!palette.empty() && palette.total() != 256)
		return false;
	image->palette = palette.clone();
	if (updateView)
		update();
	return true;
}

bool xLayeredMatView::SetTransparentColor(index_t index, std::optional<cv::Vec3b> transparentColor, bool updateView) {
	if (index >= m_items.size())
		return false;
	auto* image = std::get_if<Image>(&m_items[index].content);
	if (!image || (transparentColor && image->mat.type() != CV_8UC3))
		return false;
	image->transparentColor = transparentColor;
	if (updateView)
		update();
	return true;
}

bool xLayeredMatView::Move(index_t from, index_t to) {
	if (from >= m_items.size() || to >= m_items.size())
		return false;
	if (from != to) {
		auto item = std::move(m_items[from]);
		m_items.erase(m_items.begin() + from);
		m_items.insert(m_items.begin() + to, std::move(item));
		NotifyItemsChanged();
	}
	return true;
}

bool xLayeredMatView::BringToFront(index_t index) { return m_items.empty() ? false : Move(index, m_items.size() - 1); }
bool xLayeredMatView::SendToBack(index_t index) { return Move(index, 0); }
bool xLayeredMatView::MoveForward(index_t index) { return index + 1 < m_items.size() && Move(index, index + 1); }
bool xLayeredMatView::MoveBackward(index_t index) { return index > 0 && index < m_items.size() && Move(index, index - 1); }

QRect xLayeredMatView::ViewRect() const noexcept {
	auto rect = QWidget::rect();
	if (m_ui && m_ui->toolbar->isVisible())
		rect.setTop(m_ui->toolbar->geometry().bottom() + 1);
	return rect;
}

gtl::xPoint2d xLayeredMatView::ViewCenter() const noexcept {
	auto const rect = ViewRect();
	return {rect.left() + rect.width() / 2.0, rect.top() + rect.height() / 2.0};
}

bool xLayeredMatView::SetZoomMode(eZOOM mode, bool center) {
	m_zoomMode = mode;
	auto const bounds = SceneBounds();
	if (bounds.isEmpty())
		return false;

	switch (mode) {
	case eZOOM::one2one:
		m_zoom = 1.0;
		if (center)
			m_pan = ToPoint(-bounds.center());
		break;
	case eZOOM::fit2window:
		FitToWindow();
		return true;
	case eZOOM::fit2width:
		m_zoom = std::clamp(std::max(1.0, ViewRect().width() - 40.0) / bounds.width(), 1.0 / 8192.0, 1000.0);
		if (center)
			m_pan = ToPoint(-bounds.center() * m_zoom);
		break;
	case eZOOM::fit2height:
		m_zoom = std::clamp(std::max(1.0, ViewRect().height() - 40.0) / bounds.height(), 1.0 / 8192.0, 1000.0);
		if (center)
			m_pan = ToPoint(-bounds.center() * m_zoom);
		break;
	case eZOOM::none:
	case eZOOM::mouse_wheel_locked:
	case eZOOM::free:
		break;
	}
	update();
	emit ViewChanged(m_zoom, m_pan);
	return true;
}

void xLayeredMatView::SetZoom(double zoom, gtl::xPoint2d anchor) {
	zoom = std::clamp(zoom, 1.0 / 8192.0, 1000.0);
	if (anchor == gtl::xPoint2d{})
		anchor = ViewCenter();
	auto const worldAnchor = MapToWorld(anchor);
	m_zoom = zoom;
	m_zoomMode = eZOOM::free;
	m_pan = anchor - ViewCenter() - worldAnchor * m_zoom;
	update();
	emit ViewChanged(m_zoom, m_pan);
}

extern GTL__QT_CLASS std::vector<double> const s_dZoomLevels;

void xLayeredMatView::ZoomInOut(bool bZoomIn, xPoint2i ptAnchor) {
	double scale = m_zoom;
	if (bZoomIn) {
		if (auto iter = std::ranges::upper_bound(s_dZoomLevels, scale); iter != s_dZoomLevels.end()) {
			scale = *iter;
			SetZoom(scale, ptAnchor);
		}
	}
	else {
		if (auto iter = std::ranges::lower_bound(s_dZoomLevels, scale); iter != s_dZoomLevels.begin()) {
			--iter;
			scale = *iter;
			SetZoom(scale, ptAnchor);
		}
	}
}

void xLayeredMatView::SetPan(gtl::xPoint2d pan) {
	m_pan = pan;
	update();
	emit ViewChanged(m_zoom, m_pan);
}

void xLayeredMatView::PanBy(gtl::xPoint2d delta) { SetPan(m_pan + delta); }

void xLayeredMatView::FitToWindow(double margin) {
	auto bounds = SceneBounds();
	auto const viewRect = ViewRect();
	if (bounds.isEmpty() || viewRect.width() <= 0 || viewRect.height() <= 0)
		return;
	auto const availableWidth = std::max(1.0, viewRect.width() - 2.0 * margin);
	auto const availableHeight = std::max(1.0, viewRect.height() - 2.0 * margin);
	m_zoom = std::clamp(std::min(availableWidth / bounds.width(), availableHeight / bounds.height()), 1.0 / 8192.0, 1000.0);
	m_pan = ToPoint(-bounds.center() * m_zoom);
	m_fitOnFirstContent = false;
	m_zoomMode = eZOOM::fit2window;
	update();
	emit ViewChanged(m_zoom, m_pan);
}

void xLayeredMatView::ResetView() {
	m_zoom = 1.0;
	m_zoomMode = eZOOM::one2one;
	m_pan = {};
	m_fitOnFirstContent = false;
	update();
	emit ViewChanged(m_zoom, m_pan);
}

gtl::xPoint2d xLayeredMatView::MapToWorld(gtl::xPoint2d viewportPosition) const noexcept {
	return (viewportPosition - ViewCenter() - m_pan) / m_zoom;
}

gtl::xPoint2d xLayeredMatView::MapFromWorld(gtl::xPoint2d worldPosition) const noexcept {
	return ViewCenter() + m_pan + worldPosition * m_zoom;
}

QRectF xLayeredMatView::SceneBounds() const {
	QRectF bounds;
	bool first = true;
	for (auto const& item : m_items) {
		if (!item.visible)
			continue;
		QRectF itemBounds;
		if (auto const* image = std::get_if<Image>(&item.content)) {
			itemBounds = QRectF(ToQPoint(image->position), QSizeF(image->mat.cols, image->mat.rows));
		} else if (auto const* drawing = std::get_if<Drawing>(&item.content); drawing && drawing->drawing) {
			auto const rect = drawing->drawing->GetBoundary();
			if (std::isfinite(rect.left) && std::isfinite(rect.top) && std::isfinite(rect.right) && std::isfinite(rect.bottom))
				itemBounds = QRectF(
					QPointF(drawing->position.x + rect.left, drawing->position.y - rect.bottom),
					QPointF(drawing->position.x + rect.right, drawing->position.y - rect.top)
				).normalized();
		}
		if (!itemBounds.isValid())
			continue;
		bounds = first ? itemBounds : bounds.united(itemBounds);
		first = false;
	}
	return bounds;
}

std::optional<std::pair<gtl::xPoint2d, gtl::xPoint2d>> xLayeredMatView::GetSelectionPoints() const noexcept {
	return m_selection;
}

void xLayeredMatView::SetSelectionRect(gtl::xRect2d const& rect) {
	m_selection = std::pair{rect.pt0(), rect.pt1()};
	update();
	emit SelectionChanged(rect);
}

void xLayeredMatView::ClearSelectionRect() {
	m_selection.reset();
	update();
}

void xLayeredMatView::SetBackgroundColor(QColor color) {
	m_background = color;
	m_option.crBackground = cv::Vec3b(color.red(), color.green(), color.blue());
	update();
}

void xLayeredMatView::SetDrawPixelValue(bool draw) {
	m_drawPixelValue = draw;
	m_option.bDrawPixelValue = draw;
	update();
}

bool xLayeredMatView::SetOption(S_OPTION const& option) {
	m_option = option;
	m_drawPixelValue = option.bDrawPixelValue;
	m_background = QColor(option.crBackground[0], option.crBackground[1], option.crBackground[2]);
	update();
	return true;
}

bool xLayeredMatView::ShowSettingsDialog() {
	xMatViewSettingsDlg dialog(m_option, this);
	if (dialog.exec() != QDialog::Accepted)
		return false;
	return SetOption(dialog.m_option);
}

bool xLayeredMatView::ShowToolBar(bool show) {
	if (!m_ui || !m_ui->toolbar)
		return false;
	m_ui->toolbar->setVisible(show);
	if (m_zoomMode == eZOOM::fit2window || m_zoomMode == eZOOM::fit2width || m_zoomMode == eZOOM::fit2height)
		SetZoomMode(m_zoomMode);
	else
		update();
	return true;
}

bool xLayeredMatView::IsToolBarVisible() const {
	return m_ui && m_ui->toolbar && m_ui->toolbar->isVisible();
}

void xLayeredMatView::SyncToolbar() {
	if (!m_ui)
		return;
	QSignalBlocker blockMode(m_ui->cmbZoomMode);
	QSignalBlocker blockZoom(m_ui->spinZoom);
	if (m_zoomMode != eZOOM::none)
		m_ui->cmbZoomMode->setCurrentIndex(std::to_underlying(m_zoomMode));
	m_ui->spinZoom->setValue(m_zoom * 100.0);
}

void xLayeredMatView::UpdateStatus(gtl::xPoint2d worldPosition) {
	if (!m_ui)
		return;

	QString status = QStringLiteral("x:%1  y:%2").arg(worldPosition.x, 0, 'f', 2).arg(worldPosition.y, 0, 'f', 2);
	for (auto i = m_items.size(); i-- > 0;) {
		auto const& item = m_items[i];
		if (!item.visible)
			continue;
		auto const* image = std::get_if<Image>(&item.content);
		if (!image || image->mat.empty())
			continue;

		auto const x = static_cast<int>(std::floor(worldPosition.x - image->position.x));
		auto const y = static_cast<int>(std::floor(worldPosition.y - image->position.y));
		if (x < 0 || y < 0 || x >= image->mat.cols || y >= image->mat.rows)
			continue;

		QString value;
		switch (image->mat.type()) {
		case CV_8UC1: {
			auto const pixel = image->mat.at<uchar>(y, x);
			value = QStringLiteral("Gray:%1").arg(pixel);
			break;
		}
		case CV_8UC3: {
			auto const pixel = image->mat.at<cv::Vec3b>(y, x);
			if (image->transparentColor && pixel == *image->transparentColor)
				continue; // Color-key pixel: report the visible image below it.
			value = QStringLiteral("BGR:[%1,%2,%3]").arg(pixel[0]).arg(pixel[1]).arg(pixel[2]);
			break;
		}
		case CV_8UC4: {
			auto const pixel = image->mat.at<cv::Vec4b>(y, x);
			if (pixel[3] == 0)
				continue; // Fully transparent pixel: report the visible image below it.
			value = QStringLiteral("BGRA:[%1,%2,%3,%4]").arg(pixel[0]).arg(pixel[1]).arg(pixel[2]).arg(pixel[3]);
			break;
		}
		default:
			continue;
		}

		status += QStringLiteral("  layer:%1  image:[%2,%3]  %4").arg(i).arg(x).arg(y).arg(value);
		break;
	}

	if (status != m_ui->edtInfo->text()) {
		m_ui->edtInfo->setText(status);
		m_ui->edtInfo->setCursorPosition(0);
	}
}

void xLayeredMatView::SetMinTextHeight(double height) {
	m_minTextHeight = std::max(1.0, height);
	update();
}

void xLayeredMatView::NotifyItemsChanged() {
	update();
	emit ItemsChanged();
	if (m_fitOnFirstContent && isVisible())
		FitToWindow();
}

void xLayeredMatView::paintEvent(QPaintEvent*) {
	auto const viewRect = ViewRect();
	if (viewRect.width() <= 0 || viewRect.height() <= 0)
		return;
	cv::Mat canvas(viewRect.height(), viewRect.width(), CV_8UC4,
		cv::Scalar(m_background.blue(), m_background.green(), m_background.red(), 255));
	auto const viewOrigin = gtl::xPoint2d(viewRect.left(), viewRect.top());
	auto const screenOffset = ViewCenter() - viewOrigin + m_pan;
	gtl::xCoordTrans2d const worldToScreen(m_zoom, cv::Matx22d::eye(), {}, screenOffset);
	auto const worldTopLeft = MapToWorld(viewOrigin);
	auto const worldBottomRight = MapToWorld({static_cast<double>(viewRect.right() + 1), static_cast<double>(viewRect.bottom() + 1)});
	QRectF const visibleWorld = QRectF(ToQPoint(worldTopLeft), ToQPoint(worldBottomRight)).normalized();
	cv::Rect const canvasRect(0, 0, canvas.cols, canvas.rows);

	for (auto const& item : m_items) {
		if (!item.visible)
			continue;
		if (auto const* image = std::get_if<Image>(&item.content)) {
			auto const roi = VisibleImageRoi(image->mat, image->position, visibleWorld);
			if (!roi)
				continue;
			cv::Mat source = image->mat(*roi);
			cv::Mat sourceColor;
			if (source.type() == CV_8UC1 && !image->palette.empty())
				cv::applyColorMap(source, sourceColor, image->palette);
			else
				sourceColor = source;
			cv::Mat sourceBGRA;
			if (sourceColor.type() == CV_8UC1)
				cv::cvtColor(sourceColor, sourceBGRA, cv::COLOR_GRAY2BGRA);
			else if (sourceColor.type() == CV_8UC3)
				cv::cvtColor(sourceColor, sourceBGRA, cv::COLOR_BGR2BGRA);
			else if (sourceColor.type() == CV_8UC4)
				sourceBGRA = sourceColor;
			else
				continue;
			cv::Mat sourceAlpha;
			if (source.type() == CV_8UC3 && image->transparentColor) {
				auto const& key = *image->transparentColor;
				cv::inRange(source, cv::Scalar(key[0], key[1], key[2]), cv::Scalar(key[0], key[1], key[2]), sourceAlpha);
				cv::bitwise_not(sourceAlpha, sourceAlpha);
			}

			auto const target0 = worldToScreen(image->position + gtl::xPoint2d(roi->x, roi->y));
			auto const target1 = worldToScreen(image->position + gtl::xPoint2d(roi->x + roi->width, roi->y + roi->height));
			cv::Rect target(cvFloor(std::min(target0.x, target1.x)), cvFloor(std::min(target0.y, target1.y)),
				std::max(1, cvCeil(std::abs(target1.x - target0.x))), std::max(1, cvCeil(std::abs(target1.y - target0.y))));
			cv::Mat resized;
			cv::resize(sourceBGRA, resized, target.size(), 0, 0, InterpolationFor(m_zoom, m_option));
			if (!sourceAlpha.empty()) {
				cv::Mat resizedAlpha;
				cv::resize(sourceAlpha, resizedAlpha, target.size(), 0, 0, cv::INTER_NEAREST);
				int const fromTo[]{0, 3};
				cv::mixChannels(&resizedAlpha, 1, &resized, 1, fromTo, 1);
			}
			auto const clipped = target & canvasRect;
			if (!clipped.empty()) {
				cv::Rect sourceClip(clipped.x - target.x, clipped.y - target.y, clipped.width, clipped.height);
				cv::Mat destination = canvas(clipped);
				AlphaComposite(destination, resized(sourceClip));
			}

			if (m_drawPixelValue) {
				gtl::xCoordTrans2d const imageToScreen(m_zoom, cv::Matx22d::eye(), {}, worldToScreen(image->position));
				gtl::DrawPixelValue(canvas, image->mat, *roi, imageToScreen, m_minTextHeight * devicePixelRatioF());
			}
		} else if (auto const* drawing = std::get_if<Drawing>(&item.content); drawing && drawing->drawing) {
			cv::Matx22d const flipY{1.0, 0.0, 0.0, -1.0};
			gtl::xCoordTrans2d const drawingToScreen(m_zoom, flipY, {}, worldToScreen(drawing->position));
			xLayeredMatViewCanvasMat shapeCanvas(canvas, drawingToScreen);
			drawing->drawing->Draw(shapeCanvas);
		}
	}

	if (m_selection) {
		auto pt0 = worldToScreen(m_selection->first);
		auto pt1 = worldToScreen(m_selection->second);
		cv::Rect selection(cv::Point(cvRound(pt0.x), cvRound(pt0.y)), cv::Point(cvRound(pt1.x), cvRound(pt1.y)));
		selection &= canvasRect;
		if (!selection.empty()) {
			cv::Mat overlay = canvas(selection).clone();
			overlay = cv::Scalar(127, 255, 255, 255);
			cv::addWeighted(overlay, .18, canvas(selection), .82, 0, canvas(selection));
			cv::rectangle(canvas, selection, cv::Scalar(127, 255, 255, 255), 1, cv::LINE_AA);
		}
	}

	QImage image(canvas.data, canvas.cols, canvas.rows, static_cast<qsizetype>(canvas.step), QImage::Format_ARGB32);
	QPainter painter(this);
	painter.drawImage(viewRect.topLeft(), image);
}

void xLayeredMatView::wheelEvent(QWheelEvent* event) {
	if (m_option.bZoomLock || m_zoomMode == eZOOM::mouse_wheel_locked) {
		event->ignore();
		return;
	}
	bool bZoomIn = event->angleDelta().y() > 0;
	ZoomInOut(bZoomIn, ToPoint(event->position()));
	event->accept();
}

void xLayeredMatView::mousePressEvent(QMouseEvent* event) {
	emit MousePressed(event);
	if ((event->button() == Qt::RightButton) ||
		(event->button() == Qt::LeftButton && event->modifiers().testFlag(Qt::ShiftModifier))) {
		m_selecting = true;
		auto const point = MapToWorld(ToPoint(event->position()));
		m_selection = std::pair{point, point};
		update();
		event->accept();
		return;
	}
	if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
		if (m_option.bPanningLock && m_zoomMode != eZOOM::free) {
			event->ignore();
			return;
		}
		m_panning = true;
		m_lastMouse = ToPoint(event->position());
		setCursor(Qt::ClosedHandCursor);
		event->accept();
		return;
	}
	QWidget::mousePressEvent(event);
}

void xLayeredMatView::mouseMoveEvent(QMouseEvent* event) {
	auto const world = MapToWorld(ToPoint(event->position()));
	UpdateStatus(world);
	emit MouseMoved(event, world);
	if (m_selecting && m_selection) {
		m_selection->second = world;
		update();
		event->accept();
		return;
	}
	if (m_panning) {
		auto const current = ToPoint(event->position());
		PanBy((current - m_lastMouse) * m_option.dPanningSpeed);
		m_lastMouse = current;
		event->accept();
		return;
	}
	QWidget::mouseMoveEvent(event);
}

void xLayeredMatView::mouseReleaseEvent(QMouseEvent* event) {
	emit MouseReleased(event);
	if (m_selecting && (event->button() == Qt::RightButton || event->button() == Qt::LeftButton)) {
		m_selecting = false;
		if (m_selection) {
			auto rect = gtl::xRect2d(m_selection->first, m_selection->second);
			rect.NormalizeRect();
			emit SelectionChanged(rect);
		}
		event->accept();
		return;
	}
	if (m_panning && (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton)) {
		m_panning = false;
		unsetCursor();
		event->accept();
		return;
	}
	QWidget::mouseReleaseEvent(event);
}

void xLayeredMatView::mouseDoubleClickEvent(QMouseEvent* event) {
	emit MouseDoubleClicked(event);
	QWidget::mouseDoubleClickEvent(event);
}

void xLayeredMatView::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_M && event->modifiers() == (Qt::ControlModifier | Qt::AltModifier)) {
		ShowToolBar(!IsToolBarVisible());
		event->accept();
		return;
	}
	if (!m_option.bKeyboardNavigation) {
		QWidget::keyPressEvent(event);
		return;
	}
	auto const step = event->modifiers().testFlag(Qt::ShiftModifier) ? 50.0 : 10.0;
	switch (event->key()) {
	case Qt::Key_Left: PanBy({step, 0}); break;
	case Qt::Key_Right: PanBy({-step, 0}); break;
	case Qt::Key_Up: PanBy({0, step}); break;
	case Qt::Key_Down: PanBy({0, -step}); break;
	case Qt::Key_PageUp: PanBy({0, ViewRect().height() * .8}); break;
	case Qt::Key_PageDown: PanBy({0, -ViewRect().height() * .8}); break;
	case Qt::Key_Home: FitToWindow(); break;
	case Qt::Key_Plus:
	case Qt::Key_Equal: ZoomInOut(true, ViewCenter()); break;
	case Qt::Key_Minus: ZoomInOut(false, ViewCenter()); break;
	case Qt::Key_Escape: ClearSelectionRect(); break;
	default: QWidget::keyPressEvent(event); return;
	}
	event->accept();
}

void xLayeredMatView::resizeEvent(QResizeEvent* event) {
	QWidget::resizeEvent(event);
	if (m_fitOnFirstContent && !m_items.empty())
		FitToWindow();
}

} // namespace gtl::qt
