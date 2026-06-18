#pragma once

#include <QDialog>
#include "ui_LayeredMatViewDlg.h"

class xLayeredMatViewDlg : public QDialog {
	Q_OBJECT

public:
	explicit xLayeredMatViewDlg(QWidget* parent = nullptr);
	~xLayeredMatViewDlg() override;

private:
	Ui::xLayeredMatViewDlgClass ui;
	int m_addedImageCount{};

	void Populate();
	void AddImage();
};
