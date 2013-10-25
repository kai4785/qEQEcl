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
        ui->ini_section_box->clear();
        QStringList groups = eqclient->childGroups();
        ui->ini_section_box->addItems(groups);
        update_table(groups[0]);
    } else if(eqpath.entryList().contains("eqclient.ini", Qt::CaseInsensitive)) {
        qDebug() << "eqclient.ini exists, but isn't all lowercase, and I don't want to deal with your problems right now.";
        qDebug() << eqpath.entryList().filter("eqclient.ini", Qt::CaseInsensitive);
    }
    eqgame = new QProcess();
    this->connect(eqgame, SIGNAL(started()), this, SLOT(eqgame_started()));
    this->connect(eqgame, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(eqgame_finished(int, QProcess::ExitStatus)));
}

qEQEcl::~qEQEcl()
{
    update_config();
    delete eqgame;
    delete ui;
}

void qEQEcl::read_config()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString eqpath = conf->value("eqpath", "").toString();
    QString wineprefix = conf->value("wineprefix", "").toString();
    if(wineprefix == "") {
        wineprefix = env.value("WINEPREFIX", "");
        conf->setValue("wineprefix", wineprefix);
        qDebug() << "WINEPREFIX" << eqpath;
    }
    if(eqpath == "") {
        eqpath = wineprefix;
    }
    if(eqpath == "") {
        eqpath = QDir::currentPath();
        qDebug() << "QDir::currentPath()" << eqpath;
    }
    conf->setValue("eqpath", eqpath);
    /* TODO: Change color to red if eqgame.exe not found */
    this->ui->eqpath_text->setPlainText(eqpath);
}

void qEQEcl::update_config()
{
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

void qEQEcl::update_table(const QString &section)
{
    if(!eqclient)
        return;
    ui->ini_table->clear();
    ui->ini_table->setRowCount(0);
    eqclient->beginGroup(section);
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

void qEQEcl::on_ini_table_cellChanged(int row, int column)
{
    /* TODO: Find a better check to ignore the clear() */
    if(!ui->ini_table->verticalHeaderItem(row))
        return;
    QString section = ui->ini_section_box->currentText();
    QString key = ui->ini_table->verticalHeaderItem(row)->text();
    QString value = ui->ini_table->item(row, column)->text();
    qDebug() << "Cell Changed!" << row << column << section << key << value;
    eqclient->beginGroup(section);
    eqclient->setValue(key, value);
    eqclient->endGroup();
}

void qEQEcl::on_ini_section_box_activated(const QString &arg1)
{
   qDebug() << "on_ini_section_box_activate" << arg1;
   update_table(arg1);
}

void qEQEcl::on_run_btn_clicked()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString wineprefix = conf->value("wineprefix", "").toString();
    QString eqpath = conf->value("eqpath", "").toString();
    if(wineprefix != "") {
        env.insert("WINEPREFIX", wineprefix);
    }
    eqgame->setProcessEnvironment(env);
    eqgame->setWorkingDirectory(eqpath);
    //eqgame->start("wine", QStringList() << "eqgame.exe" << "patchme");
    eqgame->start("winecfg", QStringList());
}

void qEQEcl::eqgame_started()
{
    /* TODO: Only allow one instance please */
    qDebug() << "eqgame_started";
}

void qEQEcl::eqgame_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    /* TODO: Only allow one instance please */
    qDebug() << "eqgame_finished" << exitCode << exitStatus;
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
