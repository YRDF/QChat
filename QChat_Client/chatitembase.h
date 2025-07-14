#ifndef CHATITEMBASE_H
#define CHATITEMBASE_H
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include "global.h"
class BubbleFrame;

class ChatItemBase: public QWidget
{
    Q_OBJECT
public:
    explicit ChatItemBase(ChatRole role, QWidget *parent = nullptr);
    void setUserName(const QString &name);
    void setUserIcon(const QPixmap &icon);
    void setWidget(QWidget *w);

private:
    //聊天对象是自己还是 他人
    ChatRole m_role;
    QLabel *m_pNameLabel;
    QLabel *m_pIconLabel;
    QWidget *m_pBubble;
};

#endif // CHATITEMBASE_H
