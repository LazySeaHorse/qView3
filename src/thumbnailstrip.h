#ifndef THUMBNAILSTRIP_H
#define THUMBNAILSTRIP_H

#include <QWidget>
#include <QListView>
#include <QStringListModel>
#include <QPropertyAnimation>
#include <QTimer>

class ThumbnailStrip : public QWidget
{
    Q_OBJECT

public:
    explicit ThumbnailStrip(QWidget *parent = nullptr);

    void setFiles(const QStringList &paths, int currentIndex);
    void setPinned(bool pinned);
    bool isPinned() const { return pinned; }
    void setTimeout(int ms);

    void showAnimated();
    void hideAnimated();

signals:
    void thumbnailActivated(const QString &path);
    void requestHide();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void resetIdleTimer();

    QListView *listView;
    QStringListModel model;
    QPropertyAnimation *posAnim;
    QPropertyAnimation *opacityAnim;
    QTimer idleTimer;
    bool pinned;
};

#endif // THUMBNAILSTRIP_H


