#include "qEQEcl.h"
#include "ui_qEQEcl.h"

#include <QDebug>
#include <QFileDialog>
#include <QString>
#include <QFile>
#include <QSettings>
#include <QMessageBox>
#include <QLineEdit>
#include <QThread>

#include "miniz.h"

qEQEcl::qEQEcl(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::qEQEcl)
{
    ui->setupUi(this);
    conf = new QSettings("qEQEcl", "Kai");
    find_eqpath();
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
    }
    eqgame = new QProcess();
    qDebug() << "ideal Thread Count" << QThread::idealThreadCount();
    ui->cpu_num->clear();
    if(QThread::idealThreadCount() > 0) {
        for(int i = 0; i < QThread::idealThreadCount(); ++i) {
            ui->cpu_num->addItem(QString::number(i));
        }
    } else {
        ui->cpu_num->addItem("0");
    }
    int pin_cpu = conf->value("pin_cpu", -1).toInt();
    if(pin_cpu >= 0) {
        ui->pin_cpu_check->setChecked(true);
        ui->cpu_num->setCurrentIndex(pin_cpu);
    } else {
        ui->pin_cpu_check->setChecked(false);
    }
    manager = new QNetworkAccessManager(this);
    this->connect(eqgame, SIGNAL(started()), this, SLOT(eqgame_started()));
    this->connect(eqgame, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(eqgame_finished(int, QProcess::ExitStatus)));
}

qEQEcl::~qEQEcl()
{
    update_eqpath();
    delete eqgame;
    delete ui;
}

void qEQEcl::find_eqpath()
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

void qEQEcl::update_eqpath()
{
    conf->setValue("eqpath", this->ui->eqpath_text->toPlainText());
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

void qEQEcl::on_eqpath_browse_btn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    QString new_eqpath = dialog.getExistingDirectory(this, "Everquest Directory", this->ui->eqpath_text->toPlainText(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    conf->setValue("eqpath", new_eqpath);
    qDebug() << "getExistingDirectory " << new_eqpath;
    this->ui->eqpath_text->setPlainText(new_eqpath);
    QDir eqpath(new_eqpath);
    if(!eqpath.entryList().contains("eqgame.exe", Qt::CaseInsensitive)) {
        QMessageBox::information(this, "eqgame.exe missing",
                                 "There is no eqgame.exe file in this directory.\n"
                                 "TODO: Don't be so mean about this.\n"
                                 "Maybe turn the text box color red, and be happy with that.");
    }
    else if(is_dir_case_sensitive(new_eqpath)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Case Sensitive Directory",
                                      "This filesystem is Case Sensitive. It's in your best interest to lowercase everything.\n"
                                      "Click Yes to lowercase everything recursively in the directory " + new_eqpath + "\n"
                                      "Click No if you've already done it, or you just don't wanna.",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
            lower_case_everything(new_eqpath);
    }
}

void qEQEcl::on_ini_import_btn_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    QString eqpath = dialog.getOpenFileName(this, "eqclient", conf->value("eqpath", "").toString(), "eqclient.ini");
    qDebug() << "getOpenFileName " << eqpath;
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
    if(!eqclient || !ui->ini_table->verticalHeaderItem(row))
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

void qEQEcl::on_cpu_num_activated(const QString &arg1)
{
    /* TODO: Decide if we want to make cpu_num and pin_cpu separate, or the same variable in settings. */
    qDebug() << "cpu_num_activated" << arg1;
    if(ui->pin_cpu_check->isChecked())
        conf->setValue("pin_cpu", ui->cpu_num->currentIndex());
}

void qEQEcl::on_pin_cpu_check_clicked(bool checked)
{
    /* TODO: Decide if we want to make cpu_num and pin_cpu separate, or the same variable in settings. */
    qDebug() << "on_pin_cpu_check" << checked;
    if(checked)
        conf->setValue("pin_cpu", ui->cpu_num->currentIndex());
    else
        conf->setValue("pin_cpu", -1);
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
    QStringList args;
    int pin_cpu = conf->value("pin_cpu", -1).toInt();
    qDebug() << "pin_cpu" << pin_cpu;
#if defined linux
    if(pin_cpu >= 0) {
        args << "taskset" << "-c" << QString::number(pin_cpu);
    }
    args << "wine";
#elif defined _WIN32
    if(pin_cpu >= 0) {
        QMessageBox::information(this, "CPU Affinity", "TODO: Implement CPU affinity for windows");
    }
#endif
    args << "eqgame.exe" << "patchme";
    qDebug () << "args:" << args.join(" ");
    //eqgame->start(args.join(" "));
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
    QMessageBox::information(this, "Implement Detection here", "TODO: Detection not implemented yet");
}

void qEQEcl::on_add_ini_values_btn_clicked()
{
    QMessageBox::information(this, "Implement adding values here", "TODO: Present fields for section(default current selected section), key, and value.");
}

void qEQEcl::on_p99_check_clicked(bool checked)
{
    qDebug() << "on_p99_check_clicked" << checked;
    QString eqpath(conf->value("eqpath", "").toString());
    if(eqpath == "")
        return;
    QDir eqdir(eqpath);
    if(!eqdir.entryList().contains("eqgame.exe")) {
        return;
    }
    if(!checked) {
        QMessageBox::information(this, "Implement un-doing P99 files", "TODO: We should be able to revert P99 changes.");
    } else {
        QMessageBox::information(this, "Implement downloading zip file", "TODO: Download the P99Files<num>.zip file. For now, manually download it and put it next to eqgame.exe");
#if 0
        if(checked) {
            connect(manager, SIGNAL(finished(QNetworkReply*)),
                    this, SLOT(replyFinished(QNetworkReply*)));
            manager->get(QNetworkRequest(QUrl("http://kai.gnukai.com/ip.php")));
        }
#endif
        QStringList p99list = eqdir.entryList(QStringList() << "p99files30.zip", QDir::AllEntries);
        QFileInfo p99zip;
        if(p99list.length()) {
            eqdir.rename(p99list.at(0), p99list.at(0).toLower());
            p99list.at(0).toLower();
            p99zip.setFile(eqdir.filePath(p99list.at(0).toLower()));
        }
        if(p99zip.exists()) {
            qDebug() << "Found p99 zip archive!";
            mz_zip_archive zip_archive;
            memset(&zip_archive, 0, sizeof(zip_archive));
            QByteArray q_zip_archive_name = p99zip.absoluteFilePath().toUtf8();
            qDebug() << "q_zip_archive_name" << q_zip_archive_name;
            const char *c_zip_archive_name = q_zip_archive_name.constData();
            qDebug() << "c_zip_archive_name" << c_zip_archive_name;
            mz_bool status = mz_zip_reader_init_file(&zip_archive, c_zip_archive_name, 0);
            QMap<QString, QString> unzipped;
            if(!status) {
                qDebug() << "Failed to open zip archive" << c_zip_archive_name;
                return;
            }
            for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); i++) {
                mz_zip_archive_file_stat file_stat;
                if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
                {
                    qDebug() << "mz_zip_reader_file_stat() failed!";
                    mz_zip_reader_end(&zip_archive);
                    return;
                }
                QString fname = QString::fromUtf8(file_stat.m_filename);
                mz_bool isdir = (mz_zip_reader_is_file_a_directory(&zip_archive, i));
                if(!isdir) {
                    qDebug() << "Extracting:" << fname << " -> " << fname.toLower();
                    // TODO: Do the extraction.
                    //mz_bool mz_zip_reader_extract_to_file(mz_zip_archive *pZip, mz_uint file_index, const char *pDst_filename, mz_uint flags);
                }
                //qDebug() << "Filename:" << file_stat.m_filename;
                //qDebug() << "Comment:" << file_stat.m_comment;
                //qDebug() << "Uncompressed size:" << file_stat.m_uncomp_size;
                //qDebug() << "Compressed size:" << file_stat.m_comp_size;
                //qDebug() << "Is Dir:" << mz_zip_reader_is_file_a_directory(&zip_archive, i);
            }
            mz_zip_reader_end(&zip_archive);
        }
    }
}

void qEQEcl::replyFinished(QNetworkReply* reply)
{
    /* TODO: This seems to build up QNetworkReply objects, and they all get added.... */
    reply->deleteLater();
    int v = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "status code" << v;
    QString replyText = reply->readAll();
    qDebug() << "replyText" << replyText;
}
