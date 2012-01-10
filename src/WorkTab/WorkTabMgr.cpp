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


#include "WorkTabMgr.h"
#include "WorkTabInfo.h"
#include "ui/HomeTab.h"
#include "ui/tabdialog.h"

#include "LorrisProbe/lorrisprobeinfo.h"
#include "LorrisTerminal/lorristerminalinfo.h"
#include "LorrisAnalyzer/lorrisanalyzerinfo.h"

WorkTabMgr::WorkTabMgr()
{
    //put ALL plugins into this vector
    m_workTabInfos.push_back(new LorrisAnalyzerInfo);
    //m_workTabInfos.push_back(new LorrisProbeInfo);
    m_workTabInfos.push_back(new LorrisTerminalInfo);

    tabIdCounter = 0;
    tabWidget = NULL;
    hometab = NULL;
}

WorkTabMgr::~WorkTabMgr()
{
    for(quint8 itr = 0; itr < m_workTabInfos.size(); ++itr)
        delete m_workTabInfos[itr];

    for(qint16 i = tabWidget->count(); tabWidget->count() && hometab == NULL;)
        removeTab(--i);

    CloseHomeTab();
    delete tabWidget;
}

std::vector<WorkTabInfo*> *WorkTabMgr::GetWorkTabInfos()
{
    return &m_workTabInfos;
}

quint16 WorkTabMgr::AddWorkTab(WorkTab *tab, QString label)
{
    quint16 id = tabIdCounter++;

    m_workTabs.insert(std::make_pair<quint16, WorkTab*>(id, tab));
    tab->setId(id);

    tab->setParent(tabWidget);
    quint16 index = tabWidget->addTab(tab, label);
    if(tabWidget->count() > 1)
    {
        tabWidget->setTabsClosable(true);
        tabWidget->setCurrentIndex(index);
    }
    CloseHomeTab();
    return id;
}

void WorkTabMgr::removeTab(WorkTab *tab)
{
    tabWidget->removeTab(tabWidget->indexOf(tab));
    m_workTabs.erase(tab->getId());
    if(tabWidget->count() == 0)
    {
        OpenHomeTab();
        tabWidget->setTabsClosable(false);
    }
    delete tab;
}

void WorkTabMgr::OpenHomeTab()
{
    hometab = new HomeTab(tabWidget);
    tabWidget->addTab(hometab, QObject::tr("Home"));
}

void WorkTabMgr::CloseHomeTab()
{
    if(!hometab)
        return;
    tabWidget->removeTab(tabWidget->indexOf(hometab));
    delete hometab;
    hometab = NULL;
}

void WorkTabMgr::NewTabDialog()
{
    TabDialog *dialog = new TabDialog((QWidget*)tabWidget->parent());
    dialog->exec();
    delete dialog;
}
