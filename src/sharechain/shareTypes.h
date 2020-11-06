#ifndef SHARE_TYPES_H
#define SHARE_TYPES_H

#include <string>
#include <vector>
#include <uint256.h>
#include <memory>
#include "univalue.h"

//TODO: MerkleLink.index -> IntType(0)?????
//TODO: HashLinkType.extra_data ->  FixedStrType(0) ?????

//VarIntType = unsigned long long
//IntType(256) = uint256
//IntType(160) = uint160
//IntType(64) = unsigned long long
//IntType(32) = unsigned int
//IntType(16) = unsigned short
//PossiblyNoneType(0, IntType(256)) — Переменная, у которой 0 и None/nullptr — одно и то же.

namespace c2pool::shares
{
    bool is_segwit_activated(int version, int segwit_activation_version);

    enum ShareVersion
    {
        NoneVersion = 0,
        Share = 17,
        PreSegwitShare = 32,
        NewShare = 33
    };

    enum StaleInfo
    {
        None = 0,
        orphan = 253,
        doa = 254
    };

    class HashLinkType
    {
    public:
        std::string state;         //TODO: pack.FixedStrType(32)
        std::string extra_data;    //TODO: pack.FixedStrType(0) # bit of a hack, but since the donation script is at the end, const_ending is long enough to always make this empty
        unsigned long long length; //pack.VarIntType()

        HashLinkType() {}
        HashLinkType(std::string state, std::string extra_data, unsigned long long length);

        friend std::istream &operator>>(std::istream &is, HashLinkType &value);
        friend std::ostream &operator<<(std::ostream &os, const HashLinkType &value);
        friend bool operator==(const HashLinkType &first, const HashLinkType &second);

        friend bool operator!=(const HashLinkType &first, const HashLinkType &second);

        /*
        operator UniValue(){
            UniValue result(UniValue::VOBJ);
        

            return result;
        }
        */

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("state", state);
            result.pushKV("extra_data", extra_data);
            result.pushKV("lenght", length);

            return result;
        }
    };

    class MerkleLink
    {
    public:
        std::vector<uint256> branch; //pack.ListType(pack.IntType(256))
        int index;                   //TODO: pack.IntType(0) # it will always be 0

        MerkleLink()
        {
            index = 0;
        };
        MerkleLink(std::vector<uint256> branch, int index);

        friend std::istream &operator>>(std::istream &is, MerkleLink &value);
        friend std::ostream &operator<<(std::ostream &os, const MerkleLink &value);

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            UniValue branch_list(UniValue::VARR);
            for (auto num : branch)
            {
                branch_list.push_back(num.GetHex());
            }

            result.pushKV("branch", branch_list);
            result.pushKV("index", index);

            return result;
        }
    };

    class SmallBlockHeaderType
    {
    public:
        unsigned long long version; // + ('version', pack.VarIntType()),
        uint256 previous_block;     // none — ('previous_block', pack.PossiblyNoneType(0, pack.IntType(256))),
        unsigned int timestamp;     // ('timestamp', pack.IntType(32)),
        unsigned int bits;          // ('bits', bitcoin_data.FloatingIntegerType()),
        unsigned int nonce;         // ('nonce', pack.IntType(32)),

        SmallBlockHeaderType(){};
        SmallBlockHeaderType(unsigned long long version, uint256 previous_block, unsigned int timeStamp, unsigned int bits, unsigned int nonce);

        friend std::istream &operator>>(std::istream &is, SmallBlockHeaderType &value);
        friend std::ostream &operator<<(std::ostream &os, const SmallBlockHeaderType &value);

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("version", version);
            result.pushKV("previous_block", previous_block.GetHex());
            result.pushKV("timestamp", (uint64_t)timestamp);
            result.pushKV("bits", (uint64_t)bits);
            result.pushKV("nonce", (uint64_t)nonce);

            return result;
        }
    };

    class ShareData
    {
    public:
        uint256 previous_share_hash; //none — pack.PossiblyNoneType(0, pack.IntType(256))
        std::string coinbase;
        unsigned int nonce;         //pack.IntType(32)
        uint160 pubkey_hash;        //pack.IntType(160)
        unsigned long long subsidy; //pack.IntType(64)
        unsigned short donation;    //pack.IntType(16)
        StaleInfo stale_info;
        unsigned long long desired_version; //pack.VarIntType()

        ShareData(){};
        ShareData(uint256 previous_share_hash, std::string coinbase, unsigned int nonce, uint160 pubkey_hash, unsigned long long subsidy, unsigned short donation, StaleInfo stale_info, unsigned long long desired_version);

        friend std::istream &operator>>(std::istream &is, ShareData &value);
        friend std::ostream &operator<<(std::ostream &os, const ShareData &value);

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("previous_share_hash", previous_share_hash.GetHex());
            result.pushKV("coinbase", coinbase);
            result.pushKV("nonce", (uint64_t)nonce);
            result.pushKV("pubkey_hash", pubkey_hash.GetHex());
            result.pushKV("subsidy", subsidy);
            result.pushKV("donation", donation);
            result.pushKV("stale_info", (int)stale_info);
            result.pushKV("desired_version", desired_version);

            return result;
        }
    };

    class SegwitData
    {
        //SEGWIT DATA, 94 data.py
    public:
        std::shared_ptr<MerkleLink> txid_merkle_link; //---------------
        uint256 wtxid_merkle_root;                    //pack.IntType(256)

        //InitPossiblyNoneType
        SegwitData(){
            //TODO: txid_merkle_link=dict(branch=[], index=0)
            //TODO: wtxid_merkle_root=2**256-1
        };
        SegwitData(std::shared_ptr<MerkleLink> txid_merkle_link, uint256 wtxid_merkle_root);

        friend std::istream &operator>>(std::istream &is, SegwitData &value);
        friend std::ostream &operator<<(std::ostream &os, const SegwitData &value);

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("txid_merkle_link", *txid_merkle_link);
            result.pushKV("wtxid_merkle_root", wtxid_merkle_root.GetHex());

            return result;
        }
    };

    class TransactionHashRef
    {
    public:
        unsigned long long share_count; //VarIntType
        unsigned long long tx_count;    //VarIntType

        TransactionHashRef(){};
        TransactionHashRef(unsigned long long share_count, unsigned long long tx_count);

        friend std::istream &operator>>(std::istream &is, TransactionHashRef &value);
        friend std::ostream &operator<<(std::ostream &os, const TransactionHashRef &value);

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("share_count", share_count);
            result.pushKV("tx_count", tx_count);

            return result;
        }
    };

    class ShareInfoType
    {
    public:
        std::shared_ptr<ShareData> share_data;
        std::shared_ptr<SegwitData> segwit_data;

        std::vector<uint256> new_transaction_hashes;           //pack.ListType(pack.IntType(256))
        std::vector<TransactionHashRef> transaction_hash_refs; //pack.ListType(pack.VarIntType(), 2)), # pairs of share_count, tx_count
        uint256 far_share_hash;                                //none — pack.PossiblyNoneType(0, pack.IntType(256))
        unsigned int max_bits;                                 //bitcoin_data.FloatingIntegerType() max_bits;
        unsigned int bits;                                     //bitcoin_data.FloatingIntegerType() bits;
        unsigned int timestamp;                                //pack.IntType(32)
        unsigned long absheigth;                               //pack.IntType(32)

        uint128 abswork; //pack.IntType(128)

        ShareInfoType(){};
        ShareInfoType(std::shared_ptr<ShareData> share_data, std::vector<uint256> new_transaction_hashes, std::vector<TransactionHashRef> transaction_hash_refs, uint256 far_share_hash, unsigned int max_bits, unsigned int bits, unsigned int timestamp, unsigned long absheigth, uint128 abswork, std::shared_ptr<SegwitData> segwit_data = nullptr);

        friend std::istream &operator>>(std::istream &is, ShareInfoType &value);
        friend std::ostream &operator<<(std::ostream &os, const ShareInfoType &value);

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("share_data", *share_data);
            result.pushKV("segwit_data", *segwit_data);

            UniValue new_transaction_hashes_array;
            for (auto hash : new_transaction_hashes)
            {
                new_transaction_hashes_array.push_back(hash.GetHex());
            }
            result.pushKV("new_transaction_hashes", new_transaction_hashes_array);

            UniValue transaction_hash_refs_array;
            for (auto tx_hash_ref : transaction_hash_refs){
                transaction_hash_refs_array.push_back(tx_hash_ref);
            }
            result.pushKV("transaction_hash_refs", transaction_hash_refs_array);

            result.pushKV("far_share_hash", far_share_hash.GetHex());
            result.pushKV("max_bits", (uint64_t)max_bits);
            result.pushKV("bits", (uint64_t)bits);
            result.pushKV("timestamp", (uint64_t)timestamp);
            result.pushKV("absheight", (uint64_t)absheigth);
            result.pushKV("abswork", abswork.GetHex());

            return result;
        }
    };

    class ShareType
    {
    public:
        std::shared_ptr<SmallBlockHeaderType> min_header;
        std::shared_ptr<ShareInfoType> share_info;
        std::shared_ptr<MerkleLink> ref_merkle_link;
        unsigned long long last_txout_nonce; //IntType64
        std::shared_ptr<HashLinkType> hash_link;
        std::shared_ptr<MerkleLink> merkle_link;

        ShareType(){};
        ShareType(std::shared_ptr<SmallBlockHeaderType> min_header, std::shared_ptr<ShareInfoType> share_info, std::shared_ptr<MerkleLink> ref_merkle_link, unsigned long long last_txout_nonce, std::shared_ptr<HashLinkType> hash_link, std::shared_ptr<MerkleLink> merkle_link);

        friend std::istream &operator>>(std::istream &is, ShareType &value);
        friend std::ostream &operator<<(std::ostream &os, const ShareType &value);

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("min_header", *min_header);
            result.pushKV("share_info", *share_info);
            result.pushKV("ref_merkle_link", *ref_merkle_link);
            result.pushKV("last_txout_nonce", last_txout_nonce);
            result.pushKV("hash_link", *hash_link);
            result.pushKV("merkle_link", *merkle_link);

            return result;
        }
    };

    class RefType
    {
    public:
        std::string identifier; //TODO: pack.FixedStrType(64//8)
        std::shared_ptr<ShareInfoType> share_info;

        RefType(){};
        RefType(std::string identifier, std::shared_ptr<ShareInfoType> share_info);

        friend std::istream &operator>>(std::istream &is, RefType &value);
        friend std::ostream &operator<<(std::ostream &os, const RefType &value);

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("identifier", identifier);
            result.pushKV("share_info", *share_info);

            return result;
        }
    };

    //TODO:
    class Header
    {
    public:
        std::shared_ptr<SmallBlockHeaderType> header; //TODO: name: header?
        uint256 merkle_root;                          //TODO: arith_uint256?

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("header", *header);
            result.pushKV("merklke_root", merkle_root.GetHex());

            return result;
        }
    };

    class RawShare
    {
        /*
            share_type = pack.ComposedType([
                ('type', pack.VarIntType()),
                ('contents', pack.VarStrType()),
            ])
        */
    public:
        int type; //enum ShareVersion
        ShareType contents;

        friend std::istream &operator>>(std::istream &is, RawShare &value);
        friend std::ostream &operator<<(std::ostream &os, const RawShare &value);

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("type", type);
            result.pushKV("contents", contents);

            return result;
        }
    };
} // namespace c2pool::shares

#endif