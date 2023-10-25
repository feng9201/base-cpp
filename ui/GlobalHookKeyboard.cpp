#include "GlobalHookKeyboard.h"
#include <QKeyEvent>


GlobalHookKeyboard::GlobalHookKeyboard(int& argc, char** argv)
	: QApplication(argc, argv)
{}

GlobalHookKeyboard::~GlobalHookKeyboard()
{}

void GlobalHookKeyboard::setWindowInstance(QWidget* wnd)
{
    _widget = wnd;
}

bool GlobalHookKeyboard::notify(QObject* obj, QEvent* e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
        if (keyEvent->key() == Qt::Key_G && keyEvent->modifiers() == (Qt::AltModifier | Qt::ControlModifier))
        {
            
        }
        if (keyEvent->key() == Qt::Key_K && keyEvent->modifiers() == (Qt::ControlModifier))
        {
            
        }
    }
    else if (e->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
        if (keyEvent->key() == Qt::Key_G)
        {
        }
    }
    return QApplication::notify(obj, e);
}