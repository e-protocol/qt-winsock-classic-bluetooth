#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QDialog>
#include <QToolButton>
#include <QPainter>
#include "customTitleBar.h"

class MsgBox : public QDialog
{
    Q_OBJECT

public:
    MsgBox();

    enum Button
    {
        YES,
        NO,
        DROP,
        NONE
    };

    static void warning(const QString &title, const QString &message);
    static void information(const QString &title, const QString &message);
    static void critical(const QString &title, const QString &message);
    void setTitle(const QString &title) { _title = title; }
    void setMessage(const QString &message) { _message = message; }
    void setButtons(Button yesBtn = NONE, Button noBtn = NONE, Button dropBtn = NONE);
    int exec(CustomTitleBar::WindowType type);
    int exec() override;

private:
    Button _yesBtn = NONE;
    Button _noBtn = NONE;
    Button _dropBtn = NONE;
    Button clickedBtn = NONE;
    QString _title = "";
    QString _message = "";
    const int dialogSide = 100;
    const int spacing = 10;
    const int btnHeight = 36;
    const int btnWidth = 76;
    const int iconSide = 60;
    const int minDialogWidth = 350;
    QIcon* icon = nullptr;
    QVBoxLayout* vLayout = nullptr;
    CustomTitleBar* customTitleBar = nullptr;
    CustomTitleBar::WindowType _type = CustomTitleBar::DIALOG_MSGBOX;
    const int headerHeight = 30;
    void initMessageBox();
    void closeEvent(QCloseEvent *closeEvent) override;

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void finished();
};

#endif // MESSAGEBOX_H
