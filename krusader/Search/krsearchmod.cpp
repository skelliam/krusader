/***************************************************************************
                                krsearchmod.cpp
                            -------------------
   copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
   email                : krusader@users.sourceforge.net
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

                                                    S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "krsearchmod.h"

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QtCore/QDir>
#include <QtCore/QRegExp>

#include <kde_file.h>
#include <KLocale>
#include <KMimeType>

#include "../VFS/krquery.h"
#include "../resources.h"
#include "../VFS/vfile.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/krarchandler.h"

#define  EVENT_PROCESS_DELAY     250

KRSearchMod::KRSearchMod(const KRQuery* q)
{
    stopSearch = false; /// ===> added
    query = new KRQuery(*q);
    connect(query, SIGNAL(status(const QString &)),
            this,  SIGNAL(searching(const QString&)));
    connect(query, SIGNAL(processEvents(bool &)),
            this,  SLOT(slotProcessEvents(bool &)));

    remote_vfs = 0;
    virtual_vfs = 0;
}

KRSearchMod::~KRSearchMod()
{
    delete query;
    if (remote_vfs)
        delete remote_vfs;
    if (virtual_vfs)
        delete virtual_vfs;
}

void KRSearchMod::start()
{
    unScannedUrls.clear();
    scannedUrls.clear();
    timer.start();

    KUrl::List whereToSearch = query->searchInDirs();

    // search every dir that needs to be searched
    for (int i = 0; i < whereToSearch.count(); ++i)
        scanURL(whereToSearch [ i ]);

    emit finished();
}

void KRSearchMod::stop()
{
    stopSearch = true;
}

void KRSearchMod::scanURL(KUrl url)
{
    if (stopSearch) return;

    unScannedUrls.push(url);
    while (!unScannedUrls.isEmpty()) {
        KUrl urlToCheck = unScannedUrls.pop();

        if (stopSearch) return;

        if (query->isExcluded(urlToCheck)) {
            if (!query->searchInDirs().contains(urlToCheck))
                continue;
        }

        if (scannedUrls.contains(urlToCheck))
            continue;
        scannedUrls.push(urlToCheck);

        emit searching(urlToCheck.pathOrUrl());

        if (urlToCheck.isLocalFile())
            scanLocalDir(urlToCheck);
        else
            scanRemoteDir(urlToCheck);
    }
}

void KRSearchMod::scanLocalDir(KUrl urlToScan)
{
    QString dir = urlToScan.path(KUrl::AddTrailingSlash);

    DIR* d = opendir(dir.toLocal8Bit());
    if (!d) return ;

    KDE_struct_dirent* dirEnt;

    while ((dirEnt = KDE_readdir(d)) != NULL) {
        QString name = QString::fromLocal8Bit(dirEnt->d_name);

        // we don't scan the ".",".." enteries
        if (name == "." || name == "..") continue;

        KDE_struct_stat stat_p;
        KDE_lstat((dir + name).toLocal8Bit(), &stat_p);

        KUrl url = KUrl(dir + name);

        QString mime;
        if (query->searchInArchives() || !query->hasMimeType()) {
            KMimeType::Ptr mt = KMimeType::findByUrl(url, stat_p.st_mode, true, false);
            if (mt)
                mime = mt->name();
        }

        // creating a vfile object for matching with krquery
        vfile * vf = new vfile(name, (KIO::filesize_t)stat_p.st_size, KRpermHandler::mode2QString(stat_p.st_mode),
                               stat_p.st_mtime, S_ISLNK(stat_p.st_mode), false/*FIXME*/, stat_p.st_uid, stat_p.st_gid,
                               mime, "", stat_p.st_mode);
        vf->vfile_setUrl(url);

        if (query->isRecursive()) {
            if (S_ISLNK(stat_p.st_mode) && query->followLinks())
                unScannedUrls.push(KUrl(QDir(dir + name).canonicalPath()));
            else if (S_ISDIR(stat_p.st_mode))
                unScannedUrls.push(url);
        }
        if (query->searchInArchives()) {
            QString type = mime.right(4);
            if (mime.contains("-rar")) type = "-rar";

            if (KRarcHandler::arcSupported(type)) {
                KUrl archiveURL = url;
                bool encrypted;
                QString realType = KRarcHandler::getType(encrypted, url.path(), mime);

                if (!encrypted) {
                    if (realType == "-tbz" || realType == "-tgz" || realType == "tarz" || realType == "-tar" || realType == "-tlz")
                        archiveURL.setProtocol("tar");
                    else
                        archiveURL.setProtocol("krarc");

                    unScannedUrls.push(archiveURL);
                }
            }
        }

        if (query->match(vf)) {
            // if we got here - we got a winner
            results.append(dir + name);
            emit found(name, dir, (KIO::filesize_t) stat_p.st_size, stat_p.st_mtime,
                       KRpermHandler::mode2QString(stat_p.st_mode), stat_p.st_uid, stat_p.st_gid, query->foundText());
        }
        delete vf;

        if (timer.elapsed() >= EVENT_PROCESS_DELAY) {
            qApp->processEvents();
            timer.start();
            if (stopSearch) return;
        }
    }
    // clean up
    closedir(d);
}

void KRSearchMod::scanRemoteDir(KUrl url)
{
    vfs * vfs_;


    if (url.protocol() == "virt") {
        if (virtual_vfs == 0)
            virtual_vfs = new virt_vfs(0);
        vfs_ = virtual_vfs;
    } else {
        if (remote_vfs == 0)
            remote_vfs = new ftp_vfs(0);
        vfs_ = remote_vfs;
    }

    if (!vfs_->vfs_refresh(url)) return ;

    for (vfile * vf = vfs_->vfs_getFirstFile(); vf != 0 ; vf = vfs_->vfs_getNextFile()) {
        QString name = vf->vfile_getName();
        KUrl fileURL = vfs_->vfs_getFile(name);

        if (query->isRecursive() && ((vf->vfile_isSymLink() && query->followLinks()) || vf->vfile_isDir()))
            unScannedUrls.push(fileURL);

        if (query->match(vf)) {
            // if we got here - we got a winner
            results.append(vfs::pathOrUrl(fileURL, KUrl::RemoveTrailingSlash));

            emit found(fileURL.fileName(), vfs::pathOrUrl(fileURL.upUrl(), KUrl::RemoveTrailingSlash), vf->vfile_getSize(), vf->vfile_getTime_t(),
                       vf->vfile_getPerm(), vf->vfile_getUid(), vf->vfile_getGid(), query->foundText());
        }

        if (timer.elapsed() >= EVENT_PROCESS_DELAY) {
            qApp->processEvents();
            timer.start();
            if (stopSearch) return;
        }
    }
}

void KRSearchMod::slotProcessEvents(bool & stopped)
{
    qApp->processEvents();
    stopped = stopSearch;
}

#include "krsearchmod.moc"
