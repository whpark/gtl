#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QGroupBox>

namespace gtl::qt { class GTL__QT_CLASS QCollapsibleGroupBox; }

class gtl::qt::QCollapsibleGroupBox : public QGroupBox {
	Q_OBJECT
	Q_PROPERTY(bool collapsed READ IsCollapsed WRITE Collapse)
public:
	using this_t = QCollapsibleGroupBox;
	using base_t = QGroupBox;

	int m_heightDeflated{20};
	int m_heightInflated{};

	bool m_bCollapsed{false};
protected:
	int m_heightMin0{}, m_heightMax0{0xff'ffff};
	QPropertyAnimation m_aniPropMaxHeightD, m_aniPropMaxHeight, m_aniPropMaxHeightFinal, m_aniPropMinHeightFinal;
	QSequentialAnimationGroup m_animationsDeflate, m_animationsInflate;
	std::chrono::milliseconds m_durAnimation{ 200 };
public:
	explicit QCollapsibleGroupBox(QWidget *parent = nullptr);
	explicit QCollapsibleGroupBox(const QString &title, QWidget *parent = nullptr);
	~QCollapsibleGroupBox();
protected:
	void Init();

public:
	bool PrepareAnimation(std::chrono::milliseconds durAnimation = std::chrono::milliseconds(300));

	bool Collapse(bool bCollapse, std::optional<std::chrono::milliseconds> durAnimation = std::nullopt);
	bool IsCollapsed() const;
};
