/****************************************************************************
**
**    This file is part of Lorris.
**    Copyright (C) 2012 Vojtěch Boček
**
**    Contact: <vbocek@gmail.com>
**             https://github.com/Tasssadar
**
**    Lorris is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    Lorris is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with Lorris.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef ANALYZERDATAAREA_H
#define ANALYZERDATAAREA_H

#include <QFrame>
#include <QHash>

#include "DataWidgets/datawidget.h"

class AnalyzerDataFile;
class AnalyzerDataStorage;
class LorrisAnalyzer;

class AnalyzerDataArea : public QFrame
{
    Q_OBJECT

Q_SIGNALS:
    void updateData();
    void mouseStatus(bool in, const data_widget_info& info, qint32 parent);

public:
    typedef QHash<quint32, DataWidget*> w_map;
    typedef QHash<quint32, QRect> mark_map;

    explicit AnalyzerDataArea(LorrisAnalyzer* analyzer, AnalyzerDataStorage* storage);
    ~AnalyzerDataArea();

    void clear();

    void SaveWidgets(AnalyzerDataFile *file);
    void LoadWidgets(AnalyzerDataFile *file, bool skip);
    static DataWidget *newWidget(quint8 type, QWidget *parent);
    DataWidget *addWidget(QPoint pos, quint8 type, bool show = true);
    void moveWidgets(QPoint diff);

    void skipNextMove() { m_skipNextMove = true; }

    DataWidget *getWidget(quint32 id);

public slots:
    void removeWidget(quint32 id);
    void updateMarker(DataWidget *w);
    
protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent *event);
    void dropEvent ( QDropEvent * event );
    void dragEnterEvent( QDragEnterEvent *event );
    void paintEvent(QPaintEvent *event);
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *);

private:
    void getMarkPos(int &x, int &y, QSize &size);
    quint32 getNewId() { return m_widgetIdCounter++; }

    w_map m_widgets;
    mark_map m_marks;
    quint32 m_widgetIdCounter;
    AnalyzerDataStorage *m_storage;
    LorrisAnalyzer* m_analyzer;

    QPoint m_mouse_orig;

    bool m_skipNextMove;
};

#endif // ANALYZERDATAAREA_H
