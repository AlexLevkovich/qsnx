#ifndef QSNXWINDOW_H
#define QSNXWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "qsnxclient.h"

class QEvent;
class SNXSystemTrayIcon;
class QSNXWindow;
class QAction;
class QMenu;
class QCloseEvent;
class QWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class QSNXWindow; }
QT_END_NAMESPACE

class SNXSystemTrayIcon: public QSystemTrayIcon {
    Q_OBJECT
public:
    enum State {
        Connecting,
        Disconnecting,
        Connected,
        Disconnected
    };

    SNXSystemTrayIcon();
    ~SNXSystemTrayIcon();
    bool isConnecting() const;
    bool isDisconnecting() const;
    bool isConnected() const;
    bool isDisconnected() const;
signals:
    void connected();
    void disconnected();
    void connecting();
    void disconnecting();
private slots:
    void connect(const QString & name);
    void disconnect();
    void show_main_window();
    void recreate_connect_menu();
private:
    QWindow * findWindow(const QString & title);

    QAction * disconnect_action;
    QMenu * menu;
    QMenu * connect_menu;
    QAction * exit_action;
    QAction * about_action;
    QAction * show_action;
    QSNXWindow * m_window;
    State m_state;
    QSNXClient m_client;
};

class QSNXWindow : public QMainWindow {
    Q_OBJECT
public:
    QSNXWindow(QWidget *parent = nullptr);
    ~QSNXWindow();
    static void showTray();

private slots:
    void on_profilesButton_clicked();
    void connected();
    void disconnected();
    void connecting();
    void disconnecting();

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void connect_pressed(const QString & name);
    void disconnect_pressed();
    void about_pressed();
    void profiles_updated();

private:
    void fill_profiles();

    Ui::QSNXWindow *ui;
    static SNXSystemTrayIcon * m_tray_icon;
};
#endif // QSNXWINDOW_H
