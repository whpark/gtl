#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include <QDialog>
#include "gtl/reflection_glaze.h"
#include "MatViewGV.h"
#include "MatViewGVCanvas.h"

namespace Ui {
	class MatViewGVDlgClass;
}

namespace gtl::qt {

	using namespace std::literals;

	class GTL__QT_CLASS xMatViewGVDlg : public QDialog {
		Q_OBJECT

	public:
		using this_t = xMatViewGVDlg;
		using base_t = QDialog;

	protected:
		xMatViewGV* m_widgetMatView{};

	public:
		xMatViewGVDlg(QWidget* parent = nullptr);
		~xMatViewGVDlg();

		bool SetImage(cv::Mat const& img, bool bCopy = false);
		xMatViewGV& GetViewWidget() { assert(m_widgetMatView); return *m_widgetMatView; }
		xMatViewGVCanvas& GetView() { assert(m_widgetMatView); return m_widgetMatView->GetView(); }


	private:
		std::unique_ptr<Ui::MatViewGVDlgClass> ui;
	};

} // namespace gtl::qt
