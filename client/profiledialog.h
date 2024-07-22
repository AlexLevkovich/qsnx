/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QStringList>
#include "simplecrypt.h"

class ConfReader;

namespace Ui {
class ProfileDialog;
}

class Profile {
public:
    Profile();
    Profile(const Profile & profile);
    Profile(const QString & name);
    Profile(const QString & site,int port,const QString & certificate,bool backward);
    Profile(const QString & site,int port,const QString & username,const QString & password,bool backward);
    Profile(const QString & site,int port,const QString & username,const QString & password,const QString & certificate,bool use_userpw,bool backward);
    static const QStringList profileNames();
    static const QMap<QString,Profile> profiles();
    static const Profile defaultInstance();
    static const QString defaultName();
    static void remove(const QString & name);
    static bool setDefault(const QString & name);
    static void clear();

    bool save(const QString & name);
    Profile &operator=(const Profile &profile);
    bool isValid() const;
    bool isCertificateValid() const;
    bool isUserPasswordValid() const;

    QString userName() const;
    QString password() const;
    QString certificate() const;
    QString site() const;
    int port() const;
    bool isUserPassword() const;
    bool isBackwardCompatabilityEnabled();

    void setUserName(const QString & username);
    void setPassword(const QString & password);
    void setCertificate(const QString & certificate);
    void setSite(const QString & site);
    void setPort(int port);
    void setUsingUserPassword(bool flag);
    void setBackwardCompatabilityMode(bool flag);

private:
    Profile & init(ConfReader * reader,const QString & name);
    static const QString user_dir();
    static const QString conf_file_name();

    static quint64 crypto_key;
    QString m_username;
    QString m_password;
    QString m_certificate;
    QString m_site;
    SimpleCrypt m_crypto;
    int m_port;
    bool m_use_userpw;
    bool m_backward;
};

class ProfileDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProfileDialog(QWidget *parent = nullptr);
    ~ProfileDialog();

private slots:
    void on_newProfileButton_clicked();
    void on_deleteProfileButton_clicked();
    void on_siteEdit_textChanged(const QString &arg1);
    void on_userEdit_textChanged(const QString &arg1);
    void on_passwordEdit_textChanged(const QString &arg1);
    void on_certEdit_textChanged(const QString &arg1);
    void on_certButton_clicked();
    void on_buttonBox_accepted();
    void on_certRadio_clicked(bool checked);
    void on_userPwRadio_clicked(bool checked);
    void on_editProfileNameButton_clicked();
    void on_portSpin_valueChanged(int arg1);
    void on_list_selectionChanged();

    void on_backCheck_clicked(bool checked);

private:
    void update_ok_state();
    bool contains_profile_name(const QString & name,int current_row) const;

    Ui::ProfileDialog *ui;
    QList<Profile> m_profiles;
};

#endif // PROFILEDIALOG_H
