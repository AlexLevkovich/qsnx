#include "systemdresolved.h"
#include "manager_interface.h"
#include <net/if.h>

SystemdResolved::SystemdResolved(const QString & ifname,QObject *parent) : QObject{parent} {
    m_interface = new OrgFreedesktopResolve1ManagerInterface("org.freedesktop.resolve1","/org/freedesktop/resolve1",QDBusConnection::systemBus(),this);
    if (!m_interface->isValid()) Logger() << "invalid connection to" << OrgFreedesktopResolve1ManagerInterface::staticInterfaceName() << lastError();
    ifindex = if_nametoindex(ifname.toLocal8Bit().constData());
    if (ifindex == 0) Logger() << "Cannot get if index from if name:" << QString::fromLocal8Bit((const char *)strerror(errno));
}

QString SystemdResolved::lastError() const {
    return m_interface->lastError().message();
}

bool SystemdResolved::isValid() const {
    return ifindex != 0 && m_interface->isValid();
}

IndexedDNSAddress SystemdResolved::currentDNSServer() const {
    if (!isValid()) return IndexedDNSAddress();
    return m_interface->currentDNSServer();
}

IndexedDNSArray SystemdResolved::dNS() const {
    if (!isValid()) return IndexedDNSArray();
    return m_interface->dNS();
}

QString SystemdResolved::dNSOverTLS() const {
    if (!isValid()) return QString();
    return m_interface->dNSOverTLS();
}

QString SystemdResolved::dNSSEC() const {
    if (!isValid()) return QString();
    return m_interface->dNSSEC();
}

QStringList SystemdResolved::dNSSECNegativeTrustAnchors() const {
    if (!isValid()) return QStringList();
    return m_interface->dNSSECNegativeTrustAnchors();
}

bool SystemdResolved::dNSSECSupported() const {
    if (!isValid()) return false;
    return m_interface->dNSSECSupported();
}

QString SystemdResolved::dNSStubListener() const {
    if (!isValid()) return QString();
    return m_interface->dNSStubListener();
}

IndexedDomainArray SystemdResolved::domains() const {
    if (!isValid()) return IndexedDomainArray();
    return m_interface->domains();
}

IndexedDNSArray SystemdResolved::fallbackDNS() const {
    if (!isValid()) return IndexedDNSArray();
    return m_interface->fallbackDNS();
}

QString SystemdResolved::lLMNR() const {
    if (!isValid()) return QString();
    return m_interface->lLMNR();
}

QString SystemdResolved::lLMNRHostname() const {
    if (!isValid()) return QString();
    return m_interface->lLMNRHostname();
}

QString SystemdResolved::multicastDNS() const {
    if (!isValid()) return QString();
    return m_interface->multicastDNS();
}

QString SystemdResolved::resolvConfMode() const {
    if (!isValid()) return QString();
    return m_interface->resolvConfMode();
}

bool SystemdResolved::ResolveAddress(int family, const QByteArray &address, qulonglong flags,NameArray & addrs, qulonglong & out_flags) {
    if (!isValid()) return false;

    return replyToValue<NameArray,qulonglong>(m_interface->ResolveAddress(ifindex,family,address,flags),addrs,out_flags);
}

bool SystemdResolved::ResolveHostname(const QString &name, int family, qulonglong flags,IndexedDNSArray & hostnames, QString & canonical, qulonglong & out_flags) {
    if (!isValid()) return false;

    return replyToValue<IndexedDNSArray,QString,qulonglong>(m_interface->ResolveHostname(ifindex,name,family,flags),hostnames,canonical,out_flags);
}

bool SystemdResolved::RevertLink() {
    if (!isValid()) return false;

    return replyToVoid(m_interface->RevertLink(ifindex));
}

bool SystemdResolved::SetLinkDNS(DNSArray addresses) {
    if (!isValid()) return false;

    return replyToVoid(m_interface->SetLinkDNS(ifindex,addresses));
}

bool SystemdResolved::SetLinkDNSOverTLS(const QString &mode) {
    if (!isValid()) return false;

    return replyToVoid(m_interface->SetLinkDNSOverTLS(ifindex,mode));
}

bool SystemdResolved::SetLinkDNSSEC(const QString &mode) {
    if (!isValid()) return false;

    return replyToVoid(m_interface->SetLinkDNSSEC(ifindex,mode));
}

bool SystemdResolved::SetLinkDNSSECNegativeTrustAnchors(const QStringList &names) {
    if (!isValid()) return false;

    return replyToVoid(m_interface->SetLinkDNSSECNegativeTrustAnchors(ifindex,names));
}

bool SystemdResolved::SetLinkDefaultRoute(bool enable) {
    if (!isValid()) return false;

    return replyToVoid(m_interface->SetLinkDefaultRoute(ifindex,enable));
}

bool SystemdResolved::SetLinkDomains(DomainArray domains) {
    if (!isValid()) return false;

    return replyToVoid(m_interface->SetLinkDomains(ifindex,domains));
}

bool SystemdResolved::SetLinkLLMNR(const QString &mode) {
    if (!isValid()) return false;

    return replyToVoid(m_interface->SetLinkLLMNR(ifindex,mode));
}

bool SystemdResolved::SetLinkMulticastDNS(const QString &mode) {
    if (!isValid()) return false;

    return replyToVoid(m_interface->SetLinkMulticastDNS(ifindex,mode));
}
