#pragma once

#include "ui_MainWnd.h"

class xMainWnd : public QMainWindow {
	Q_OBJECT
public:
	using this_t = xMainWnd;
	using base_t = QMainWindow;

public:
	xMainWnd(QWidget* parent = nullptr);
	~xMainWnd();

protected:
	void OnAction_About(bool checked = false);

private:
	Ui::MainWndClass ui;
};
