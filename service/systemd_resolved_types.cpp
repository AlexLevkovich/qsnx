#include "systemd_resolved_types.h"
#include <arpa/inet.h>
#include <QtDBus>

static bool registerMetaType() {
    qRegisterMetaType<DNSAddress>("DNSAddress");
    qDBusRegisterMetaType<DNSAddress>();
    qRegisterMetaType<IndexedDNSAddress>("IndexedDNSAddress");
    qDBusRegisterMetaType<IndexedDNSAddress>();
    qRegisterMetaType<ResolvedName>("ResolvedName");
    qDBusRegisterMetaType<ResolvedName>();
    qRegisterMetaType<Domain>("Domain");
    qDBusRegisterMetaType<Domain>();
    qRegisterMetaType<IndexedDomain>("IndexedDomain");
    qDBusRegisterMetaType<IndexedDomain>();
    qRegisterMetaType<DNSArray>("DNSArray");
    qDBusRegisterMetaType<DNSArray>();
    qRegisterMetaType<IndexedDNSArray>("IndexedDNSArray");
    qDBusRegisterMetaType<IndexedDNSArray>();
    qRegisterMetaType<NameArray>("NameArray");
    qDBusRegisterMetaType<NameArray>();
    qRegisterMetaType<DomainArray>("DomainArray");
    qDBusRegisterMetaType<DomainArray>();
    qRegisterMetaType<IndexedDomainArray>("IndexedDomainArray");
    qDBusRegisterMetaType<IndexedDomainArray>();
    return true;
}

const bool registerMetaType_val = registerMetaType();

DNSAddress::DNSAddress() {
    sin_family = AF_INET;
    ip_addr.s_addr = 0;
}

DNSAddress::DNSAddress(const QHostAddress & addr) {
    sin_family = AF_INET;
    setAddress(addr);
}

DNSAddress::DNSAddress(const QString & addr) {
    sin_family = AF_INET;
    inet_aton(addr.toLocal8Bit().constData(),&ip_addr);
}

bool DNSAddress::isValid() const {
    return (ip_addr.s_addr != 0);
}

QHostAddress DNSAddress::address() const {
    return QHostAddress(QString((const char *)inet_ntoa(ip_addr)));
}

void DNSAddress::setAddress(const QHostAddress & addr) {
    inet_aton(addr.toString().toLocal8Bit().constData(),&ip_addr);
}

const QList<DNSAddress> dnsArrayFromStrings(const QStringList & dns) {
    QList<DNSAddress> ret;
    for (QString ip: dns) {
        ret.append(DNSAddress(ip));
    }
    return ret;
}

QDBusArgument & operator<<(QDBusArgument &argument, const DNSAddress & val) {
    argument.beginStructure();
    argument << val.sin_family << QByteArray::fromRawData((const char *)&val.ip_addr.s_addr,sizeof(uint32_t));
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument,DNSAddress & val) {
    argument.beginStructure();
    QByteArray arr;
    argument >> val.sin_family >> arr;
    argument.endStructure();
    val.ip_addr.s_addr = *((uint32_t *)arr.data());
    return argument;
}

IndexedDNSAddress::IndexedDNSAddress() {
    findex = -1;
    ((DNSAddress)*this) = DNSAddress();
}

IndexedDNSAddress::IndexedDNSAddress(int32_t findex,DNSAddress addr) {
    this->findex = findex;
    ((DNSAddress)*this) = addr;
}

int32_t IndexedDNSAddress::fIndex() const {
    return findex;
}

QDBusArgument & operator<<(QDBusArgument &argument, const IndexedDNSAddress & val) {
    argument.beginStructure();
    argument << val.findex << (DNSAddress)val;
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument,IndexedDNSAddress & val) {
    argument.beginStructure();
    argument >> val.findex >> (DNSAddress &)val;
    argument.endStructure();
    return argument;
}

QDBusArgument & operator<<(QDBusArgument &argument, const ResolvedName & val) {
    argument.beginStructure();
    argument << val.findex << val.name;
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument,ResolvedName & val) {
    argument.beginStructure();
    argument >> val.findex >> val.name;
    argument.endStructure();
    return argument;
}

Domain::Domain() {
    routing = false;
}

Domain::Domain(QString name) {
    this->name = name;
    routing = false;
}

Domain::Domain(bool routing,QString name) {
    this->name = name;
    this->routing = routing;
}

bool Domain::isRoutingOne() const {
    return routing;
}

QString Domain::nameOne() const {
    return name;
}

QList<Domain> domainArrayFromStrings(const QStringList & domains) {
    QList<Domain> ret;
    for (QString domain: domains) {
        ret.append(Domain(domain));
    }
    return ret;
}

QDBusArgument & operator<<(QDBusArgument &argument, const Domain & val) {
    argument.beginStructure();
    argument << val.name << val.routing;
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument,Domain & val) {
    argument.beginStructure();
    argument >> val.name >> val.routing;
    argument.endStructure();
    return argument;
}

IndexedDomain::IndexedDomain() {
    findex = -1;
    ((Domain)*this) = Domain();
}

IndexedDomain::IndexedDomain(int32_t findex,Domain domain) {
    this->findex = findex;
    ((Domain)*this) = domain;
}

int32_t IndexedDomain::fIndex() const {
    return findex;
}

QDBusArgument & operator<<(QDBusArgument &argument, const IndexedDomain & val) {
    argument.beginStructure();
    argument << val.findex << (Domain)val;
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument,IndexedDomain & val) {
    argument.beginStructure();
    argument >> val.findex >> (Domain &)val;
    argument.endStructure();
    return argument;
}
