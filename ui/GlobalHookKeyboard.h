#pragma once
/*
@bref 捕获QT 全局鼠标，键盘事件
main 函数中，使用GlobalHookKeyboard a(argc, argv);替换QApplication a(argc, argv);
*/
#include <QObject>
#include <qapplication.h>


class GlobalHookKeyboard  : public QApplication
{
	Q_OBJECT

public:
	GlobalHookKeyboard(int& argc, char** argv);
	~GlobalHookKeyboard();

	bool notify(QObject*, QEvent*);
	void setWindowInstance(QWidget* wnd);

private:
	QWidget*	_widget;
	bool		_Alt = false;
	bool		_Ctrl = false;
};
