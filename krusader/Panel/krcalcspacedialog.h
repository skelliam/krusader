/***************************************************************************
                          krcalcspacedialog.h  -  description
                             -------------------
    begin                : Fri Jan 2 2004
    copyright            : (C) 2004 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRCALCSPACEDIALOG_H
#define KRCALCSPACEDIALOG_H

/* --=={ Patch by Heiner <h.eichmann@gmx.de> }==-- */

// KDE Includes
#include <kdialog.h>
#include <QtCore/QMutex>
#include <kio/jobclasses.h>
// Qt Includes
#include <QtCore/QThread>
#include <QLabel>
// Krusader Includes
#include "../VFS/vfs.h"
class KrPanel;
class KrView;


/**
 * Dialog calculating showing the number of files and directories and its total
 * size in a dialog. If wanted, the dialog appears after 3 seconds of
 * calculation, to avoid a short appearance if the result was found quickly.
 * Computes the result in a different thread.
 */
class KrCalcSpaceDialog : public KDialog
{
    Q_OBJECT

    friend class CalcThread;
    class QTimer * m_pollTimer;

    class CalcThread : public QThread
    {
        KIO::filesize_t m_totalSize;
        KIO::filesize_t  m_currentSize;
        unsigned long m_totalFiles;
        unsigned long m_totalDirs;
        const QStringList m_items;
        QHash <QString, KIO::filesize_t> m_sizes;
        KUrl m_url;
        mutable QMutex m_mutex;
        bool m_stop;

    public:
        CalcThread(KUrl url, const QStringList & items);

        KIO::filesize_t getItemSize(QString item) const;
        void updateItems(KrView *view) const;
        void getStats(KIO::filesize_t  &totalSize,
                      unsigned long &totalFiles,
                      unsigned long &totalDirs) const;
        void run(); // start calculation
        void stop(); // stop it. Thread continues until vfs_calcSpace returns
    } * m_thread;

    QLabel * m_label;
    bool m_autoClose; // true: wait 3 sec. before showing the dialog. Close it, when done
    bool m_canceled; // true: cancel was pressed
    int m_timerCounter; // internal counter. The timer runs faster as the rehresh (see comment there)
    const QStringList m_items;
    KrView *m_view;

    void calculationFinished(); // called if the calculation is done
    void showResult(); // show the current result in teh dialog

protected slots:
    void timer(); // poll timer was fired
    void slotCancel(); // cancel was pressed

public:
    // autoclose: wait 3 sec. before showing the dialog. Close it, when done
    KrCalcSpaceDialog(QWidget *parent, KrPanel * panel, const QStringList & items, bool autoclose);
    ~KrCalcSpaceDialog();

    void getStats(KIO::filesize_t  &totalSize,
                  unsigned long &totalFiles,
                  unsigned long &totalDirs) const {
        m_thread->getStats(totalSize, totalFiles, totalDirs);
    }
    bool wasCanceled() const {
        return m_canceled;
    } // cancel was pressed; result is probably wrong

public slots:
    void exec(); // start calculation
};
/* End of patch by Heiner <h.eichmann@gmx.de> */

#endif
