#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>


enum class messageType {
	TIP_BOX = 0,
	WARN_BOX,
};

class QConformMsgBox  : public QDialog
{
	Q_OBJECT

public:
	QConformMsgBox(QWidget *parent = nullptr);
	~QConformMsgBox();

	void initUI();
	int showMsg(const QString& msg, const QString& title="", messageType msgType = messageType::TIP_BOX);
protected:
	void paintEvent(QPaintEvent*);

private:
	QLabel*			lb_title_ = nullptr;
	QLabel*			lb_content_ = nullptr;
	QPushButton*	pb_cancel_ = nullptr;
	QPushButton*	pb_sure_ = nullptr;
};
