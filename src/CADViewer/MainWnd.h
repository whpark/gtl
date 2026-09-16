#pragma once

#include "ui_MainWnd.h"
#include "ui_OperationsDlg.h"
#include "ui_SelectionFilterDlg.h"
#include <memory>
#include <vector>
#include <optional>
#include <set>
#include <cstdint>

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
	bool SaveShape(QString const& filename);
	bool ExecuteOperation(QString const& operation);

protected:
	void closeEvent(QCloseEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	bool eventFilter(QObject* watched, QEvent* event) override;
	void OnActionAbout(bool checked = false);

private:
    using EntityId = std::uint64_t;
    using RevisionId = std::uint64_t;
    struct EditState {
        std::shared_ptr<gtl::shape::xDrawing> drawing;
        std::vector<EntityId> entityIds;
        std::set<EntityId> workingSetIds, hiddenIds, selectedIds;
        RevisionId revision{};
        QString label;
    };
    std::vector<EntityId> m_entityIds;
    std::set<EntityId> m_workingSetIds, m_hiddenIds;
    std::vector<EditState> m_history;
    size_t m_historyPosition{};
    EntityId m_nextEntityId{1};
    RevisionId m_revision{}, m_savedRevision{}, m_nextRevision{1};
    QString m_filePath;
    std::shared_ptr<gtl::shape::xDrawing> m_clipboard;
    bool m_isPickingPoint{};
    void SetupEditor();
    void RefreshDrawing(std::set<EntityId> const& selected = {});
    std::set<EntityId> SelectedIds() const;
    void SelectIds(std::set<EntityId> const& ids);
    EditState CaptureState(QString label) const;
    void PushHistory(QString label);
    void RestoreHistory(size_t position);
    void RefreshEditorPanels();
    bool MatchesFilter(size_t index) const;
    bool ConfirmDiscard();
    void CopySelection();
    void ExportShapes(QString filename, std::set<EntityId> const& ids);
    void Log(QString const& text);
	Ui::MainWndClass m_ui;
    Ui::OperationsDlg m_operationsUi;
    Ui::SelectionFilterDlg m_filterUi;
    QDialog* m_operationsDialog{};
    QDialog* m_filterDialog{};
	std::shared_ptr<gtl::shape::xDrawing> m_drawing;
	std::vector<gtl::shape::xShape const*> m_sourceShapes;
	std::vector<gtl::shape::xShape*> m_displayShapes;
	std::size_t m_selectionOverlayIndex{static_cast<std::size_t>(-1)};
	std::optional<QPointF> m_dragStart;
	void SelectRegion(QPointF start, QPointF end);
	void FitTreeItem(QTreeWidgetItem* item);
	void PopulateTree();
	void RefreshSelectionPresentation();
};
