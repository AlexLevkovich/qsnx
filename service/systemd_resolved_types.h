#ifndef SYSTEMD_RESOLVED_TYPES_H
#define SYSTEMD_RESOLVED_TYPES_H
#include <stdint.h>
#include <netinet/in.h>
#include <QList>
#include <QtDBus/QDBusArgument>
#include <QHostAddress>

class DNSAddress {
private:
    int32_t sin_family;
    struct in_addr ip_addr;
public:
    DNSAddress();
    DNSAddress(const QHostAddress & addr);
    DNSAddress(const QString & addr);
    bool isValid() const;
    QHostAddress address() const;
    void setAddress(const QHostAddress & addr);
    friend QDBusArgument & operator<<(QDBusArgument &argument, const DNSAddress & val);
    friend const QDBusArgument & operator>>(const QDBusArgument &argument,DNSAddress & val);
};
Q_DECLARE_METATYPE(DNSAddress)

const QList<DNSAddress> dnsArrayFromStrings(const QStringList & dns);

class IndexedDNSAddress : public DNSAddress {
private:
    int32_t findex;
public:
    IndexedDNSAddress();
    IndexedDNSAddress(int32_t findex,DNSAddress addr);
    int32_t fIndex() const;
    friend QDBusArgument & operator<<(QDBusArgument &argument, const IndexedDNSAddress & val);
    friend const QDBusArgument & operator>>(const QDBusArgument &argument,IndexedDNSAddress & val);
};
Q_DECLARE_METATYPE(IndexedDNSAddress)

struct ResolvedName {
    int32_t findex;
    QString name;
};
Q_DECLARE_METATYPE(ResolvedName)
QDBusArgument & operator<<(QDBusArgument &argument, const ResolvedName & val);
const QDBusArgument & operator>>(const QDBusArgument &argument,ResolvedName & val);

class Domain {
private:
    bool routing;
    QString name;
public:
    Domain();
    Domain(QString name);
    Domain(bool routing,QString name);
    bool isRoutingOne() const;
    QString nameOne() const;
    friend QDBusArgument & operator<<(QDBusArgument &argument, const Domain & val);
    friend const QDBusArgument & operator>>(const QDBusArgument &argument,Domain & val);
};
Q_DECLARE_METATYPE(Domain)

QList<Domain> domainArrayFromStrings(const QStringList & domains);

class IndexedDomain : public Domain {
private:
    int32_t findex;
public:
    IndexedDomain();
    IndexedDomain(int32_t findex,Domain domain);
    int32_t fIndex() const;
    friend QDBusArgument & operator<<(QDBusArgument &argument, const IndexedDomain & val);
    friend const QDBusArgument & operator>>(const QDBusArgument &argument,IndexedDomain & val);
};
Q_DECLARE_METATYPE(IndexedDomain)

typedef QList<DNSAddress> DNSArray;
typedef QList<IndexedDNSAddress> IndexedDNSArray;
typedef QList<ResolvedName> NameArray;
typedef QList<Domain> DomainArray;
typedef QList<IndexedDomain> IndexedDomainArray;

Q_DECLARE_METATYPE(DNSArray)
Q_DECLARE_METATYPE(IndexedDNSArray)
Q_DECLARE_METATYPE(NameArray)
Q_DECLARE_METATYPE(DomainArray)
Q_DECLARE_METATYPE(IndexedDomainArray)


#endif // SYSTEMD_RESOLVED_TYPES_H
