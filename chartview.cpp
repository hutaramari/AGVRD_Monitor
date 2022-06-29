/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chartview.h"
#include <QtGui/QMouseEvent>
#include <QtCore/QDebug>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

ChartView::ChartView(QWidget *parent)
    : QChartView(parent)
{
}

//![1]
void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_PageUp:
        chart()->zoomIn();
        break;
    case Qt::Key_PageDown:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-5.0, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(5.0, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 5.0);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -5.0);
        break;
    case Qt::Key_Space:
//        switchChartType();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0)
    {
        chart()->zoomIn();
    }
    else {
        chart()->zoomOut();
    }
}
//![1]

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    mousePos = this->chart()->mapToValue(event->pos());
    QChartView::mouseMoveEvent(event);
    //emit signalMousePos(mousePos);
}
