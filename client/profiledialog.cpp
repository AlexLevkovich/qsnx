/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "profiledialog.h"
#include "ui_profiledialog.h"
#include "confreader.h"
#include <QDir>
#include <QIcon>
#include <QCoreApplication>
#include <QFile>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QPushButton>
#include <QCryptographicHash>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

quint64 Profile::crypto_key = Q_UINT64_C(0x0c2ad4a1acb3f021);

Profile::Profile() : m_crypto(crypto_key) {
    m_port = 443;
    m_use_userpw = true;
}

Profile::Profile(const Profile & profile) : m_crypto(crypto_key) {
    *this = profile;
}

Profile::Profile(const QString & name) : m_crypto(crypto_key) {
    ConfReader reader(conf_file_name());
    init(&reader,name);
}

Profile::Profile(const QString & site,int port,const QString & certificate,bool backward) : m_crypto(crypto_key) {
    m_site = site;
    m_port = (port <= 1)?443:port;
    m_certificate = certificate;
    m_use_userpw = false;
    m_backward = backward;
}

Profile::Profile(const QString & site,int port,const QString & username,const QString & password,bool backward) : m_crypto(crypto_key) {
    m_site = site;
    m_port = (port <= 1)?443:port;
    m_username = username;
    m_password = password;
    m_use_userpw = true;
    m_backward = backward;
}

Profile::Profile(const QString & site,int port,const QString & username,const QString & password,const QString & certificate,bool use_userpw,bool backward) : m_crypto(crypto_key) {
    m_site = site;
    m_port = (port <= 1)?443:port;
    m_username = username;
    m_password = password;
    m_certificate = certificate;
    m_use_userpw = use_userpw;
    m_backward = backward;
}

Profile & Profile::init(ConfReader * reader,const QString & name) {
    m_site = reader->value<QString>(name+"/site","");
    m_username = reader->value<QString>(name+"/username","");
    m_password = m_crypto.decryptToString(reader->value<QString>(name+"/password",""));
    m_certificate = reader->value<QString>(name+"/certificate","");
    m_port = reader->value<int>(name+"/port",443);
    m_use_userpw = reader->value<bool>(name+"/use_userpw",true);
    m_backward = reader->value<bool>(name+"/backward",false);
    return *this;
}

const Profile Profile::defaultInstance() {
    ConfReader reader(conf_file_name());
    QString name = reader.value<QString>("/defaultProfileName","");
    if (name.isEmpty()) {
        if (!reader.sections().isEmpty()) name = reader.sections().at(0);
        else return Profile();
    }
    return Profile().init(&reader,name);
}

const QString Profile::defaultName() {
    ConfReader reader(conf_file_name());
    return reader.value<QString>("/defaultProfileName","");
}

bool Profile::setDefault(const QString & name) {
    ConfReader reader(conf_file_name());
    return reader.setValues<QString>("/defaultProfileName",QStringList() << name);
}

bool Profile::isValid() const {
    return (!m_site.isEmpty() && m_port >= 1 && m_port <= 65535 && (isUserPasswordValid() || isCertificateValid()));
}

bool Profile::isCertificateValid() const {
    return (!m_certificate.isEmpty() && QFile::exists(m_certificate) && !m_use_userpw);
}

bool Profile::isUserPasswordValid() const {
    return (!m_username.isEmpty() && m_use_userpw);
}

bool Profile::isBackwardCompatabilityEnabled() {
    return m_backward;
}

const QString Profile::user_dir() {
    uid_t uid = geteuid();
    struct passwd * pw = (uid == (uid_t)-1 && errno ? NULL : getpwuid(uid));
    if (pw == NULL) return QString();

    return QString::fromLocal8Bit(pw->pw_dir);
}

const QString Profile::conf_file_name() {
    QString userdir = user_dir();
    QString configdir = userdir + QDir::separator() +".config";
    if (!QDir(configdir).exists()) configdir = userdir;
    else {
        configdir += QDir::separator() + QCoreApplication::organizationName();
        QDir().mkpath(configdir);
    }
    return configdir + QDir::separator() + QCoreApplication::applicationName() + QLatin1String(".conf");
}

const QStringList Profile::profileNames() {
    return ConfReader(conf_file_name()).sections();
}

const QMap<QString,Profile> Profile::profiles() {
    QMap<QString,Profile> ret;
    ConfReader reader(conf_file_name());
    for (QString & section: reader.sections()) {
        ret[section] = Profile(section);
    }
    return ret;
}

bool Profile::save(const QString & name) {
    ConfReader reader(conf_file_name());
    if (!reader.setValues<QString>(name+"/site",QStringList() << m_site) ||
        !reader.setValues<QString>(name+"/username",QStringList() << m_username) ||
        !reader.setValues<QString>(name+"/password",QStringList() << m_crypto.encryptToString(m_password)) ||
        !reader.setValues<QString>(name+"/certificate",QStringList() << m_certificate) ||
        !reader.setValues<int>(name+"/port",QList<int>() << m_port) ||
        !reader.setValues<bool>(name+"/use_userpw",QList<bool>() << m_use_userpw) ||
        !reader.setValues<bool>(name+"/backward",QList<bool>() << m_backward)) return false;
    return true;
}

void Profile::remove(const QString & name) {
    ConfReader reader(conf_file_name());
    reader.remove(name+"/");
}

void Profile::clear() {
    ConfReader reader(conf_file_name());
    reader.clear();
}

Profile & Profile::operator=(const Profile &profile) {
    m_site = profile.m_site;
    m_port = profile.m_port;
    m_username = profile.m_username;
    m_password = profile.m_password;
    m_certificate = profile.m_certificate;
    m_use_userpw = profile.m_use_userpw;
    m_backward = profile.m_backward;
    return *this;
}

QString Profile::userName() const {
    return m_username;
}

QString Profile::password() const {
    return m_password;
}

QString Profile::certificate() const {
    return m_certificate;
}

QString Profile::site() const {
    return m_site;
}

int Profile::port() const {
    return m_port;
}

bool Profile::isUserPassword() const {
    return m_use_userpw;
}

void Profile::setUserName(const QString & username) {
    m_username = username;
}

void Profile::setPassword(const QString & password) {
    m_password = password;
}

void Profile::setCertificate(const QString & certificate) {
    m_certificate = certificate;
}

void Profile::setSite(const QString & site) {
    m_site = site;
}

void Profile::setPort(int port) {
    m_port = port;
}

void Profile::setUsingUserPassword(bool flag) {
    m_use_userpw = flag;
}

void Profile::setBackwardCompatabilityMode(bool flag) {
    m_backward = flag;
}

ProfileDialog::ProfileDialog(bool has_backward_compability,QWidget *parent) : QDialog(parent), ui(new Ui::ProfileDialog) {
    ui->setupUi(this);

    setWindowIcon(QIcon(QIcon::fromTheme("configure").pixmap(128)));
    ui->newProfileButton->setIcon(QIcon(QIcon::fromTheme("entry-new").pixmap(128)));
    ui->editProfileNameButton->setIcon(QIcon(QIcon::fromTheme("edit-entry").pixmap(128)));
    ui->deleteProfileButton->setIcon(QIcon(QIcon::fromTheme("entry-delete").pixmap(128)));

    QAbstractItemModel * old_model = ui->profileList->model();
    ui->profileList->setModel(new QStandardItemModel());
    if (old_model != NULL) delete old_model;
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    QMapIterator<QString,Profile> i(Profile::profiles());
    while (i.hasNext()) {
        i.next();
        ((QStandardItemModel *)ui->profileList->model())->appendRow(new QStandardItem(QIcon::fromTheme("network-vpn"),i.key()));
        m_profiles.append(i.value());
    }

    connect(ui->profileList->selectionModel(),&QItemSelectionModel::selectionChanged,this,&ProfileDialog::on_list_selectionChanged);
    connect(ui->profileList->itemDelegate(),&QAbstractItemDelegate::commitData,this,[=](QWidget * editBox) {
        QModelIndex index = ui->profileList->selectionModel()->selectedIndexes()[0];
        QString text = reinterpret_cast<QLineEdit*>(editBox)->text();
        if (contains_profile_name(text,index.row())) {
            QMetaObject::invokeMethod(ui->profileList,"edit",Qt::QueuedConnection,Q_ARG(QModelIndex,index));
            return;
        }
        if (index.row() < m_profiles.count()) m_profiles[index.row()] = Profile(ui->siteEdit->text(),ui->portSpin->value(),ui->userEdit->text(),ui->passwordEdit->text(),ui->certEdit->text(),ui->userPwRadio->isChecked(),ui->backCheck->isChecked());
        else m_profiles.append(Profile(ui->siteEdit->text(),ui->portSpin->value(),ui->userEdit->text(),ui->passwordEdit->text(),ui->certEdit->text(),ui->userPwRadio->isChecked(),ui->backCheck->isChecked()));
        on_list_selectionChanged();
        update_ok_state();
    });
    ui->backCheck->setHidden(!has_backward_compability);
}

ProfileDialog::~ProfileDialog() {
    delete ui;
}

void ProfileDialog::on_list_selectionChanged() {
    QModelIndexList selection = ui->profileList->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) {
        ui->siteEdit->setEnabled(false);
        ui->portSpin->setEnabled(false);
        ui->userEdit->setEnabled(false);
        ui->passwordEdit->setEnabled(false);
        ui->certEdit->setEnabled(false);
        ui->certRadio->setEnabled(false);
        ui->userPwRadio->setEnabled(false);
        ui->siteEdit->setText("");
        ui->portSpin->setValue(443);
        ui->userEdit->setText("");
        ui->passwordEdit->setText("");
        ui->certEdit->setText("");
        ui->editProfileNameButton->setEnabled(false);
        ui->deleteProfileButton->setEnabled(false);
        ui->backCheck->setEnabled(false);
        return;
    }
    QModelIndex index = selection[0];
    if (index.row() >= m_profiles.count()) return;
    Profile & profile =  m_profiles[index.row()];
    ui->siteEdit->setText(profile.site());
    ui->portSpin->setValue(profile.port());
    ui->userEdit->setText(profile.userName());
    ui->passwordEdit->setText(profile.password());
    ui->certEdit->setText(profile.certificate());
    ui->backCheck->setEnabled(true);
    ui->siteEdit->setEnabled(true);
    ui->portSpin->setEnabled(true);
    ui->userEdit->setEnabled(true);
    ui->passwordEdit->setEnabled(true);
    ui->certEdit->setEnabled(true);
    ui->certRadio->setEnabled(true);
    ui->userPwRadio->setEnabled(true);
    ui->editProfileNameButton->setEnabled(true);
    ui->deleteProfileButton->setEnabled(true);
    if (!profile.isUserPassword()) {
        ui->certRadio->setChecked(true);
        on_certRadio_clicked(true);
    }
    else {
        ui->userPwRadio->setChecked(true);
        on_userPwRadio_clicked(true);
    }
    ui->backCheck->setChecked(profile.isBackwardCompatabilityEnabled());
}

bool ProfileDialog::contains_profile_name(const QString & name,int current_row) const {
    for (int row=0;row<ui->profileList->model()->rowCount();row++) {
        if (row == current_row) continue;
        if (ui->profileList->model()->data(ui->profileList->model()->index(row,0)).toString() == name) return true;
    }
    return false;
}

void ProfileDialog::update_ok_state() {
    for (int row=0;row<m_profiles.count();row++) {
        if (!m_profiles[row].isValid()) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            return;
        }
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void ProfileDialog::on_newProfileButton_clicked() {
    QStandardItem * item;
    ((QStandardItemModel *)ui->profileList->model())->appendRow(item = new QStandardItem(QIcon::fromTheme("network-vpn"),tr("New connection")));
    ui->profileList->selectionModel()->select(item->index(),QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Current);
    ui->profileList->edit(item->index());
    ui->siteEdit->clear();
    ui->userEdit->clear();
    ui->passwordEdit->clear();
    ui->certEdit->clear();
    ui->portSpin->setValue(443);
    ui->userPwRadio->setChecked(true);
}

void ProfileDialog::on_deleteProfileButton_clicked() {
    QStandardItem * item = ((QStandardItemModel *)ui->profileList->model())->itemFromIndex(ui->profileList->selectionModel()->selectedIndexes()[0]);
    m_profiles.removeAt(item->row());
    ui->profileList->model()->removeRow(item->row());
    update_ok_state();
}


void ProfileDialog::on_siteEdit_textChanged(const QString & text) {
    QModelIndexList indexes = ui->profileList->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty() && indexes[0].row() < m_profiles.count()) m_profiles[indexes[0].row()].setSite(text);
    update_ok_state();
}


void ProfileDialog::on_userEdit_textChanged(const QString & text) {
    QModelIndexList indexes = ui->profileList->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty() && indexes[0].row() < m_profiles.count())m_profiles[indexes[0].row()].setUserName(text);
    update_ok_state();
}


void ProfileDialog::on_passwordEdit_textChanged(const QString & text) {
    QModelIndexList indexes = ui->profileList->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty() && indexes[0].row() < m_profiles.count())m_profiles[indexes[0].row()].setPassword(text);
    update_ok_state();
}


void ProfileDialog::on_certEdit_textChanged(const QString & text) {
    QModelIndexList indexes = ui->profileList->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty() && indexes[0].row() < m_profiles.count())m_profiles[indexes[0].row()].setCertificate(text);
    update_ok_state();
}

void ProfileDialog::on_portSpin_valueChanged(int value) {
    QModelIndexList indexes = ui->profileList->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty() && indexes[0].row() < m_profiles.count())m_profiles[indexes[0].row()].setPort(value);
    update_ok_state();
}

void ProfileDialog::on_certButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this,tr("Select a certificate file"),QDir::homePath(),tr("All Files (*.*)"));
    if (fileName.isNull()) return;
    ui->certEdit->setText(fileName);
    update_ok_state();
}


void ProfileDialog::on_buttonBox_accepted() {
    Profile::clear();
    for (int row=0;row<ui->profileList->model()->rowCount();row++) {
        m_profiles[row].save(ui->profileList->model()->data(ui->profileList->model()->index(row,0)).toString());
    }
}

void ProfileDialog::on_certRadio_clicked(bool checked) {
    m_profiles[ui->profileList->selectionModel()->selectedIndexes()[0].row()].setUsingUserPassword(!checked);
    ui->userEdit->setEnabled(!checked);
    ui->passwordEdit->setEnabled(!checked);
    ui->certEdit->setEnabled(checked);
}


void ProfileDialog::on_userPwRadio_clicked(bool checked) {
    m_profiles[ui->profileList->selectionModel()->selectedIndexes()[0].row()].setUsingUserPassword(checked);
    ui->certEdit->setEnabled(!checked);
    ui->userEdit->setEnabled(checked);
    ui->passwordEdit->setEnabled(checked);
}


void ProfileDialog::on_editProfileNameButton_clicked() {
    ui->profileList->edit(ui->profileList->selectionModel()->selectedIndexes()[0]);
}

void ProfileDialog::on_backCheck_clicked(bool checked) {
    m_profiles[ui->profileList->selectionModel()->selectedIndexes()[0].row()].setBackwardCompatabilityMode(checked);
}

