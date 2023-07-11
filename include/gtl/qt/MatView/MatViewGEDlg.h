#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include <QDialog>
#include "gtl/reflection_glaze.h"
#include "MatViewWidget.h"
#include "MatViewCanvas.h"

namespace Ui {
	class MatViewDlgClass;
}

namespace gtl::qt {

	using namespace std::literals;

	class GTL__QT_CLASS xMatViewDlg : public QDialog {
		Q_OBJECT

	public:
		using this_t = xMatViewDlg;
		using base_t = QDialog;

	protected:
		xMatViewWidget* m_widgetMatView{};

	public:
		xMatViewDlg(QWidget* parent = nullptr);
		~xMatViewDlg();

		bool SetImage(cv::Mat const& img, bool bCopy = false);
		xMatViewWidget& GetViewWidget() { assert(m_widgetMatView); return *m_widgetMatView; }
		xMatViewCanvas& GetView() { assert(m_widgetMatView); return m_widgetMatView->GetView(); }


	private:
		std::unique_ptr<Ui::MatViewDlgClass> ui;
	};

} // namespace gtl::qt
