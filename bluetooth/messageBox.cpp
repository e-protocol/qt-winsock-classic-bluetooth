#include "messageBox.h"

MsgBox::MsgBox()
{
    setAttribute(Qt::WA_ShowModal, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute( Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground);
}

//окно предупреждение
void MsgBox::warning(const QString &title, const QString &message)
{
    MsgBox* msgBox = new MsgBox;
    msgBox->icon = new QIcon(msgBox->style()->standardIcon(QStyle::SP_MessageBoxWarning));
    msgBox->_title = title;
    msgBox->_message = message;
    msgBox->exec();
}

//окно информации
void MsgBox::information(const QString &title, const QString &message)
{
    MsgBox* msgBox = new MsgBox;
    msgBox->icon = new QIcon(msgBox->style()->standardIcon(QStyle::SP_MessageBoxInformation));
    msgBox->_title = title;
    msgBox->_message = message;
    msgBox->exec();
}

//окно ошибки
void MsgBox::critical(const QString &title, const QString &message)
{
    MsgBox* msgBox = new MsgBox;
    msgBox->icon = new QIcon(msgBox->style()->standardIcon(QStyle::SP_MessageBoxCritical));
    msgBox->_title = title;
    msgBox->_message = message;
    msgBox->exec();
}

//настраиваемое окно
int MsgBox::exec(CustomTitleBar::WindowType type)
{
    _type = type;
    initMessageBox();
    show();

    QEventLoop eventLoop;
    connect(this, &MsgBox::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    return clickedBtn;
}

//настраиваемое окно
int MsgBox::exec()
{
    initMessageBox();
    show();

    QEventLoop eventLoop;
    connect(this, &MsgBox::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    return clickedBtn;
}

void MsgBox::setButtons(Button yesBtn, Button noBtn, Button dropBtn)
{
    _yesBtn = yesBtn;
    _noBtn = noBtn;
    _dropBtn = dropBtn;
}

//инициализация окна
void MsgBox::initMessageBox()
{
    //сначала задать размер окна
    QLabel* messageLabel = new QLabel;
    messageLabel->setObjectName("dialogLabelMsgBox");
    messageLabel->setText(_message);
    int textWidth = 0;
    int textHeight = 0;
    QStringList strList = _message.split(QRegExp("[\\r\\n]"), Qt::KeepEmptyParts);
    QFontMetrics metrics(messageLabel->font());

    for(int k = 0; k < strList.size(); k++)
    {
        int curLength = metrics.horizontalAdvance(strList[k]);

        if(curLength > textWidth)
            textWidth = curLength;
    }

    textHeight = strList.size() * metrics.capHeight();
    int dialogWidth = textWidth + iconSide * 2 + spacing * 4;
    int dialogHeight = headerHeight + btnHeight * 3 + spacing * 3 + textHeight;

    if(dialogWidth <= dialogSide * 2)
        dialogWidth = dialogSide * 2 + iconSide;

    if(dialogWidth < minDialogWidth)
        dialogWidth = minDialogWidth;

    this->setFixedSize(dialogWidth, dialogHeight);

    //header
    vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,spacing);
    vLayout->setSpacing(0);
    QWidget* headerWidget = new QWidget;
    headerWidget->setFixedHeight(headerHeight);
    customTitleBar = new CustomTitleBar(this, nullptr, headerWidget);

    _type == CustomTitleBar::DIALOG_MSGBOX
            ? customTitleBar->setWindowType(CustomTitleBar::DIALOG_MSGBOX)
            : customTitleBar->setWindowType(CustomTitleBar::DIALOG_CLOSE);

    customTitleBar->initDialogHeader(_title, headerHeight);
    vLayout->addWidget(customTitleBar);
    this->setLayout(vLayout);

    //body
    QHBoxLayout* bodyHLayout = new QHBoxLayout;
    bodyHLayout->addSpacing(spacing * 4);

    if(icon != nullptr)
    {
        QLabel* picLabel = new QLabel;
        QPixmap pixmap = icon->pixmap(QSize(iconSide, iconSide));
        picLabel->setPixmap(pixmap);
        bodyHLayout->addWidget(picLabel, 0, Qt::AlignLeft);
        bodyHLayout->addSpacing(spacing);
    }

    bodyHLayout->addWidget(messageLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    bodyHLayout->addStretch();
    vLayout->addLayout(bodyHLayout);
    customTitleBar->setWidth(this->width() - spacing * 2);

    //footer
    QHBoxLayout* footerHLayout = new QHBoxLayout;
    footerHLayout->addStretch();

    if(_yesBtn != NONE || (_yesBtn == NONE && _noBtn == NONE))
    {
        QToolButton* yesButton = new QToolButton;
        yesButton->setFixedSize(btnWidth, btnHeight);
        yesButton->setObjectName("dialogBtnOK");
        footerHLayout->addWidget(yesButton, 0, Qt::AlignRight);
        connect(yesButton, &QToolButton::clicked, this,[=]
        {
            clickedBtn = YES;
            close();
        }, Qt::AutoConnection);
    }

    if(_noBtn != NONE)
    {
        QToolButton* noButton = new QToolButton;
        noButton->setFixedSize(btnWidth, btnHeight);
        noButton->setObjectName("cancelBtn");
        footerHLayout->addWidget(noButton, 0, Qt::AlignRight);
        connect(noButton, &QToolButton::clicked, this,[=]
        {
            clickedBtn = NO;
            close();
        }, Qt::AutoConnection);
    }

    if(_dropBtn != NONE)
    {
        QToolButton* dropButton = new QToolButton;
        dropButton->setFixedSize(btnWidth, btnHeight);
        dropButton->setObjectName("dialogBtnDrop");
        footerHLayout->addWidget(dropButton, 0, Qt::AlignRight);
        connect(dropButton, &QToolButton::clicked, this,[=]
        {
            clickedBtn = DROP;
            close();
        }, Qt::AutoConnection);
    }

    footerHLayout->addSpacing(spacing * 3);
    vLayout->addLayout(footerHLayout);
    vLayout->addSpacing(spacing);
}

//отрисовка background слоя
void MsgBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QPixmap pixmap;

    pixmap = QPixmap(":/images/dialogBackgroundGraphStyle.png").scaled(this->width(),
                     this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    painter.setRenderHint(QPainter::Antialiasing, true);
    QBrush brush = QBrush(pixmap);
    painter.setBrush(brush);

    //rounded background
    painter.drawPixmap(0,0,pixmap.width(),pixmap.height(), pixmap);
}

void MsgBox::closeEvent(QCloseEvent *closeEvent)
{
    Q_UNUSED(closeEvent);
    emit finished();
}

