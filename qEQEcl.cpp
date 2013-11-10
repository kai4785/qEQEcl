#include "qEQEcl.h"
#include "ui_qEQEcl.h"
#include "fops.h"

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
    conf = new QSettings("qEQEcl", "Settings");
    current_profile = conf->value("default_profile", "").toString();
    profile_list = conf->value("profiles", QStringList()).toStringList();
    qDebug() << "Profile list:" << profile_list << "length:" << profile_list.length();
    if(current_profile == "") {
        current_profile = "default";
        conf->setValue("default_profile", "default");
    }
    if(profile_list.length() == 0)
    {
        profile_list << current_profile;
        profile_list << "other_profile";
        conf->setValue("profiles", profile_list);
    }
    ui->menuProfile->addAction("Add", this, SLOT(add_new_profile()));
    ui->menuProfile->addSeparator();
    foreach(QString p, profile_list) {
        ui->menuProfile->addAction(p, this, SLOT(switch_profile()));
    }
    init();
    this->connect(eqgame, SIGNAL(started()), this, SLOT(eqgame_started()));
    this->connect(eqgame, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(eqgame_finished(int, QProcess::ExitStatus)));
}

void qEQEcl::add_new_profile()
{
    QObject *sender = QObject::sender();
    qDebug() << "add_new_profile" << sender->objectName();
}

void qEQEcl::switch_profile()
{
    QObject *sender = QObject::sender();
    qDebug() << "switch_profile()" << sender->objectName();
}

void qEQEcl::init()
{
    eqclient = 0;
    conf->beginGroup(current_profile);
    QString eqpath = conf->value("eqpath", "").toString();
    if(eqpath != "") {
        set_eqpath(eqpath);
    } else {
        int prompt_val = 0;
        do {
            prompt_val = prompt_eqpath();
        } while(prompt_val = 0);
    }
    qDebug() << "Constructor eqpath:" << eqpath;
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
    ui->p99_check->setChecked(conf->value("p99", false).toBool());
    QString backup_dir = conf->value("p99_backup_dir", "").toString();
    if(backup_dir == "") {
        conf->setValue("p99_backup_dir", "p99_backup");
    }
}

qEQEcl::~qEQEcl()
{
    delete eqgame;
    delete ui;
}

QString qEQEcl::find_eqpath()
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
        /* TODO: Do a search for eqgame.exe in windows path C:\Program Files*\Sony\EverQuest\ */
    }
    return eqpath;
}

#ifdef unix
QStringList qEQEcl::wine_eq_dir(const QString &path)
{
    // Breakup path, and search for drive_[a-z]
    // Join the right side do $1:join("\\") for windows path
    // Left side becomes WINEPREFIX
    QStringList retval;
    QString folder;
    qDebug() << "wine_eq_dir" << path;
    int drive_index = path.indexOf("drive_");
    if(drive_index >= 0) {
        qDebug() << "drive_ is in the path" << drive_index;
        qDebug() << "Left" << path.left(drive_index);
        retval << path.left(drive_index - 1);
        qDebug() << "Right" << path.mid(drive_index + 7);
        retval << path.mid(drive_index + 6, 1).toUpper() + ":" + path.mid(drive_index + 7).replace('/', '\\');
        qDebug() << "Drive Letter" << path.mid(drive_index + 6, 1).toUpper();
    } else {
        retval << path << path;
    }
    qDebug() << retval;
    return retval;
}
#endif

bool qEQEcl::set_eqpath(const QString &path)
{
    QDir eqdir(path);
    QString win_eqpath;
#ifdef unix
    QStringList eqpaths = wine_eq_dir(path);
    win_eqpath = eqpaths[1];
    conf->setValue("wineprefix", eqpaths[0]);
    conf->setValue("win_eqpath", eqpaths[1]);
    this->ui->wineprefix_text->setPlainText(eqpaths[0]);
#else
    conf->setValue("win_eqpath", path);
    win_eqpath = path;
#endif
    conf->setValue("eqpath", path);
    this->ui->eqpath_text->setPlainText(win_eqpath);
    if(!eqdir.entryList().contains("eqgame.exe", Qt::CaseInsensitive)) {
        QMessageBox::information(this, "eqgame.exe missing",
                                 "There is no eqgame.exe file in this directory.\n"
                                 "TODO: Don't be so mean about this.\n"
                                 "Maybe turn the text box color red, and be happy with that.");
        return false;
    }
    QDir::setCurrent(path);
    bool need_lower = !conf->value("eqpath_is_lower", false).toBool();
    qDebug() << "need_lower from conf" << need_lower;
    if(need_lower) {
        need_lower = is_dir_case_sensitive(path);
    }
    qDebug() << "need_lower after is_dir_case_sensitive" << need_lower;
    if(need_lower) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Case Sensitive Directory",
                                      "This filesystem is Case Sensitive. It's in your best interest to lowercase everything.\n"
                                      "Click Yes to lowercase everything recursively in the directory " + path + "\n"
                                      "Click No if you've already done it, or you just don't wanna.",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            lower_case_everything(path);
            need_lower = false;
        }
    }
    conf->setValue("eqpath_is_lower", !need_lower);
    qDebug() << "eqpath_is_lower" << conf->value("eqpath_is_lower").toBool();
    parse_ini();
    return true;
}

void qEQEcl::parse_ini()
{
    QString eqpath = conf->value("eqpath", "").toString();
    if(eqpath == "")
        return;
    QDir eqdir(eqpath);
    if(!eqdir.exists())
        return;
    if(eqclient)
    {
        delete eqclient;
        eqclient = 0;
    }
    if(eqdir.exists("eqclient.ini")) {
        eqclient = new QSettings(eqdir.filePath("eqclient.ini"), QSettings::IniFormat);
        qDebug() << eqclient->childGroups();
        ui->ini_table->clear();
        ui->ini_table->setRowCount(0);
        ui->ini_table->setColumnCount(1);
        ui->ini_section_box->clear();
        QStringList groups = eqclient->childGroups();
        ui->ini_section_box->addItems(groups);
        update_table(groups[0]);
        ui->ini_table->setHorizontalHeaderLabels(QStringList() << "Value");
    } else {
        ui->ini_table->clear();
        ui->ini_table->setRowCount(0);
        ui->ini_table->setColumnCount(1);
        ui->ini_section_box->clear();
        ui->ini_section_box->addItem("NoINI");
        ui->ini_table->insertRow(0);
        ui->ini_table->setVerticalHeaderItem(0, new QTableWidgetItem("No eqclient.ini"));
        ui->ini_table->setItem(0, 0, new QTableWidgetItem("!"));
        ui->ini_table->setHorizontalHeaderLabels(QStringList() << "Value");
    }
}

void qEQEcl::on_eqpath_browse_btn_clicked()
{
    int prompt_val = 0;
    do {
        prompt_val = prompt_eqpath();
    } while(prompt_val == 0);
}

/* Return value should be:
 * -1 for cancel
 * 0 for retry
 * 1 for success
 */
int qEQEcl::prompt_eqpath()
{
    int retval = 0;
    QFileDialog dialog(this, "EverQuest Directory (with eqgame.exe)");
    QString eqpath = conf->value("eqpath", "").toString();
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
    dialog.setDirectory(eqpath);
    if(dialog.exec()) {
        QStringList selected_files = dialog.selectedFiles();
        qDebug() << "Selected Files:" << selected_files;
        QString new_eqpath = selected_files[0];
        qDebug() << "getExistingDirectory " << new_eqpath;
        if(set_eqpath(new_eqpath))
            retval = 1;
        else
            retval = 0;
    } else {
        qDebug() << "Cancel button pressed";
        retval = -1;
    }
    return retval;
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
        ui->ini_table->setVerticalHeaderItem(n, new QTableWidgetItem(key));
        ui->ini_table->setItem(n, 0, new QTableWidgetItem(value));
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
    if(eqclient) {
        QStringList groups = eqclient->childGroups();
        if(groups.length() && groups.contains(section)) {
            eqclient->beginGroup(section);
            eqclient->setValue(key, value);
            eqclient->endGroup();
        }
    }
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
        qDebug() << "Adding WINEPREFIX =" << wineprefix;
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
    eqgame->start(args.join(" "));
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
    //eqpath = find_eqpath();
}

void qEQEcl::on_add_ini_values_btn_clicked()
{
    QMessageBox::information(this, "Implement adding values here", "TODO: Present fields for section(default current selected section), key, and value.");
}

void qEQEcl::unpack_p99_files(const QString &p99zip_path) {
    QFileInfo p99zip(p99zip_path);
    if(p99zip.exists()) {
        qDebug() << "Found p99 zip archive!";
        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));
        QByteArray q_zip_archive_name = p99zip.absoluteFilePath().toUtf8();
        qDebug() << "q_zip_archive_name" << q_zip_archive_name;
        const char *c_zip_archive_name = q_zip_archive_name.constData();
        qDebug() << "c_zip_archive_name" << c_zip_archive_name;
        mz_bool status = mz_zip_reader_init_file(&zip_archive, c_zip_archive_name, 0);
        QStringList backedup;
        QStringList p99_files;
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
                int backup_status = backup_file(fname.toLower(), "p99_backup");
                if(backup_status == 1) { // Backup success
                    qDebug() << "Backup success";
                    backedup << fname.toLower();
                } else if(backup_status == 0) { // File didn't exist to backup
                    qDebug() << "Backup not required";
                } else { // Some error or some other error
                    qDebug() << "Backup failed";
                }
                QByteArray bnew_file = fname.toLower().toUtf8();
                const char *cnew_file = bnew_file.constData();
                mz_bool success = true;
                success = mz_zip_reader_extract_to_file(&zip_archive, i, cnew_file, 0);
                if(success) {
                    qDebug() << "Extracted:" << fname << " -> " << fname.toLower();
                    p99_files << fname.toLower();
                } else {
                    // TODO: We should probably restore all the files and give up.
                    qDebug() << "Failed to extract:" << fname << " -> " << fname.toLower();
                    mz_zip_reader_end(&zip_archive);
                }
            }
            //qDebug() << "Filename:" << file_stat.m_filename;
            //qDebug() << "Comment:" << file_stat.m_comment;
            //qDebug() << "Uncompressed size:" << file_stat.m_uncomp_size;
            //qDebug() << "Compressed size:" << file_stat.m_comp_size;
            //qDebug() << "Is Dir:" << mz_zip_reader_is_file_a_directory(&zip_archive, i);
        }
        mz_zip_reader_end(&zip_archive);
        conf->setValue("p99", true);
        conf->setValue("p99_backedup_files", backedup);
        conf->setValue("p99_files", p99_files);
    } else {
        qDebug() << "Failed to find p99 zip file:" << p99zip_path;
    }
}

void qEQEcl::on_p99_check_clicked(bool checked)
{
    //TODO: Don't run this while EQ is running! Even if you are unzipping files that are exactly the same, it will(might) cause EQ to crash
    qDebug() << "on_p99_check_clicked" << checked;
    QString backup_dir = conf->value("p99_backup_dir", "").toString();
    if(backup_dir == "") {
        QMessageBox::information(this, "Implement backup_dir warning", "TODO: We should correctly prompt the user for backup_dir here");
        return;
    }
    QString eqpath(conf->value("eqpath", "").toString());
    if(eqpath == "")
        return;
    QDir eqdir(eqpath);
    if(!checked) {
        QMessageBox::information(this, "Implement un-doing P99 files", "TODO: We should be able to revert P99 changes.");
        QStringList backedup = conf->value("p99_backedup_files", QStringList()).toStringList();
        QStringList p99_files = conf->value("p99_files", QStringList()).toStringList();
        QString deleteme;
        foreach(deleteme, p99_files) {
            qDebug() << "Removing p99 file:" << deleteme;
            QFile::remove(deleteme);
        }
        bool success;
        success = restore_files(backedup, backup_dir);
        qDebug() << "Remove success:" << success;
        if(success) {
            conf->remove("p99_backedup_files");
            conf->remove("p99_files");
        }
    } else {
        if(eqdir.exists("p99.zip")) {
            unpack_p99_files("p99.zip");
        } else {
            if(dl) {
                QMessageBox::information(this, "Other download in progress", "TODO: Make this do the right thing.");
            }
            dl = new QtDownload;
            dl->file = "p99.zip";
            dl->url = "http://www.project1999.org/files/P99Files30.zip";
            dl->download();
            QObject::connect(dl, SIGNAL(done()), this, SLOT(p99_download_done()));
            QMessageBox::information(this, "Download running", "TODO: Progress Bar!\nDownload progress is just printing with qDebug()");
        }
    }
}
void qEQEcl::p99_download_done()
{
    dl->deleteLater();
    unpack_p99_files(dl->file);
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


void qEQEcl::on_ini_reload_btn_clicked()
{
    parse_ini();
}
