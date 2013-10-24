#include "qEQEcl.h"
#include "ui_qEQEcl.h"

#include <QDebug>
#include <QFileDialog>
#include <QString>
#include <QSettings>
#include <QMessageBox>
#include <QLineEdit>

qEQEcl::qEQEcl(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::qEQEcl)
{
    ui->setupUi(this);
    conf = new QSettings("qEQEcl", "Kai");
    read_config();
    /* TODO: If eqclient.ini found, go ahead and load it up */
    eqclient = 0;
    QDir eqpath(conf->value("eqpath", "").toString());
    if(eqpath.exists("eqclient.ini")) {
        qDebug() << "eqclient.ini exists";
        eqclient = new QSettings(eqpath.filePath("eqclient.ini"), QSettings::IniFormat);
        qDebug() << eqclient->childGroups();
        ui->ini_table->clear();
        ui->ini_table->setRowCount(0);
        ui->ini_table->setColumnCount(1);
        foreach(QString group, eqclient->childGroups()) {
            qDebug() << "Group:" << group;
            eqclient->beginGroup(group);
            foreach(QString key, eqclient->childKeys()) {
                int n = ui->ini_table->rowCount();
                QString value = eqclient->value(key, "EMPTYVALUE").toString();
                qDebug() << n << ":" << key << ":" << value;
                ui->ini_table->insertRow(n);
                ui->ini_table->setItem(n, 0, new QTableWidgetItem(value));
                ui->ini_table->setVerticalHeaderItem(n, new QTableWidgetItem(key));
            }
            eqclient->endGroup();
        }
    } else if(eqpath.entryList().contains("eqclient.ini", Qt::CaseInsensitive)) {
        qDebug() << "eqclient.ini exists, but isn't all lowercase, and I don't want to deal with your problems right now.";
        qDebug() << eqpath.entryList().filter("eqclient.ini", Qt::CaseInsensitive);
    }
}

qEQEcl::~qEQEcl()
{
    update_config();
    delete ui;
}

void qEQEcl::read_config()
{
    QString eqpath = conf->value("eqpath", "").toString();
    qDebug() << "conf" << eqpath;
    if(eqpath == "") {
        eqpath = qgetenv("WINEPREFIX");
        qDebug() << "WINEPREFIX" << eqpath;
    }
    if(eqpath == "") {
        eqpath = QDir::currentPath();
        qDebug() << "QDir::currentPath()" << eqpath;
    }
    qDebug() << "conf" << eqpath;
    conf->setValue("eqpath", eqpath);
    /* TODO: Change color to red if eqgame.exe not found */
    this->ui->eqpath_text->setPlainText(eqpath);
}

void qEQEcl::update_config()
{
    qDebug() << "Destructor!: " << this->ui->eqpath_text->toPlainText();
    conf->setValue("eqpath", this->ui->eqpath_text->toPlainText());
}

void qEQEcl::on_eqpath_browse_btn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    QString new_eqpath = dialog.getExistingDirectory(this, "Everquest Directory", this->ui->eqpath_text->toPlainText(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    conf->setValue("eqpath", new_eqpath);
    qDebug() << "getExistingDirectory " << new_eqpath;
    this->ui->eqpath_text->setPlainText(new_eqpath);
}

void qEQEcl::on_ini_import_btn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    QString eqdir = dialog.getOpenFileName(this, "eqclient", conf->value("eqpath", "").toString(), "eqclient.ini");
    qDebug() << "getOpenFileName " << eqdir;
}

void qEQEcl::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About", "I wish I knew what I was doing...");
}

void qEQEcl::on_eqpath_detect_btn_clicked()
{
    QMessageBox::information(this, "Implement Detection here", "Detection not implemented yet");
}

void qEQEcl::on_add_ini_values_btn_clicked()
{
    QMessageBox::information(this, "Implement adding values here", "Present fields for section(default current selected section), key, and value.");
}
