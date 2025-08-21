#ifndef ANIMATEDGRAPHICSVIEW_H
#define ANIMATEDGRAPHICSVIEW_H

#include "qvgraphicsview.h"

#include <QVariantAnimation>
#include <QParallelAnimationGroup>

class QPropertyAnimation;

class AnimatedGraphicsView : public QVGraphicsView
{
    Q_OBJECT

public:
    explicit AnimatedGraphicsView(QWidget *parent = nullptr);

    void startCloseAnimation();
    void abortCloseAnimation();

signals:
    void closeAnimationFinished();

protected:
    void showEvent(QShowEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void startOpenAnimation();
    int getDurationMs() const;

    QVariantAnimation *scaleAnim;
    QPropertyAnimation *opacityAnim;
    QParallelAnimationGroup *animGroup;

    QTransform baseTransform;
    bool isAnimatingClose;
};

#endif // ANIMATEDGRAPHICSVIEW_H


