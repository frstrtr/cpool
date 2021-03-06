#pragma once

#include "messages.h"
using namespace c2pool::libnet::messages;
namespace c2pool
{
    class Network;

    namespace libnet::p2p
    {
        class Protocol;
        class initialize_network_protocol;
    }
} // namespace c2pool

#include <memory>
#include <networks/network.h>
#include <boost/asio.hpp>
#include <boost/function.hpp>
namespace ip = boost::asio::ip;

namespace c2pool::libnet::p2p
{
    typedef boost::function<bool(std::shared_ptr<c2pool::libnet::p2p::Protocol>)> protocol_handle;

    class P2PSocket : public std::enable_shared_from_this<P2PSocket>
    {
    public:
        //for receive
        P2PSocket(ip::tcp::socket socket, shared_ptr<c2pool::Network> _network);

        //for connect
        void connector_init(protocol_handle handle, const boost::asio::ip::tcp::resolver::results_type endpoints);

        void init(protocol_handle handle);

        template <class protocol_type>
        void set_protocol_type_and_version(protocol_handle handle, std::shared_ptr<raw_message> raw_message_version);

        bool isConnected() const { return _socket.is_open(); }
        ip::tcp::socket &get() { return _socket; }
        void disconnect() { _socket.close(); }

        ip::tcp::endpoint endpoint()
        {
            boost::system::error_code ec;
            return _socket.remote_endpoint(ec);
        }

        void write(std::shared_ptr<base_message> msg);

    private:
        void start_read();
        void read_prefix(std::shared_ptr<raw_message> tempRawMessage);
        void read_command(std::shared_ptr<raw_message> tempRawMessage);
        void read_length(std::shared_ptr<raw_message> tempRawMessage);
        void read_checksum(shared_ptr<raw_message> tempRawMessage);
        void read_payload(std::shared_ptr<raw_message> tempRawMessage);

        void write_prefix(std::shared_ptr<base_message> msg);
        void write_message_data(std::shared_ptr<base_message> msg);

    private:
        std::shared_ptr<c2pool::Network> _net;
        ip::tcp::socket _socket;

        std::weak_ptr<c2pool::libnet::p2p::Protocol> _protocol;

        std::shared_ptr<initialize_network_protocol> initialize_protocol;
    };
} // namespace c2pool::p2p