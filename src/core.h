// Copyright (c) 2009-2015 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin developers
// Copyright (c) 2015 The DarkSilk developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DARKSILK_CORE_H
#define DARKSILK_CORE_H

#include "uint256.h"
#include "serialize.h"
#include "util.h"
#include "script.h"

#include <stdio.h>

class CScript;
class CTransaction;

/** An outpoint - a combination of a transaction hash and an index n into its vout */
class COutPoint
{
public:
    uint256 hash;
    unsigned int n;

    COutPoint() { SetNull(); }
    COutPoint(uint256 hashIn, unsigned int nIn) { hash = hashIn; n = nIn; }
    
    IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
    void SetNull() { hash = 0; n = (unsigned int) -1; }
    bool IsNull() const { return (hash == 0 && n == (unsigned int) -1); }

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
    }

    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return !(a == b);
    }

    std::string ToString() const
    {
        return strprintf("COutPoint(%s, %u)", hash.ToString().substr(0,10), n);
    }

    std::string ToStringShort() const
    {
        return strprintf("%s-%u", hash.ToString().substr(0,64), n);
    }

};

/** An inpoint - a combination of a transaction and an index n into its vin */
class CInPoint
{
public:
    CTransaction* ptx;
    unsigned int n;

    CInPoint() { SetNull(); }
    CInPoint(CTransaction* ptxIn, unsigned int nIn) { ptx = ptxIn; n = nIn; }
    void SetNull() { ptx = NULL; n = (unsigned int) -1; }
    bool IsNull() const { return (ptx == NULL && n == (unsigned int) -1); }
};

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class CTxIn
{
public:
    COutPoint prevout;
    CScript scriptSig;
    CScript prevPubKey;
    unsigned int nSequence;

    CTxIn()
    {
        nSequence = std::numeric_limits<unsigned int>::max();
    }

    explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
    {
        prevout = prevoutIn;
        scriptSig = scriptSigIn;
        nSequence = nSequenceIn;
    }

    explicit CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
    {
        prevout = COutPoint(hashPrevTx, nOut);
        scriptSig = scriptSigIn;
        nSequence = nSequenceIn;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(prevout);
        READWRITE(scriptSig);
        READWRITE(nSequence);
    )

    bool IsFinal() const
    {
        return (nSequence == std::numeric_limits<unsigned int>::max());
    }

    friend bool operator==(const CTxIn& a, const CTxIn& b)
    {
        return (a.prevout   == b.prevout &&
                a.scriptSig == b.scriptSig &&
                a.nSequence == b.nSequence);
    }

    friend bool operator!=(const CTxIn& a, const CTxIn& b)
    {
        return !(a == b);
    }

    std::string ToString() const
    {
        std::string str;
        str += "CTxIn(";
        str += prevout.ToString();
        if (prevout.IsNull())
            str += strprintf(", coinbase %s", HexStr(scriptSig));
        else
            str += strprintf(", scriptSig=%s", scriptSig.ToString().substr(0,24));
        if (nSequence != std::numeric_limits<unsigned int>::max())
            str += strprintf(", nSequence=%u", nSequence);
        str += ")";
        return str;
    }
};




/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class CTxOut
{
public:
    int64_t nValue;
    int nRounds;
    CScript scriptPubKey;

    CTxOut()
    {
        SetNull();
    }

    CTxOut(int64_t nValueIn, CScript scriptPubKeyIn)
    {
        nValue = nValueIn;
        nRounds = -10; // an initial value, should be no way to get this by calculations
        scriptPubKey = scriptPubKeyIn;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nValue);
        READWRITE(scriptPubKey);
    )

    void SetNull()
    {
        nValue = -1;
        nRounds = -10; // an initial value, should be no way to get this by calculations
        scriptPubKey.clear();
    }

    bool IsNull() const
    {
        return (nValue == -1);
    }

    void SetEmpty()
    {
        nValue = 0;
        scriptPubKey.clear();
    }

    bool IsEmpty() const
    {
        return (nValue == 0 && scriptPubKey.empty());
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    bool IsDust(int64_t MIN_RELAY_TX_FEE) const
    {
        // "Dust" is defined in terms of MIN_RELAY_TX_FEE,
        // which has units satoshis-per-kilobyte.
        // If you'd pay more than 1/3 in fees
        // to spend something, then we consider it dust.
        // A typical txout is 34 bytes big, and will
        // need a CTxIn of at least 148 bytes to spend,
        // so dust is a txout less than 546 satoshis
        // with default nMinRelayTxFee.
        return ((nValue*1000)/(3*((int)GetSerializeSize(SER_DISK,0)+148)) < MIN_RELAY_TX_FEE);
    }

    friend bool operator==(const CTxOut& a, const CTxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.nRounds      == b.nRounds &&
                a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const CTxOut& a, const CTxOut& b)
    {
        return !(a == b);
    }

    std::string ToString() const
    {
        if (IsEmpty()) return "CTxOut(empty)";
        return strprintf("CTxOut(nValue=%s, scriptPubKey=%s)", FormatMoney(nValue), scriptPubKey.ToString());
    }
};

#endif
