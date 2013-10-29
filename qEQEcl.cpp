#include "qEQEcl.h"
#include "ui_qEQEcl.h"

#include <QDebug>
#include <QFileDialog>
#include <QString>
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
    } else if(eqpath.entryList().contains("eqclient.ini", Qt::CaseInsensitive)) {
        qDebug() << "eqclient.ini exists, but isn't all lowercase, and I don't want to deal with your problems right now.";
        qDebug() << eqpath.entryList().filter("eqclient.ini", Qt::CaseInsensitive);
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

QString case_insensitive_file(const QDir &dir, const QString file) {
    QStringList list = dir.entryList(QStringList() << file, QDir::AllEntries);
    if(list.length())
        return list[0];
    else
        return "";
}

void qEQEcl::on_p99_check_clicked(bool checked)
{
#if 0
    qDebug() << "on_p99_check_clicked" << checked;
    if(checked) {
        connect(manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(replyFinished(QNetworkReply*)));
        manager->get(QNetworkRequest(QUrl("http://kai.gnukai.com/ip.php")));
    }
#endif
    QString eqpath(conf->value("eqpath", "").toString());
    if(eqpath == "")
        return;

    QDir eqdir(eqpath);
    qDebug() << "EQCLIENT.INI ALLEntries" << eqdir.entryList(QStringList() << "EQCLIENT.INI", QDir::AllEntries);
    /* Case insensitive? */
    QFileInfo p99zip(eqdir.filePath("P99Files30.zip"));
    if(p99zip.exists()) {
        qDebug() << "Found p99 zip archive!";
        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));
        QByteArray q_zip_archive_name = p99zip.absoluteFilePath().toUtf8();
        qDebug() << "q_zip_archive_name" << q_zip_archive_name;
        const char *c_zip_archive_name = q_zip_archive_name.constData();
        qDebug() << "c_zip_archive_name" << c_zip_archive_name;
        mz_bool status = mz_zip_reader_init_file(&zip_archive, c_zip_archive_name, 0);
        QMap<QString, QDir> dirmap;
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
            QStringList fname_path = fname.split(QDir::separator(), QString::SkipEmptyParts);
            qDebug() << fname_path;
            QDir fname_dir = eqdir;
            mz_bool isdir = (mz_zip_reader_is_file_a_directory(&zip_archive, i));
            for(int j = 0; j < fname_path.length() - (int)(!isdir); ++j) {
                QString child = case_insensitive_file(fname_dir, fname_path[j]);
                if(child.length() == 0) {
                    //qDebug() << "New dir:" << fname_path.join(QDir::separator());
                    child = fname_dir.absoluteFilePath(fname_path[j]);
                } else {
                    //qDebug() << "Existing dir:" << fname_path.join(QDir::separator());
                    child = fname_dir.absoluteFilePath(child);
                }
                fname_dir = dirmap.value(child, QDir(child));
                //qDebug() << "Next dir:" << fname_dir.absolutePath();
            }
            if(!isdir) {
                QString old_filename = case_insensitive_file(fname_dir, fname_path.last());
                if(old_filename == "") {
                    old_filename = fname_dir.filePath(fname_path.last());
                } else {
                    old_filename = fname_dir.filePath(old_filename);
                }
                old_filename = eqdir.relativeFilePath(old_filename);
                qDebug() << "Extracting:" << fname << " -> " << old_filename;
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

void qEQEcl::replyFinished(QNetworkReply* reply)
{
    /* TODO: This seems to build up QNetworkReply objects, and they all get added.... */
    reply->deleteLater();
    int v = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "status code" << v;
    QString replyText = reply->readAll();
    qDebug() << "replyText" << replyText;
}
