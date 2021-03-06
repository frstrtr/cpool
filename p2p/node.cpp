#include "node.h"

#include "config.h"
#include "other.h"
#include "protocol.h"
#include "factory.h"
#include "console.h"
#include "addrStore.h"
#include "events.h

#include <boost/exception/all.hpp> //TODO: all reason = boost::exception???
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <map>
#include <set>
#include <memory>
#include <iostream>
#include <cmath>
#include <iterator>

using std::log;
using std::max;
using std::min;

//c2pool::p2p::NodesManager
namespace c2pool::p2p
{
    NodesManager::NodesManager(boost::asio::io_context &_io, c2pool::config::Network *_networkConfig) : _io_context(_io)
    {
        LOG_INFO << "Initialization NodesManager...";
        _net = _networkConfig; //TODO
    }

} // namespace c2pool::p2p

//c2pool::p2p::Node
namespace c2pool::p2p
{
    Node::Node(std::shared_ptr<c2pool::p2p::NodesManager> _nodes, std::string _port, c2pool::p2p::AddrStore _addr_store) : INode(_nodes), _think_timer(_nodes->io_context(), boost::posix_time::seconds(0)), addr_store(std::move(_addr_store))
    {
        LOG_INFO << "Start Node initialization...";
        nonce = c2pool::random::RandomNonce();
        LOG_INFO << "Node nonce generated: ";
        port = _port;

        //boost::asio::io_context &io_context_, shared_ptr<c2pool::p2p::NodesManager> _nodes, int _desired_conns, int _max_attempts
        client = std::make_unique<c2pool::p2p::Client>(_nodes->io_context(), _nodes, 10, 30); // client.start()

        //boost::asio::io_context &io_context_, shared_ptr<c2pool::p2p::NodesManager> _nodes, const tcp::endpoint &endpoint, int _max_conns
        tcp::endpoint the_endpoint(tcp::v4(), atoi(_port.c_str()));                                     //ipv4, port; atoi -> str to int
        server = std::make_unique<c2pool::p2p::Server>(_nodes->io_context(), _nodes, the_endpoint, 50); // server.start()

        //todo? self.singleclientconnectors = [reactor.connectTCP(addr, port, SingleClientFactory(self)) for addr, port in self.connect_addrs]

        _think_timer.async_wait(boost::bind(&Node::_think, this, boost::asio::placeholders::error));
        LOG_INFO << "Node created.";
    }

    void Node::got_conn(std::shared_ptr<c2pool::p2p::Protocol> protocol)
    {
        if (peers.count(protocol->nonce()) != 0)
        {
            std::cout << "Already have peer!" << std::endl; //TODO: raise ValueError('already have peer')
        }
        peers.insert(std::make_pair<int, std::shared_ptr<c2pool::p2p::Protocol>>(protocol->nonce(), protocol->shared_from_this()));
    }

    void Node::lost_conn(std::shared_ptr<c2pool::p2p::Protocol> protocol, boost::exception *reason)
    {
        if (peers.count(protocol->nonce()) == 0)
        {
            std::cout << "Don't have peer!" << std::endl; //TODO: raise ValueError('''don't have peer''')
            return;
        }

        if (protocol != peers.at(protocol->nonce()))
        {
            std::cout << "Wrong conn!" << std::endl; //TODO: raise ValueError('wrong conn')
            return;
        }

        protocol.reset(); //delete for smart pointer

        //todo: print 'Lost peer %s:%i - %s' % (conn.addr[0], conn.addr[1], reason.getErrorMessage())
    }

    void Node::_think(const boost::system::error_code &error)
    { //TODO: rename method?
        LOG_DEBUG << "Node _think.";
        if (peers.size() > 0)
        {
            int pos = c2pool::random::RandomInt(0, peers.size());
            auto item = peers.begin();
            std::advance(item, pos);
            auto proto = item->second;
            auto msg = new c2pool::messages::message_getaddrs(8);
            proto->send(msg);
            //c2pool::random::RandomChoice<unsigned long long, std::shared_ptr<c2pool::p2p::Protocol>>(peers)->send(std::make_unique<c2pool::messages::message_getaddrs>(8)); //TODO: add send_getaddrs to c2pool::p2p::Protocol
        }
        float rand = c2pool::random::Expovariate(20);
        boost::posix_time::milliseconds interval(static_cast<int>(rand * 1000));
        LOG_DEBUG << "[Node::_think()] Expovariate seconds: " << rand;
        _think_timer.expires_at(_think_timer.expires_at() + interval);
        _think_timer.async_wait(boost::bind(&Node::_think, this, boost::asio::placeholders::error));
    }

    std::vector<ADDR> Node::get_good_peers(int max_count)
    {

        //TODO: this want to test
        float t = c2pool::time::timestamp();
        
        std::vector<std::pair<float, ADDR>> values;
        for (auto kv : addr_store.GetAll())
        {
            values.push_back(
                std::make_pair(
                    -log(max(3600.0, kv.second.last_seen - kv.second.first_seen)) / log(max(3600.0, t - kv.second.last_seen)) * c2pool::random::Expovariate(1),
                    kv.first));
        }

        std::sort(values.begin(), values.end(), [](std::pair<float, ADDR> a, std::pair<float, ADDR> b) {
            return a.first < b.first;
        });

        values.resize(min((int)values.size(), max_count));
        std::vector<ADDR> result;
        for (auto v : values){
            result.push_back(v.second);
        }
        return result;
    }

    //P2PNode

    P2PNode::P2PNode(std::shared_ptr<c2pool::p2p::NodesManager> _nodes, std::string _port, c2pool::p2p::AddrStore _addr_store) : Node(_nodes, _port, _addr_store)
    {
        LOG_INFO << "P2PNode created!";
    }
} // namespace c2pool::p2p