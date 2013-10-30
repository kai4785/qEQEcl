#include "fops.h"

#include <QDebug>
#include <QDir>

void lower_case_everything(const QString &path)
{
    qDebug() << "lower_case_everything" << path;
    QDir dir(path);
    QStringList files = dir.entryList(QDir::Files);
    QString sub_path, file;
    foreach(file, files) {
        qDebug() << "mv" << file << file.toLower();
        dir.rename(file, file.toLower());
    }
    QStringList dirs = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    foreach(sub_path, dirs) {
        qDebug() << "mv" << sub_path << sub_path.toLower();
        dir.rename(sub_path, sub_path.toLower());
        qDebug() << "abs" << dir.absoluteFilePath(sub_path.toLower());
        lower_case_everything(dir.absoluteFilePath(sub_path.toLower()));
    }
}


bool is_dir_case_sensitive(const QString &path)
{
    QDir dir(path);
    if(!dir.exists())
        return false;
    QFile upper(dir.absoluteFilePath("TESTFILENAMECASE.TXT"));
    upper.open(QIODevice::WriteOnly);
    upper.close();
    QFile lower(dir.absoluteFilePath("testfilenamecase.txt"));
    bool retval = !lower.exists();
    upper.remove();
    qDebug() << "is_dir_case_sensitive" << path << retval;
    return retval;
}

int backup_file(const QString &fname, const QString &path)
{
    QFileInfo finfo(fname);
    QDir dir(path);
    bool retval = -1;
    //QDir::cleanPath(path1 + QDir::separator() + path2);
    if(finfo.exists()) {
        qDebug() << fname << "file already exists";
        QFileInfo backup(path + QString(QDir::separator()) + fname);
        QDir backup_dir = backup.absoluteDir();
        if(!backup_dir.exists()) {
            backup_dir.mkpath(backup_dir.absolutePath());
        }
        //qDebug() << "mv" << fname << backup.dir();
        //qDebug() << "mv" << fname << backup.filePath();
        //qDebug() << "mv" << fname << backup.path();
        bool success = QFile::rename(finfo.filePath(), backup.filePath());
        qDebug() << "rename (" << success << ")" << finfo.filePath() << "->" << backup.filePath();
        if(success)
            retval = 1;
        else
            retval = -1;
    } else {
        qDebug() << finfo.filePath() << "file does not exist";
        retval = 0;
    }
    return retval;
}

bool restore_files(const QStringList &fnames, const QString &path)
{
    bool retval = false;
    QString fname;
    foreach(fname, fnames) {
        QString backup(path + QString(QDir::separator()) + fname);
        qDebug() << "rm" << fname;
        QFile::remove(fname);
        qDebug() << "mv" << backup << "->" << fname;
        QFile::rename(backup, fname);
    }
    return retval;
}

