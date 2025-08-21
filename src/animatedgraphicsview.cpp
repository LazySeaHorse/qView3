#include "animatedgraphicsview.h"
#include "qvapplication.h"
#include "settingsmanager.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QShowEvent>
#include <QPainter>
#include <QEasingCurve>

AnimatedGraphicsView::AnimatedGraphicsView(QWidget *parent)
    : QVGraphicsView(parent)
{
    scaleAnim = new QVariantAnimation(this);
    scaleAnim->setEasingCurve(QEasingCurve::OutCubic);
    opacityAnim = new QPropertyAnimation(parentWidget() ? parentWidget() : this, "windowOpacity", this);
    opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
    animGroup = new QParallelAnimationGroup(this);
    animGroup->addAnimation(scaleAnim);
    animGroup->addAnimation(opacityAnim);

    isAnimatingClose = false;

    connect(animGroup, &QParallelAnimationGroup::finished, this, [this]{
        if (isAnimatingClose)
            emit closeAnimationFinished();
    });
}

int AnimatedGraphicsView::getDurationMs() const
{
    return qvApp->getSettingsManager().getInteger("opencloseanimduration");
}

void AnimatedGraphicsView::showEvent(QShowEvent *event)
{
    QVGraphicsView::showEvent(event);
    if (!qvApp->getSettingsManager().getBoolean("opencloseanimenabled"))
        return;

    startOpenAnimation();
}

void AnimatedGraphicsView::startOpenAnimation()
{
    isAnimatingClose = false;
    baseTransform = transform();
    const qreal startScale = 0.15;

    setTransform(QTransform(baseTransform).scale(startScale, startScale));
    QWidget *host = window();
    if (host)
        host->setWindowOpacity(0.0);

    const int duration = getDurationMs();
    scaleAnim->stop();
    scaleAnim->setStartValue(startScale);
    scaleAnim->setEndValue(1.0);
    scaleAnim->setDuration(duration);
    connect(scaleAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v){
        const qreal s = v.toReal();
        setTransform(QTransform(baseTransform).scale(s, s));
    });

    opacityAnim->stop();
    opacityAnim->setTargetObject(host ? host : this);
    opacityAnim->setPropertyName("windowOpacity");
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);
    opacityAnim->setDuration(duration);

    animGroup->start();
}

void AnimatedGraphicsView::startCloseAnimation()
{
    if (!qvApp->getSettingsManager().getBoolean("opencloseanimenabled"))
    {
        emit closeAnimationFinished();
        return;
    }

    isAnimatingClose = true;
    baseTransform = transform();
    const int duration = getDurationMs();

    scaleAnim->stop();
    scaleAnim->setStartValue(1.0);
    scaleAnim->setEndValue(0.15);
    scaleAnim->setDuration(duration);
    disconnect(scaleAnim, nullptr, this, nullptr);
    connect(scaleAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v){
        const qreal s = v.toReal();
        setTransform(QTransform(baseTransform).scale(s, s));
    });

    QWidget *host = window();
    opacityAnim->stop();
    opacityAnim->setTargetObject(host ? host : this);
    opacityAnim->setPropertyName("windowOpacity");
    opacityAnim->setStartValue(1.0);
    opacityAnim->setEndValue(0.0);
    opacityAnim->setDuration(duration);

    animGroup->start();
}

void AnimatedGraphicsView::abortCloseAnimation()
{
    if (!isAnimatingClose)
        return;
    animGroup->stop();
    if (QWidget *host = window())
        host->setWindowOpacity(1.0);
    setTransform(baseTransform);
    isAnimatingClose = false;
}

void AnimatedGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QVGraphicsView::drawBackground(painter, rect);
    auto &settings = qvApp->getSettingsManager();
    if (!settings.getBoolean("overlayenabled"))
        return;
    const int opacity = settings.getInteger("overlayopacity");
    QColor c(0, 0, 0, qBound(0, opacity, 255));
    painter->save();
    painter->fillRect(viewport()->rect(), c);
    painter->restore();
}


