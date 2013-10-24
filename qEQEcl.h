#ifndef EQEMULAUNCHER_H
#define EQEMULAUNCHER_H

#include <QMainWindow>
#include <QSettings>

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
    void read_config();
    void update_config();
private slots:
    void on_eqpath_browse_btn_clicked();
    void on_ini_import_btn_clicked();
    void on_actionAbout_triggered();
    void on_eqpath_detect_btn_clicked();
    void on_add_ini_values_btn_clicked();
};

#endif // EQEMULAUNCHER_H
