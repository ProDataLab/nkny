#include "security.h"


Security::Security(const long &tickerId, Contract *contract, QObject *parent)
    : m_tickerId(tickerId)
    , m_contract(contract)
    , QObject(parent)
    , m_histDataRequested(false)
{

}

Security::~Security()
{
//    foreach(TimeFrame tf, m_dataMap.keys) {
//        qDelete(m_dataMap[tf]);
//    }
}

