#ifndef EQEMULAUNCHER_H
#define EQEMULAUNCHER_H

#include <QMainWindow>
#include <QSettings>
#include <QProcess>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include "qtfiledownload.h"

namespace Ui {
class qEQEcl;
}

class qEQEcl : public QMainWindow
{
    Q_OBJECT
    
public:
/* TODO: http://qt-project.org/faq/answer/how_can_i_create_a_one-line_qtextedit */
    explicit qEQEcl(QWidget *parent = 0);
    ~qEQEcl();
    
private:
    Ui::qEQEcl *ui;
    QSettings *conf;
    QSettings *eqclient;
    QProcess *eqgame;
    QtDownload *dl;
    void find_eqpath();
    void update_eqpath();
    void update_table(const QString &section);
    bool set_eqpath(const QString &path);
    void unpack_p99_files(const QString &p99zip_path);
#ifdef unix
    QStringList wine_eq_dir(const QString &path);
#endif
    void parse_ini();
private slots:
    void on_eqpath_browse_btn_clicked();
    void on_ini_import_btn_clicked();
    void on_actionAbout_triggered();
    void on_eqpath_detect_btn_clicked();
    void on_add_ini_values_btn_clicked();
    void on_ini_section_box_activated(const QString &arg1);
    void on_ini_table_cellChanged(int row, int column);
    void on_run_btn_clicked();
    void on_cpu_num_activated(const QString &arg1);
    void on_pin_cpu_check_clicked(bool checked);
    void on_p99_check_clicked(bool checked);
    void p99_download_done();
    void on_ini_reload_btn_clicked();

public slots:
    void eqgame_started();
    void eqgame_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void replyFinished(QNetworkReply*);
};

#endif // EQEMULAUNCHER_H
