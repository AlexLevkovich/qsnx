#ifndef SYSTEMDRESOLVED_H
#define SYSTEMDRESOLVED_H

#include <QDBusPendingReply>
#include "logger.h"
#include "systemd_resolved_types.h"

class OrgFreedesktopResolve1ManagerInterface;

class SystemdResolved : public QObject {
    Q_OBJECT
public:
    explicit SystemdResolved(const QString & ifname,QObject *parent = nullptr);

    bool isValid() const;
    QString lastError() const;
    IndexedDNSAddress currentDNSServer() const;
    IndexedDNSArray dNS() const;
    QString dNSOverTLS() const;
    QString dNSSEC() const;
    QStringList dNSSECNegativeTrustAnchors() const;
    bool dNSSECSupported() const;
    QString dNSStubListener() const;
    IndexedDomainArray domains() const;
    IndexedDNSArray fallbackDNS() const;
    QString lLMNR() const;
    QString lLMNRHostname() const;
    QString multicastDNS() const;
    QString resolvConfMode() const;

    bool ResolveAddress(int family, const QByteArray &address, qulonglong flags,NameArray & addrs, qulonglong & out_flags);
    bool ResolveHostname(const QString &name, int family, qulonglong flags,IndexedDNSArray & hostnames, QString & canonical, qulonglong & out_flags);
    bool RevertLink();
    bool SetLinkDNS(DNSArray addresses);
    bool SetLinkDNSOverTLS(const QString &mode);
    bool SetLinkDNSSEC(const QString &mode);
    bool SetLinkDNSSECNegativeTrustAnchors(const QStringList &names);
    bool SetLinkDefaultRoute(bool enable);
    bool SetLinkDomains(DomainArray domains);
    bool SetLinkLLMNR(const QString &mode);
    bool SetLinkMulticastDNS(const QString &mode);

private:
    template<typename T,typename V> bool replyToValue(const QDBusPendingReply<T,V> & in_reply,T & ret1,V & ret2) const {
        QDBusPendingReply<T,V> & reply = (QDBusPendingReply<T,V> &)in_reply;
        reply.waitForFinished();
        if (reply.isError()) {
            Logger() << reply.error().message();
            return false;
        }
        ret1 = ((QVariant)reply.argumentAt(0)).value<T>();
        ret2 = ((QVariant)reply.argumentAt(1)).value<V>();
        return true;
    }

    template<typename T,typename V,typename K> bool replyToValue(const QDBusPendingReply<T,V,K> & in_reply,T & ret1,V & ret2,K & ret3) const {
        QDBusPendingReply<T,V,K> & reply = (QDBusPendingReply<T,V,K> &)in_reply;
        reply.waitForFinished();
        if (reply.isError()) {
            Logger() << reply.error().message();
            return false;
        }
        ret1 = ((QVariant)reply.argumentAt(0)).value<T>();
        ret2 = ((QVariant)reply.argumentAt(1)).value<V>();
        ret3 = ((QVariant)reply.argumentAt(2)).value<K>();
        return true;
    }

    bool replyToVoid(const QDBusPendingReply<> & in_reply) const {
        QDBusPendingReply<> & reply = (QDBusPendingReply<> &)in_reply;
        reply.waitForFinished();
        if (reply.isError()) {
            Logger() << reply.error().message();
            return false;
        }
        return true;
    }

    OrgFreedesktopResolve1ManagerInterface * m_interface;
    int ifindex;
};

#endif // SYSTEMDRESOLVED_H
