#pragma once

#include "ui_MainWnd.h"
#include <memory>
#include <vector>
#include <optional>

namespace gtl::shape { class xDrawing; class xShape; }
class QTreeWidgetItem;

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
	bool eventFilter(QObject* watched, QEvent* event) override;
	void OnAction_About(bool checked = false);

private:
	Ui::MainWndClass ui;
	std::shared_ptr<gtl::shape::xDrawing> m_drawing;
	std::vector<gtl::shape::xShape const*> m_shapes;
	std::vector<gtl::shape::xShape*> m_displayShapes;
	std::size_t m_highlight{static_cast<std::size_t>(-1)};
	std::optional<QPointF> m_dragStart;
	void SelectRegion(QPointF start, QPointF end);
	void FitTreeItem(QTreeWidgetItem* item);
	void PopulateTree();
	void SelectEntity(QTreeWidgetItem* item);
};
