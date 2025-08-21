#include "thumbnailstrip.h"

#include <QHBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QFileInfo>
#include <QIcon>
#include <QTimerEvent>
#include <QEasingCurve>

ThumbnailStrip::ThumbnailStrip(QWidget *parent)
    : QWidget(parent), pinned(false)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    listView = new QListView(this);
    listView->setViewMode(QListView::IconMode);
    listView->setFlow(QListView::LeftToRight);
    listView->setResizeMode(QListView::Adjust);
    listView->setUniformItemSizes(true);
    const int icon = qRound(72 * devicePixelRatioF());
    listView->setIconSize(QSize(icon, icon));
    listView->setModel(&model);
    layout->addWidget(listView);

    connect(listView, &QListView::activated, this, [this](const QModelIndex &idx){
        emit thumbnailActivated(model.data(idx, Qt::UserRole + 1).toString());
    });
    connect(listView, &QListView::clicked, this, [this](const QModelIndex &idx){
        emit thumbnailActivated(model.data(idx, Qt::UserRole + 1).toString());
    });

    posAnim = new QPropertyAnimation(this, "pos", this);
    posAnim->setEasingCurve(QEasingCurve::OutCubic);
    opacityAnim = new QPropertyAnimation(this, "windowOpacity", this);
    opacityAnim->setEasingCurve(QEasingCurve::OutCubic);

    idleTimer.setSingleShot(true);
    setTimeout(3000);
    connect(&idleTimer, &QTimer::timeout, this, [this]{ if (!pinned) hideAnimated(); });
}

void ThumbnailStrip::setFiles(const QStringList &paths, int currentIndex)
{
    // Build simple model with display role as file name, decoration role as icon, user role path
    model.setStringList(QStringList());

    QAbstractItemModel *m = &model;
    QStringList names;
    names.reserve(paths.size());
    for (const QString &p : paths)
        names << QFileInfo(p).fileName();
    model.setStringList(names);
    const int iconTarget = qRound(72 * devicePixelRatioF());
    for (int i = 0; i < names.size(); ++i)
    {
        const QString &p = paths.at(i);
        const QModelIndex idx = model.index(i, 0);
        m->setData(idx, p, Qt::UserRole + 1);
        QPixmap thumb(p);
        if (!thumb.isNull())
        {
            QPixmap scaled = thumb.scaled(iconTarget, iconTarget, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m->setData(idx, QIcon(scaled), Qt::DecorationRole);
        }
    }
    if (currentIndex >= 0 && currentIndex < model.rowCount())
        listView->setCurrentIndex(model.index(currentIndex, 0));
}

void ThumbnailStrip::setPinned(bool value)
{
    pinned = value;
    resetIdleTimer();
}

void ThumbnailStrip::setTimeout(int ms)
{
    idleTimer.setInterval(ms);
    resetIdleTimer();
}

void ThumbnailStrip::showAnimated()
{
    show();
    raise();
    const int h = height();
    const QPoint endPos(0, parentWidget()->height() - h);
    const QPoint startPos(0, parentWidget()->height());

    move(startPos);
    setWindowOpacity(0.0);

    posAnim->stop();
    posAnim->setDuration(200);
    posAnim->setStartValue(startPos);
    posAnim->setEndValue(endPos);

    opacityAnim->stop();
    opacityAnim->setDuration(200);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);

    posAnim->start();
    opacityAnim->start();
    resetIdleTimer();
}

void ThumbnailStrip::hideAnimated()
{
    if (!isVisible())
        return;
    const int h = height();
    const QPoint startPos = pos();
    const QPoint endPos(0, parentWidget()->height());

    posAnim->stop();
    posAnim->setDuration(200);
    posAnim->setStartValue(startPos);
    posAnim->setEndValue(endPos);

    opacityAnim->stop();
    opacityAnim->setDuration(200);
    opacityAnim->setStartValue(windowOpacity());
    opacityAnim->setEndValue(0.0);

    connect(posAnim, &QPropertyAnimation::finished, this, [this]{ hide(); emit requestHide(); });
    posAnim->start();
    opacityAnim->start();
}

void ThumbnailStrip::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);
    resetIdleTimer();
}

void ThumbnailStrip::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    resetIdleTimer();
}

void ThumbnailStrip::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    move(0, parentWidget()->height() - height());
}

void ThumbnailStrip::resetIdleTimer()
{
    if (!pinned)
        idleTimer.start();
    else
        idleTimer.stop();
}


