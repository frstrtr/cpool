#pragma once

#include <string>
#include <sstream>
#include "univalue.h"
#include <devcore/common.h>

//todo: move all methods to types.cpp
namespace c2pool::util::messages
{

    class address_type
    {
    public:
        /*
            ('services', pack.IntType(64)),
            ('address', pack.IPV6AddressType()),
            ('port', pack.IntType(16, 'big')),
         */
        unsigned long long services;
        std::string address;
        int port;

        address_type();

        address_type(unsigned long long _services, std::string _address, int _port);

        address_type &operator=(UniValue value)
        {
            services = c2pool::dev::str_to_int<unsigned long long>(value["services"].get_str());

            address = value["address"].get_str();
            port = value["port"].get_int();
            return *this;
        }

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("services", (uint64_t)services);
            result.pushKV("address", address);
            result.pushKV("port", port);

            return result;
        }

        friend bool operator==(const address_type &first, const address_type &second);

        friend bool operator!=(const address_type &first, const address_type &second);
    };

    class share_type
    {
    public:
        /*
            ('type', pack.VarIntType()),
            ('contents', pack.VarStrType()),
        */
        int type;
        std::string contents;

        share_type();

        share_type(int _type, std::string _contents);

        share_type &operator=(UniValue value)
        {
            type = value["type"].get_int();
            contents = value["contents"].get_str();
            return *this;
        }

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("type", type);
            result.pushKV("contents", contents);

            return result;
        }
    };

    class addr
    {
    public:
        int timestamp;
        address_type address;

        addr();

        addr(int t, address_type a);

        addr(int t, int _services, std::string _address, int _port);

        addr &operator=(UniValue value)
        {
            timestamp = c2pool::dev::str_to_int<int>(value["timestamp"].get_str());


            address = value["address"].get_obj();
            return *this;
        }

        operator UniValue()
        {
            UniValue result(UniValue::VOBJ);

            result.pushKV("timestamp", timestamp);
            result.pushKV("contents", address);

            return result;
        }

        friend bool operator==(const addr &first, const addr &second);

        friend bool operator!=(const addr &first, const addr &second);
    };
} // namespace c2pool::messages