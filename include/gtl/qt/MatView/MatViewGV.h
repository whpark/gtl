#pragma once

#include <QWidget>
#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/qt/MatView/MatViewGVCanvas.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MatViewGVClass; }
QT_END_NAMESPACE

namespace gtl::qt {
	class GTL__QT_CLASS xMatViewGV;
}	// namespace gtl::qt

class GTL__QT_CLASS gtl::qt::xMatViewGV : public QWidget {
	Q_OBJECT

public:
	using this_t = xMatViewGV;
	using base_t = QWidget;

protected:
	xMatViewGVCanvas* m_view{};
public:
	xMatViewGV(QWidget* parent = nullptr);
	~xMatViewGV();

	xMatViewGVCanvas& GetView() { assert(m_view); return *m_view; }
	xMatViewGVCanvas const& GetView() const { assert(m_view); return *m_view; }

	bool SetImage(cv::Mat const& img, bool bCopy = false) {
		if (!m_view)
			return false;
		return m_view->SetImage(img, bCopy);
	}

protected slots:
	void OnSigMouseMoved(int x, int y);
	void OnSigMouseSelected(QRect rect);
	void OnSigZoom(double zoom);
	void OnSpinZoomValueChanged(double value);
	void OnBtnZoomIn();
	void OnBtnZoomOut();
	void OnBtnZoomFit();
	void OnBtnSettings();

private:
	std::unique_ptr<Ui::MatViewGVClass> ui;
};
