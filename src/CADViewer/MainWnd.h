#pragma once

#include "ui_MainWnd.h"
#include <memory>
#include <vector>

namespace gtl::qt { class xLayeredMatView; }
namespace gtl::shape { class xDrawing; class xShape; }
class QTreeWidget;
class QTreeWidgetItem;
class QPlainTextEdit;

class xMainWnd : public QMainWindow {
	Q_OBJECT
public:
	using this_t = xMainWnd;
	using base_t = QMainWindow;

public:
	xMainWnd(QWidget* parent = nullptr);
	~xMainWnd();
	bool OpenFile(QString const& filename);

protected:
	void OnAction_About(bool checked = false);

private:
	Ui::MainWndClass ui;
	gtl::qt::xLayeredMatView* m_view{};
	QTreeWidget* m_entities{};
	QTreeWidget* m_properties{};
	QPlainTextEdit* m_details{};
	std::shared_ptr<gtl::shape::xDrawing> m_drawing;
	std::vector<gtl::shape::xShape const*> m_shapes;
	std::size_t m_highlight{static_cast<std::size_t>(-1)};
	void PopulateTree();
	void SelectEntity(QTreeWidgetItem* item);
};
