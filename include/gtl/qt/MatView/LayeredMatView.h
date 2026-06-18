#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/qt/MatView/MatView.h"
#include "gtl/coord.h"

#include <QWidget>
#include <opencv2/core/mat.hpp>

#include <memory>
#include <optional>
#include <variant>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class LayeredMatViewClass; }
QT_END_NAMESPACE

namespace gtl::shape {
	class xDrawing;
}

namespace gtl::qt {

class GTL__QT_CLASS xLayeredMatView : public QWidget {
	Q_OBJECT

public:
	using index_t = std::size_t;
	using S_OPTION = xMatView::S_OPTION;
	using eZOOM = xMatView::eZOOM;
	using eZOOM_IN = xMatView::eZOOM_IN;
	using eZOOM_OUT = xMatView::eZOOM_OUT;
	static constexpr index_t npos = static_cast<index_t>(-1);

	struct Image {
		cv::Mat mat;
		cv::Mat palette;
		gtl::xPoint2d position;
		std::optional<cv::Vec3b> transparentColor; // BGR color key for CV_8UC3
	};

	struct Drawing {
		std::shared_ptr<gtl::shape::xDrawing> drawing;
		gtl::xPoint2d position;
	};

	using content_t = std::variant<Image, Drawing>;

	struct Item {
		content_t content;
		bool visible{true};

		bool IsImage() const noexcept { return std::holds_alternative<Image>(content); }
		bool IsDrawing() const noexcept { return std::holds_alternative<Drawing>(content); }
		gtl::xPoint2d Position() const noexcept;
		void SetPosition(gtl::xPoint2d position) noexcept;
	};

	using container_t = std::vector<Item>;
	using iterator = container_t::iterator;
	using const_iterator = container_t::const_iterator;

	explicit xLayeredMatView(QWidget* parent = nullptr);
	~xLayeredMatView() override;

	index_t Add(cv::Mat const& image, gtl::xPoint2d position = {}, bool copy = true,
		std::optional<cv::Vec3b> transparentColor = {});
	index_t Add(cv::Mat const& image, gtl::xPoint2d position, cv::Vec3b transparentColor, bool copy = true);
	index_t Add(gtl::shape::xDrawing const& drawing, gtl::xPoint2d position = {});
	index_t Add(std::shared_ptr<gtl::shape::xDrawing> drawing, gtl::xPoint2d position = {});
	bool Delete(index_t index);
	void Clear();

	index_t Count() const noexcept { return m_items.size(); }
	bool Empty() const noexcept { return m_items.empty(); }
	Item& At(index_t index) { return m_items.at(index); }
	Item const& At(index_t index) const { return m_items.at(index); }
	Item& operator[](index_t index) noexcept { return m_items[index]; }
	Item const& operator[](index_t index) const noexcept { return m_items[index]; }
	iterator begin() noexcept { return m_items.begin(); }
	iterator end() noexcept { return m_items.end(); }
	const_iterator begin() const noexcept { return m_items.begin(); }
	const_iterator end() const noexcept { return m_items.end(); }
	const_iterator cbegin() const noexcept { return m_items.cbegin(); }
	const_iterator cend() const noexcept { return m_items.cend(); }

	bool SetPosition(index_t index, gtl::xPoint2d position);
	bool SetVisible(index_t index, bool visible);
	bool SetPalette(index_t index, cv::Mat const& palette, bool updateView = true);
	bool SetTransparentColor(index_t index, std::optional<cv::Vec3b> transparentColor, bool updateView = true);
	bool Move(index_t from, index_t to);
	bool BringToFront(index_t index);
	bool SendToBack(index_t index);
	bool MoveForward(index_t index);
	bool MoveBackward(index_t index);

	double Zoom() const noexcept { return m_zoom; }
	eZOOM ZoomMode() const noexcept { return m_zoomMode; }
	bool SetZoomMode(eZOOM mode, bool center = true);
	gtl::xPoint2d Pan() const noexcept { return m_pan; }
	void SetZoom(double zoom, gtl::xPoint2d anchor = {});
	void ZoomInOut(bool bZoomIn, xPoint2i ptAnchor);
	void SetPan(gtl::xPoint2d pan);
	void PanBy(gtl::xPoint2d delta);
	void FitToWindow(double margin = 20.0);
	void ResetView();

	gtl::xPoint2d MapToWorld(gtl::xPoint2d viewportPosition) const noexcept;
	gtl::xPoint2d MapFromWorld(gtl::xPoint2d worldPosition) const noexcept;
	QRectF SceneBounds() const;
	std::optional<std::pair<gtl::xPoint2d, gtl::xPoint2d>> GetSelectionPoints() const noexcept;
	void SetSelectionRect(gtl::xRect2d const& rect);
	void ClearSelectionRect();

	QColor BackgroundColor() const noexcept { return m_background; }
	void SetBackgroundColor(QColor color);
	bool DrawPixelValue() const noexcept { return m_drawPixelValue; }
	void SetDrawPixelValue(bool draw);
	double MinTextHeight() const noexcept { return m_minTextHeight; }
	void SetMinTextHeight(double height);
	S_OPTION const& GetOption() const noexcept { return m_option; }
	bool SetOption(S_OPTION const& option);
	bool ShowSettingsDialog();
	bool ShowToolBar(bool show);
	bool IsToolBarVisible() const;

signals:
	void ViewChanged(double zoom, gtl::xPoint2d pan);
	void ItemsChanged();
	void SelectionChanged(gtl::xRect2d rect);
	void MousePressed(QMouseEvent* event);
	void MouseReleased(QMouseEvent* event);
	void MouseDoubleClicked(QMouseEvent* event);
	void MouseMoved(QMouseEvent* event, gtl::xPoint2d worldPosition);

protected:
	void paintEvent(QPaintEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	container_t m_items;
	double m_zoom{1.0};
	eZOOM m_zoomMode{eZOOM::fit2window};
	gtl::xPoint2d m_pan;
	QColor m_background{Qt::black};
	bool m_drawPixelValue{true};
	double m_minTextHeight{8.0};
	S_OPTION m_option;
	bool m_panning{};
	bool m_selecting{};
	gtl::xPoint2d m_lastMouse;
	std::optional<std::pair<gtl::xPoint2d, gtl::xPoint2d>> m_selection;
	bool m_fitOnFirstContent{true};
	std::unique_ptr<Ui::LayeredMatViewClass> m_ui;

	gtl::xPoint2d ViewCenter() const noexcept;
	QRect ViewRect() const noexcept;
	void SyncToolbar();
	void UpdateStatus(gtl::xPoint2d worldPosition);
	void NotifyItemsChanged();
};

} // namespace gtl::qt
