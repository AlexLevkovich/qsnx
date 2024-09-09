/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "qsnxwindow.h"
#include "ui_qsnxwindow.h"
#include "profiledialog.h"
#include "windowcenterer.h"
#include <QGuiApplication>
#include <QMenu>
#include <QIcon>
#include <QMessageBox>
#include <QEvent>
#include <QInputDialog>
#include <QDebug>
#include <QWindow>

SNXSystemTrayIcon * QSNXWindow::m_tray_icon = NULL;

SNXSystemTrayIcon::SNXSystemTrayIcon() : QSystemTrayIcon(QCoreApplication::instance()) {
    m_window = NULL;
    m_state = Disconnected;
    menu = new QMenu();
    connect_menu = NULL;
    show_action = menu->addAction(QIcon(QIcon::fromTheme("window").pixmap(128)),tr("Show window"));
    disconnect_action = menu->addAction(QIcon(QIcon::fromTheme("network-disconnect").pixmap(128)),tr("Disconnect"));
    recreate_connect_menu();
    about_action = menu->addAction(QIcon(QIcon::fromTheme("help-about").pixmap(128)),tr("About..."));
    exit_action = menu->addAction(QIcon(QIcon::fromTheme("application-exit").pixmap(128)),tr("Exit"));
    setContextMenu(menu);
    setIcon(QIcon(QIcon("://pics/key_grey.svg").pixmap(128)));
    setToolTip("QSNX Client : "+tr("disconnected"));
    disconnect_action->setEnabled(isConnected());
    connect_menu->setEnabled(isDisconnected());

    QObject::connect(disconnect_action,&QAction::triggered,this,&SNXSystemTrayIcon::disconnect);
    QObject::connect(this,&SNXSystemTrayIcon::activated,this,[=](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) show_main_window();
    });
    QObject::connect(QCoreApplication::instance(),&QCoreApplication::aboutToQuit,this,[=]() {
        if (isConnected()) disconnect();
    });

    QObject::connect(exit_action,&QAction::triggered,this,[=](){
        if (m_window != NULL) m_window->close();
        QCoreApplication::instance()->quit();
    });
    QObject::connect(show_action,&QAction::triggered,this,&SNXSystemTrayIcon::show_main_window);
    QObject::connect(about_action,&QAction::triggered,this,[=]() {
        QString title = tr("About QSNX Client...");
        if (findWindow(title) == NULL) {
            QMessageBox::about(NULL,title,tr("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">QSNX Client</span> is GUI to proprietary SNX Vpn Application.</p>"
                                             "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">QSNX Client is %1.</p>"
                                             "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Developer: Alex Levkovich (<a href=\"mailto:alevkovich@tut.by\"><span style=\" text-decoration: underline; color:#0057ae;\">alevkovich@tut.by</span></a>)</p>"
                                             "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">License: GPL</p>").arg(VERSION));
        }
    });
    QObject::connect(&m_client,&QSNXClient::passwordRequested,this,[=]() {
        bool ok;
        QString pw = QInputDialog::getText(NULL,tr("Password is needed"),tr("Password:"),QLineEdit::Password,"", &ok);
        if (!ok) m_client.terminate();
        else m_client.sendPassword(pw);
    });
    QObject::connect(&m_client,&QSNXClient::error,this,[=](const QString & str) {
        QMessageBox::critical(NULL,tr("snx returned the error"),str);
    });
    QObject::connect(&m_client,&QSNXClient::connected,this,[=](const QString &ip, const QStringList &dns_ips, const QStringList &dns_suffixes) {
        m_state = Connected;
        setIcon(QIcon(QIcon("://pics/key.svg").pixmap(128)));
        setToolTip("QSNX Client : "+tr("connected to ")+m_profile_name);
        connect_menu->setEnabled(false);
        disconnect_action->setEnabled(true);
        m_ip = ip;
        m_dns_ips = dns_ips;
        m_dns_suffixes = dns_suffixes;
        emit connected();
    });
    QObject::connect(&m_client,&QSNXClient::disconnected,this,[=]() {
        m_state = Disconnected;
        setIcon(QIcon(QIcon("://pics/key_grey.svg").pixmap(128)));
        setToolTip("QSNX Client : "+tr("disconnected"));
        connect_menu->setEnabled(true);
        disconnect_action->setEnabled(false);
        emit disconnected();
    });
}

SNXSystemTrayIcon::~SNXSystemTrayIcon() {
    delete menu;
    delete connect_menu;
}

void SNXSystemTrayIcon::recreate_connect_menu() {
    if (connect_menu != NULL) connect_menu->deleteLater();
    connect_menu = new QMenu();
    connect_menu->setTitle(tr("Connect to"));
    for (const QString & profile: Profile::profileNames()) {
        connect_menu->addAction(QIcon(QIcon::fromTheme("network-vpn").pixmap(128)),profile);
    }
    QObject::connect(connect_menu,&QMenu::triggered,this,[=](QAction * action) {
        connect(action->text());
    });
    menu->insertMenu(disconnect_action,connect_menu);
}

void SNXSystemTrayIcon::show_main_window() {
    if (m_window != NULL) return;
    m_window = new QSNXWindow(&m_client);
    m_window->show();
    QObject::connect(m_window,&QSNXWindow::connect_pressed,this,&SNXSystemTrayIcon::connect);
    QObject::connect(m_window,&QSNXWindow::disconnect_pressed,this,&SNXSystemTrayIcon::disconnect);
    QObject::connect(m_window,&QSNXWindow::about_pressed,about_action,&QAction::trigger);
    QObject::connect(m_window,&QObject::destroyed,this,[=]() {
        m_window = NULL;
        show_action->setEnabled(true);
    });
    QObject::connect(m_window,&QSNXWindow::profiles_updated,this,&SNXSystemTrayIcon::recreate_connect_menu);
    show_action->setEnabled(false);
}

QWindow * SNXSystemTrayIcon::findWindow(const QString & title) {
    for (QWindow * & wnd: QGuiApplication::topLevelWindows()) {
        if (wnd->title() == title) return wnd;
    }
    return NULL;
}

void SNXSystemTrayIcon::connect(const QString & name) {
    m_profile_name = name;
    m_state = Connecting;
    setToolTip("QSNX Client : "+tr("connecting to ")+m_profile_name);
    connect_menu->setEnabled(false);
    disconnect_action->setEnabled(false);
    emit connecting();
    Profile profile(name);
    if (profile.isUserPassword()) m_client.connect(profile.site(),profile.userName(),profile.password(),profile.isBackwardCompatabilityEnabled(),profile.port());
    else m_client.connect(profile.site(),profile.certificate(),profile.isBackwardCompatabilityEnabled(),profile.port());
}

void SNXSystemTrayIcon::disconnect() {
    m_state = Disconnecting;
    connect_menu->setEnabled(false);
    setToolTip("QSNX Client : "+tr("disconnecting from ")+m_profile_name);
    disconnect_action->setEnabled(false);
    emit disconnecting();
    m_client.disconnect();
}

bool SNXSystemTrayIcon::isConnecting() const {
    return (m_state == Connecting);
}

bool SNXSystemTrayIcon::isDisconnecting() const {
    return (m_state == Disconnecting);
}

bool SNXSystemTrayIcon::isConnected(QString * ip,QStringList * dns_ips,QStringList * dns_suffixes) const {
    if (m_state == Connected) {
        if (ip != NULL) *ip = m_ip;
        if (dns_ips != NULL) *dns_ips = m_dns_ips;
        if (dns_suffixes != NULL) *dns_suffixes = m_dns_suffixes;
    }
    return (m_state == Connected);
}

bool SNXSystemTrayIcon::isDisconnected() const {
    return (m_state == Disconnected);
}

void QSNXWindow::showTray() {
    (m_tray_icon = new SNXSystemTrayIcon())->show();
}

QSNXWindow::QSNXWindow(QSNXClient * client,QWidget *parent) : QMainWindow(parent) , ui(new Ui::QSNXWindow) {
    m_client = client;
    ui->setupUi(this);

    ui->connectButton->setIcon(QIcon(QIcon::fromTheme("network-connect").pixmap(128)));
    ui->profilesButton->setIcon(QIcon(QIcon::fromTheme("configure").pixmap(128)));
    ui->aboutButton->setIcon(QIcon(QIcon::fromTheme("help-about").pixmap(128)));
    setAttribute(Qt::WA_DeleteOnClose,true);
    new WindowCenterer(this);
    QCoreApplication::instance()->installEventFilter(this);
    connect(ui->connectButton,&QPushButton::clicked,this,[=]() {
        if (m_tray_icon->isDisconnected()) emit connect_pressed(ui->profileCombo->currentText());
        else emit disconnect_pressed();
    });
    connect(ui->aboutButton,&QPushButton::clicked,this,&QSNXWindow::about_pressed);
    connect(m_tray_icon,&SNXSystemTrayIcon::connected,this,&QSNXWindow::connected);
    connect(m_tray_icon,&SNXSystemTrayIcon::disconnected,this,&QSNXWindow::disconnected);
    connect(m_tray_icon,&SNXSystemTrayIcon::connecting,this,&QSNXWindow::connecting);
    connect(m_tray_icon,&SNXSystemTrayIcon::disconnecting,this,&QSNXWindow::disconnecting);
    fill_profiles();

    if (m_tray_icon->isConnecting()) connecting();
    else if (m_tray_icon->isDisconnecting()) disconnecting();
    else if (m_tray_icon->isConnected()) connected();
    else if (m_tray_icon->isDisconnected()) disconnected();
}

QSNXWindow::~QSNXWindow() {
    delete ui;
}

void QSNXWindow::closeEvent(QCloseEvent *event) {
    if (ui->profileCombo->count() > 0) Profile::setDefault(ui->profileCombo->currentText());
    QMainWindow::closeEvent(event);
}

void QSNXWindow::on_profilesButton_clicked() {
    QString ip;
    QStringList dns_ips;
    QStringList dns_suffixes;
    if (m_tray_icon->isConnected(&ip,&dns_ips,&dns_suffixes)) {
        QMessageBox::information(this,tr("Info about the current connection"),(QString::fromLocal8Bit("<html><head/><body><table border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" cellspacing=\"2\" cellpadding=\"0\"><tr><td><p><span style=\" font-weight:600;\">")+tr("Office Mode IP")+QString::fromLocal8Bit("</span></p></td><td><p>%1</p></td></tr><tr><td><p><span style=\" font-weight:600;\">")+tr("DNS Addresses")+QString::fromLocal8Bit("</span></p></td><td><p>%2</p></td></tr><tr><td><p><span style=\" font-weight:600;\">")+tr("DNS Suffix")+QString::fromLocal8Bit("</span></p></td><td><p>%3</p></td></tr></table></body></html>")).arg(ip).arg(dns_ips.join("<br/>")).arg(dns_suffixes.join("<br/>")));
    }
    else {
        if (ProfileDialog(m_client->hasBackwardCompabilityOption(),this).exec() == QDialog::Rejected) return;
        fill_profiles();
    }
}

void QSNXWindow::fill_profiles() {
    ui->profileCombo->clear();
    for (const QString & profile: Profile::profileNames()) {
        ui->profileCombo->addItem(QIcon(QIcon::fromTheme("network-vpn").pixmap(128)),profile);
    }
    QString name = Profile::defaultName();
    if (!name.isEmpty()) ui->profileCombo->setCurrentText(name);
    ui->connectButton->setEnabled(ui->profileCombo->count() > 0);
    emit profiles_updated();
}

void QSNXWindow::connected() {
    ui->connectButton->setEnabled(ui->profileCombo->count() > 0);
    ui->connectButton->setText(tr("Disconnect"));
    ui->connectButton->setIcon(QIcon(QIcon::fromTheme("network-disconnect").pixmap(128)));
    ui->profilesButton->setEnabled(true);
    ui->profilesButton->setToolTip(tr("Shows the connection info"));
}

void QSNXWindow::disconnected() {
    ui->connectButton->setEnabled(ui->profileCombo->count() > 0);
    ui->connectButton->setText(tr("Connect"));
    ui->connectButton->setIcon(QIcon(QIcon::fromTheme("network-connect").pixmap(128)));
    ui->profilesButton->setEnabled(true);
    ui->profilesButton->setToolTip(tr("The profile configurator"));
}

void QSNXWindow::connecting() {
    ui->connectButton->setEnabled(false);
    ui->connectButton->setText(tr("Connect"));
    ui->connectButton->setIcon(QIcon(QIcon::fromTheme("network-connect").pixmap(128)));
    ui->profilesButton->setEnabled(false);
    ui->profilesButton->setToolTip("");
}

void QSNXWindow::disconnecting() {
    ui->connectButton->setEnabled(false);
    ui->connectButton->setText(tr("Disconnect"));
    ui->connectButton->setIcon(QIcon(QIcon::fromTheme("network-disconnect").pixmap(128)));
    ui->profilesButton->setEnabled(false);
    ui->profilesButton->setToolTip("");
}
