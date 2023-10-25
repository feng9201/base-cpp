#pragma once

#include <QWidget>
#include <QMouseEvent>

enum class DragType {
	Drag_Null =0,
	Drag_All=1,			//可随意拖拽大小
	Drag_Weight,
	Drag_Heigt
};

enum ClickType
{
	CLICK_NULL,
	CLICK_TOP,
	CLICK_BOTTOM,
	CLICK_LEFT,
	CLICK_RIGHT,
	CLICK_BOTTOM_RIGHT,
	CLICK_TOP_RIGHT,
	CLICK_TOP_LETT,
	CLICK_BOTTOM_LETT,
};

class FrameLessWidget  : public QWidget
{
	Q_OBJECT

public:
	FrameLessWidget(QWidget *parent);
	~FrameLessWidget();

	void setWidgetType(bool canMove, DragType drag_type, bool cam_double_max=false);

protected:
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
private:
	bool		m_bCanMove = true;		//可移动
	bool		m_bCanMax = false;
	DragType	drag_type_ = DragType::Drag_All;
	bool		m_bCanDrag = true;	
	bool		m_bMax = false;
	bool		m_isPressed = false;

	ClickType	m_clickType = CLICK_NULL;
	bool		m_bCanResize = false;
	bool		m_bResizeIng = false;

	QSize		m_oldSize;
	QPoint		m_globalPoint;
	QPoint		m_startMovePos;
};
