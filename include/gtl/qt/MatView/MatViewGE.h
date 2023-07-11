#pragma once

#include <QWidget>
#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/qt/MatView/MatViewCanvas.h"

namespace Ui {
	class MatViewWidgetClass;
}

namespace gtl::qt {

	class GTL__QT_CLASS xMatViewWidget : public QWidget {
		Q_OBJECT

	public:
		using this_t = xMatViewWidget;
		using base_t = QWidget;

	protected:
		xMatViewCanvas* m_view{};
	public:
		xMatViewWidget(QWidget* parent = nullptr);
		~xMatViewWidget();

		xMatViewCanvas& GetView() { assert(m_view); return *m_view; }
		xMatViewCanvas const& GetView() const { assert(m_view); return *m_view; }

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
		std::unique_ptr<Ui::MatViewWidgetClass> ui;
	};

}	// namespace gtl::qt
