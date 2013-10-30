#ifndef QEQECL_FOPS_H
#define QEQECL_FOPS_H
#include <QString>
#include <QStringList>
void lower_case_everything(const QString &path);
bool is_dir_case_sensitive(const QString &path);
int backup_file(const QString &fname, const QString &path);
bool restore_files(const QStringList &fnames, const QString &path);
#ifdef unix
QStringList wine_eq_dir(const QString &path);
#endif
#endif // QEQECL_FOPS_H
