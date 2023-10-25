#include "FrameLessWidget.h"


#define MOUSE_GAP 10
FrameLessWidget::FrameLessWidget(QWidget *parent)
	: QWidget(parent)
{
	this->setMouseTracking(true);
	this->setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint);
	this->setMinimumSize(100, 100);

	m_oldSize = this->size();
	m_globalPoint = this->pos();
}

FrameLessWidget::~FrameLessWidget()
{}

void FrameLessWidget::setWidgetType(bool canMove, DragType drag_type,bool can_max)
{
    m_bCanMove = canMove;
    drag_type_ = drag_type;
    m_bCanMax = can_max;
}

void FrameLessWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (m_bCanMax) {
        m_bMax = !m_bMax;
        if (m_bMax) {
            this->showMaximized();
        }
        else {
            this->showNormal();
        }
    }
}

void FrameLessWidget::mousePressEvent(QMouseEvent* event)
{
	if (!m_bMax){
		m_isPressed = true;
		m_startMovePos = event->globalPos();

		if (event->buttons() == Qt::LeftButton && m_clickType != CLICK_NULL){
			m_bCanResize = true;
		}
	}
	return QWidget::mousePressEvent(event);
}

void FrameLessWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (ev->buttons() == Qt::RightButton) {
        return;
    }
	static QPoint rightTop;
	static QPoint leftBottom;
	static QPoint leftTop;
	static QPoint rightBottom;

    if (!m_bResizeIng && drag_type_!= DragType::Drag_Null){
        if (drag_type_ == DragType::Drag_Heigt) {
            if (ev->x() > MOUSE_GAP && ev->y() < MOUSE_GAP){
                this->setCursor(Qt::SizeVerCursor);
                m_clickType = CLICK_TOP;
            }
            else if (ev->y() > this->height() - MOUSE_GAP){
                this->setCursor(Qt::SizeVerCursor);
                m_clickType = CLICK_BOTTOM;
            }
            else{
                this->setCursor(Qt::ArrowCursor);
                m_clickType = CLICK_NULL;
            }
        }
        if (drag_type_ == DragType::Drag_Weight) {
            if (ev->x() < MOUSE_GAP){
                this->setCursor(Qt::SizeHorCursor);
                m_clickType = CLICK_LEFT;
            }
            else if (ev->x() > this->width() - MOUSE_GAP && ev->y() < this->height() - MOUSE_GAP){
                this->setCursor(Qt::SizeHorCursor);
                m_clickType = CLICK_RIGHT;
            }
            else {
                this->setCursor(Qt::ArrowCursor);
                m_clickType = CLICK_NULL;
            }
        }
        else if (drag_type_ == DragType::Drag_All) {
            //右下角
            if (ev->x() > this->width() - MOUSE_GAP && ev->y() > this->height() - MOUSE_GAP) {
                this->setCursor(Qt::SizeFDiagCursor);
                m_clickType = CLICK_BOTTOM_RIGHT;
            }
            //右上
            else if (ev->x() > this->width() - MOUSE_GAP && ev->x() > MOUSE_GAP && ev->y() < MOUSE_GAP) {
                this->setCursor(Qt::SizeBDiagCursor);
                m_clickType = CLICK_TOP_RIGHT;
            }
            //左上
            else if (ev->x() < MOUSE_GAP && ev->y() < MOUSE_GAP){
                this->setCursor(Qt::SizeFDiagCursor);
                m_clickType = CLICK_TOP_LETT;
            }
            //左下
            else if (ev->x() < MOUSE_GAP && ev->y() > this->height() - MOUSE_GAP){
                this->setCursor(Qt::SizeBDiagCursor);
                m_clickType = CLICK_BOTTOM_LETT;
            }
            //左
            else if (ev->x() < MOUSE_GAP){
                this->setCursor(Qt::SizeHorCursor);
                m_clickType = CLICK_LEFT;
            }
            //右边
            else if (ev->x() > this->width() - MOUSE_GAP && ev->y() < this->height() - MOUSE_GAP){
                this->setCursor(Qt::SizeHorCursor);
                m_clickType = CLICK_RIGHT;
            }
            //上边
            else if (ev->x() > MOUSE_GAP && ev->y() < MOUSE_GAP){
                this->setCursor(Qt::SizeVerCursor);
                m_clickType = CLICK_TOP;
            }
            //下边
            else if (ev->y() > this->height() - MOUSE_GAP){
                this->setCursor(Qt::SizeVerCursor);
                m_clickType = CLICK_BOTTOM;
            }
            else{
                this->setCursor(Qt::ArrowCursor);
                m_clickType = CLICK_NULL;
            }
        }
    }

    if (m_bCanResize && m_clickType != CLICK_NULL){
        if (m_clickType == CLICK_RIGHT){
            if (m_globalPoint.x() < ev->globalPos().x() && ev->globalPos().x() > rightTop.x()){
                m_bResizeIng = true;
                this->resize(m_oldSize.width() + (ev->globalX() - m_globalPoint.x()), this->height());
            }
            else if (ev->globalPos().x() < m_globalPoint.x()){
                m_bResizeIng = true;
                this->resize(m_oldSize.width() - (m_globalPoint.x() - ev->globalX()), this->height());
            }
        }
        else if (m_clickType == CLICK_BOTTOM){
            if (ev->globalY() > m_globalPoint.y() && ev->globalY() > this->y() + this->height()){
                m_bResizeIng = true;
                this->resize(this->width(), this->height() + ev->globalY() - m_globalPoint.y());
            }
            else if (ev->globalY() < m_globalPoint.y()){
                m_bResizeIng = true;
                this->resize(this->width(), this->height() - (m_globalPoint.y() - ev->globalY()));
            }
        }
        else if (m_clickType == CLICK_LEFT){
            if (m_globalPoint.x() > ev->globalX() && leftTop.x() > ev->globalX()){
                m_bResizeIng = true;
                this->resize(m_oldSize.width() + m_globalPoint.x() - ev->globalX(), this->height());
                this->move(this->x() - (m_globalPoint.x() - ev->globalPos().x()), this->y());
            }
            else if (m_globalPoint.x() < ev->globalX()){
                if (this->width() != this->minimumWidth()){
                    m_bResizeIng = true;
                    this->resize(m_oldSize.width() + m_globalPoint.x() - ev->globalX(), this->height());
                    this->move(rightTop.x() - this->width(), this->y());
                }
            }
        }
        else if (m_clickType == CLICK_TOP){
            if (m_globalPoint.y() > ev->globalY() && ev->globalY() < this->y()){
                m_bResizeIng = true;

                this->resize(this->width(), this->height() + (m_globalPoint.y() - ev->globalY()));
                this->move(this->x(), this->y() - (m_globalPoint.y() - ev->globalY()));
            }
            else if (m_globalPoint.y() < ev->globalY()){
                m_bResizeIng = true;
                this->resize(this->width(), this->height() - (ev->globalY() - m_globalPoint.y()));
                this->move(this->x(), leftBottom.y() - this->height());
            }
        }
        else if (m_clickType == CLICK_BOTTOM_RIGHT){
            if (ev->globalX() > m_globalPoint.x() || ev->globalY() > m_globalPoint.y()){
                if (ev->globalX() > rightBottom.x()){
                    m_bResizeIng = true;
                    this->resize(m_oldSize.width() + (ev->globalX() - m_globalPoint.x()), this->height());
                }
                if (ev->globalY() > rightBottom.y()){
                    m_bResizeIng = true;
                    this->resize(this->width(), this->height() + ev->globalY() - m_globalPoint.y());
                }
            }
            else if (ev->globalX() < m_globalPoint.x() || ev->globalY() < m_globalPoint.y()){
                m_bResizeIng = true;
                this->resize(m_oldSize.width() - (m_globalPoint.x() - ev->globalX()), this->height());
                this->resize(this->width(), this->height() - (m_globalPoint.y() - ev->globalY()));
            }
        }
        else if (m_clickType == CLICK_TOP_RIGHT){
            if (ev->globalX() > m_globalPoint.x() || ev->globalY() < m_globalPoint.y()){
                if (ev->globalX() > rightTop.x()){
                    m_bResizeIng = true;
                    this->resize(m_oldSize.width() + (ev->globalX() - m_globalPoint.x()), this->height());
                }
                if (ev->globalY() < rightTop.y()){
                    m_bResizeIng = true;
                    this->resize(this->width(), this->height() + (m_globalPoint.y() - ev->globalY()));
                    this->move(leftTop.x(), this->y() - (m_globalPoint.y() - ev->globalY()));
                }

            }
            else if (ev->globalX() < m_globalPoint.x() || ev->globalY() > m_globalPoint.y()){
                m_bResizeIng = true;
                this->resize(this->width(), this->height() - (ev->globalY() - m_globalPoint.y()));
                this->resize(m_oldSize.width() - (m_globalPoint.x() - ev->globalX()), this->height());
                this->move(leftTop.x(), leftBottom.y() - this->height());
            }
        }
        else if (m_clickType == CLICK_TOP_LETT){
            if (ev->globalX() < m_globalPoint.x() || ev->globalY() < m_globalPoint.y()){
                if (ev->globalX() < leftTop.x()){
                    m_bResizeIng = true;
                    this->resize(m_oldSize.width() + m_globalPoint.x() - ev->globalX(), this->height());
                    this->move(this->x() - (m_globalPoint.x() - ev->globalPos().x()), leftTop.y());
                }
                if (ev->globalY() < leftTop.y()){
                    m_bResizeIng = true;
                    this->resize(this->width(), this->height() + (m_globalPoint.y() - ev->globalY()));
                    this->move(this->x(), this->y() - (m_globalPoint.y() - ev->globalY()));
                }
            }
            else if (ev->globalX() > m_globalPoint.x() || ev->globalY() > m_globalPoint.y()){
                m_bResizeIng = true;
                this->resize(m_oldSize.width() + m_globalPoint.x() - ev->globalX(), this->height());
                this->move(rightTop.x() - this->width(), this->y());

                if (ev->globalY() > leftTop.y()){
                    this->resize(this->width(), this->height() - (ev->globalY() - m_globalPoint.y()));
                    this->move(this->x(), leftBottom.y() - this->height());
                }
            }
        }
        else if (m_clickType == CLICK_BOTTOM_LETT){
            if (ev->globalX() < m_globalPoint.x() || ev->globalY() > m_globalPoint.y()){
                if (ev->globalX() < leftTop.x()){
                    m_bResizeIng = true;
                    this->resize(m_oldSize.width() + m_globalPoint.x() - ev->globalX(), this->height());
                    this->move(this->x() - (m_globalPoint.x() - ev->globalPos().x()), this->y());
                }
                if (ev->globalY() > leftBottom.y()){
                    m_bResizeIng = true;
                    this->resize(this->width(), this->height() + ev->globalY() - m_globalPoint.y());
                }
            }
            else if (ev->globalX() > m_globalPoint.x() || ev->globalY() < m_globalPoint.y()){
                m_bResizeIng = true;
                this->resize(this->width(), this->height() - (m_globalPoint.y() - ev->globalY()));
                this->resize(m_oldSize.width() + m_globalPoint.x() - ev->globalX(), this->height());
                this->move(rightTop.x() - this->width(), this->y());
            }
        }
    }

    if (m_clickType == CLICK_NULL && m_bCanMove) {
        if (m_isPressed){
            QPoint movePoint = ev->globalPos() - m_startMovePos;
            m_startMovePos = ev->globalPos();
            this->move(pos().x() + movePoint.x(), pos().y()+movePoint.y());
        }
    }

    leftTop = this->pos();
    leftBottom.setX(this->pos().x()); leftBottom.setY(this->y() + this->height());
    rightTop.setX(this->x() + this->width()); rightTop.setY(this->y());
    rightBottom.setX(this->x() + this->width()); rightBottom.setY(this->y() + this->height());
    m_oldSize = this->size();
    m_globalPoint = ev->globalPos();

	return QWidget::mouseMoveEvent(ev);
}

void FrameLessWidget::mouseReleaseEvent(QMouseEvent* event)
{
	this->setCursor(Qt::ArrowCursor);
	m_isPressed = false;
	m_clickType = CLICK_NULL;
	m_bCanResize = false;
	m_bResizeIng = false;
	return QWidget::mouseReleaseEvent(event);
}