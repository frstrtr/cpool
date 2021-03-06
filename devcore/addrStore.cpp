#include "addrStore.h"
#include "logger.h"
#include "common.h"
#include <univalue.h>
#include <networks/network.h>

#include <fstream>
#include <string>
#include <memory>

using std::string;
using std::shared_ptr;

namespace c2pool::dev
{
    //todo: boost::filesystem
    //template path — "data//bitcoin//addrs"
    AddrStore::AddrStore(string path, shared_ptr<c2pool::Network> net)
    {
        filePath = path;
        std::fstream AddrsFile(path, std::ios_base::in);

        //FILE
        //exist file
        if (AddrsFile)
        {
            string json;
            //Если будет баг с тем, что файл как-то не так читается, то винить эту строку, не меня.
            //Кто же знал, что вы будете разделять json файл на кучу строк.
            AddrsFile >> json;
            FromJSON(json);
        }
        else
        {
            LOG_WARNING << "AddrsFile not found!";
        }

        //BOOTSTRAP
        for (auto key : net->BOOTSTRAP_ADDRS)
        {
            store[key] = {
                0,
                (double)c2pool::dev::timestamp(),
                (double)c2pool::dev::timestamp()};
        }

        //SAVE IN FILE
        if (store.size() > 0)
        {
            SaveToFile();
        }
        else
        {
            LOG_WARNING << "AddrStore is empty!";
        }

        AddrsFile.close();
    }

    void AddrStore::SaveToFile()
    {
        std::fstream AddrsFile(filePath, std::ios_base::out);
        AddrsFile << ToJSON();

        AddrsFile.close();
        LOG_DEBUG << "Addrs saved in file!";
    }

    bool AddrStore::Check(ADDR key)
    {
        if (store.find(key) != store.end())
            return true;
        else
            return false;
    }

    AddrValue AddrStore::Get(ADDR key)
    {
        if (Check(key))
            return store[key];
        else
            return EMPTY_ADDR_VALUE;
    }

    std::vector<std::pair<ADDR, AddrValue>> AddrStore::GetAll()
    {
        std::vector<std::pair<ADDR, AddrValue>> res;
        for (auto kv : store){
            res.push_back(kv);
        }
        return res;
    }

    bool AddrStore::Add(ADDR key, AddrValue value)
    {
        if (Check(key))
            return false;
        store.insert(std::pair<ADDR, AddrValue>(key, value));
        SaveToFile();
        return true;
    }

    bool AddrStore::Remove(ADDR key)
    {
        if (Check(key))
            return false;
        store.erase(key);
        SaveToFile();
        return true;
    }

    string AddrStore::ToJSON()
    {
        UniValue dict(UniValue::VARR);

        for (auto kv : store)
        {
            UniValue value(UniValue::VOBJ);

            value.pushKV("address", std::get<0>(kv.first));
            value.pushKV("port", std::get<1>(kv.first));
            value.pushKV("services", kv.second.service);
            value.pushKV("first_seen", kv.second.first_seen);
            value.pushKV("last_seen", kv.second.last_seen);

            dict.push_back(value);
        }

        std::string json = dict.write();
        return json;
    }

    void AddrStore::FromJSON(string json)
    {
        UniValue AddrsValue(UniValue::VARR);
        AddrsValue.read(json); //TODO: add check for valid json.

        for (int i = 0; i < AddrsValue.size(); i++)
        {
            ADDR key = std::make_tuple(AddrsValue[i]["address"].get_str(),
                                       AddrsValue[i]["port"].get_str());
            store[key] = {AddrsValue[i]["services"].get_int(),
                          AddrsValue[i]["first_seen"].get_real(),
                          AddrsValue[i]["last_seen"].get_real()};
        }
    }
} // namespace c2pool::p2p