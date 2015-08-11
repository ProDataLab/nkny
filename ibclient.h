#ifndef IBCLIENT_H
#define IBCLIENT_H

#include "ibticktype.h"
#include "ibfadatatype.h"
#include <QObject>

class QTcpSocket;
struct ContractDetails;
struct Contract;
struct CommissionReport;
struct UnderComp;
struct Order;
struct OrderState;
struct Execution;


#define TickerId long
#define OrderId  long

class IBClient : public QObject
{
    Q_OBJECT
public:
    explicit IBClient(QObject *parent = 0);
    ~IBClient();

    void connectToTWS(const QString & host, quint16 port, int clientId);
    void disconnectTWS();
    void send();

signals:
    void tickPrice(const TickerId & tickerId, const TickType & field, const double & price, const int & canAutoExecute);
    void tickSize(const TickerId & tickerId, const TickType & field, const int & size);
    void tickOptionComputation(const TickerId & tickerId, const TickType & tickType, const double & impliedVol, const double & delta,
        const double & optPrice, const double & pvDividend, const double & gamma, const double & vega, const double & theta, const double & undPrice);
    void tickGeneric(const TickerId & tickerId, const TickType & tickType, const double & value);
    void tickString(const TickerId & tickerId, const TickType & tickType, const QByteArray & value);
    void tickEFP(const TickerId & tickerId, const TickType & tickType, const double & basisPoints, const QByteArray & formattedBasisPoints,
        double totalDividends, int holdDays, const QByteArray& futureExpiry, double dividendImpact, double dividendsToExpiry);
    void orderStatus( OrderId orderId, const QByteArray &status, int filled,
        int remaining, double avgFillPrice, int permId, int parentId,
        double lastFillPrice, int clientId, const QByteArray& whyHeld);
    void openOrder( OrderId orderId, const Contract&, const Order&, const OrderState&);
    void openOrderEnd();
    void winError( const QByteArray &str, int lastError);
    void connectionClosed();
    void updateAccountValue(const QByteArray& key, const QByteArray& val,
    const QByteArray& currency, const QByteArray& accountName);
    void updatePortfolio( const Contract& contract, int position,
       double marketPrice, double marketValue, double averageCost,
       double unrealizedPNL, double realizedPNL, const QByteArray& accountName);
    void updateAccountTime(const QByteArray& timeStamp);
    void accountDownloadEnd(const QByteArray& accountName);
    void nextValidId( OrderId orderId);
    void contractDetails( int reqId, const ContractDetails& contractDetails);
    void bondContractDetails( int reqId, const ContractDetails& contractDetails);
    void contractDetailsEnd( int reqId);
    void execDetails( int reqId, const Contract& contract, const Execution& execution);
    void execDetailsEnd( int reqId);
    void error(const int id, const int errorCode, const QByteArray errorString);
    void updateMktDepth(TickerId id, int position, int operation, int side,
       double price, int size);
    void updateMktDepthL2(TickerId id, int position, QByteArray marketMaker, int operation,
       int side, double price, int size);
    void updateNewsBulletin(int msgId, int msgType, const QByteArray& newsMessage, const QByteArray& originExch);
    void managedAccounts( const QByteArray& accountsList);
    void receiveFA(FaDataType pFaDataType, const QByteArray& cxml);
    void historicalData(TickerId reqId, const QByteArray& date, double open, double high,
        double low, double close, int volume, int barCount, double WAP, int hasGaps);
    void scannerParameters(const QByteArray &xml);
    void scannerData(int reqId, int rank, const ContractDetails &contractDetails,
        const QByteArray &distance, const QByteArray &benchmark, const QByteArray &projection,
        const QByteArray &legsStr);
    void scannerDataEnd(int reqId);
    void realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
        long volume, double wap, int count);
    void currentTime(long time);
    void fundamentalData(TickerId reqId, const QByteArray& data);
    void deltaNeutralValidation(int reqId, const UnderComp& underComp);
    void tickSnapshotEnd( int reqId);
    void marketDataType( TickerId reqId, int marketDataType);
    void commissionReport( const CommissionReport &commissionReport);
    void position( const QByteArray& account, const Contract& contract, int position, double avgCost);
    void positionEnd();
    void accountSummary( int reqId, const QByteArray& account, const QByteArray& tag, const QByteArray& value, const QByteArray& curency);
    void accountSummaryEnd( int reqId);
    void verifyMessageAPI( const QByteArray& apiData);
    void verifyCompleted( bool isSuccessful, const QByteArray& errorText);
    void displayGroupList( int reqId, const QByteArray& groups);
    void displayGroupUpdated( int reqId, const QByteArray& contractInfo);
public slots:

private slots:
    void onConnected();
    void onReadyRead();

private:
    QTcpSocket* m_socket;
    int         m_clientId;
    QByteArray  m_outBuffer;
    QByteArray  m_inBuffer;
    bool        m_connected;
    int         m_serverVersion;
    QByteArray  m_twsTime;
    int         m_begIdx;
    int         m_endIdx;
    bool        m_extraAuth;

    void        decodeField(int & value);
    void        decodeField(bool & value);
    void        decodeField(long & value);
    void        decodeField(double & value);
    void        decodeField(QByteArray & value);
    QByteArray  decodeField();

    void        decodeFieldMax(int & value);
    void        decodeFieldMax(long & value);
    void        decodeFieldMax(double & value);

    void        encodeField(const int & value);
    void        encodeField(const bool & value);
    void        encodeField(const long & value);
    void        encodeField(const double & value);
    void        encodeField(const QByteArray & buf);

    void        cleanInBuffer();

};

#endif // IBCLIENT_H
