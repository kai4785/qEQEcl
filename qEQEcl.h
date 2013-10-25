#ifndef EQEMULAUNCHER_H
#define EQEMULAUNCHER_H

#include <QMainWindow>
#include <QSettings>
#include <QProcess>

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
    void read_config();
    void update_config();
    void update_table(const QString &section);
private slots:
    void on_eqpath_browse_btn_clicked();
    void on_ini_import_btn_clicked();
    void on_actionAbout_triggered();
    void on_eqpath_detect_btn_clicked();
    void on_add_ini_values_btn_clicked();
    void on_ini_section_box_activated(const QString &arg1);
    void on_ini_table_cellChanged(int row, int column);
    void on_run_btn_clicked();
public slots:
    void eqgame_started();
    void eqgame_finished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // EQEMULAUNCHER_H
