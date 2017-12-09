#define BOOST_COROUTINES_NO_DEPRECATION_WARNING
#include "server.hpp"
#include "game.hpp"
#include "objects/spaceship.hpp"

#include <unordered_map>
#include <thread>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "websocket.hpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using Conn = std::shared_ptr<WsServer::Connection>;
using boost::property_tree::read_json;
using boost::property_tree::ptree;


class Connection
{
public:
    virtual ~Connection() = default;
    virtual void on_message(const ptree& json) = 0;
    virtual void on_close(int status, const std::string& reason)
    {
        std::cout << "Closed connection: " << reason << std::endl;
    }

    virtual void on_error(const boost::system::error_code& ec)
    {
        std::cerr << ec << std::endl;
    }

    template<typename Handler>
    void post(Handler handler) {
        Game::Current().service()->post(handler);
    }
};

class UploadConnection: public Connection
{
public:
    UploadConnection(WsServer& server, Conn conn)
        : server(server)
        , conn(conn)
    {
        auto hash = conn->path_match[1];
        std::cout << "new connection with id " << hash << std::endl;
        player = Game::Current().get_player_by_hash(hash);

        if (!player) {
            std::cout << "No player with id '" << hash << "'" << std::endl;
            server.send_close(conn, 1, "Unknown player hash");
        }
    }

    void on_message(const ptree& json) override
    {
        auto path = json.get<std::string>("path");
        auto code = json.get<std::string>("code");

        post([path, code, this]() {
            player->mainShip->interact_send_code(path, code);
            player->mainShip->interact_reboot();
        });
    }

private:
    WsServer& server;
    Conn conn;
    Player* player;
};

class ServerImpl: public Server
{
public:
    ServerImpl()
    {
    }

    ~ServerImpl() override
    {
    }

    void start(int port) override
    {
        server.config.port = port;

        make_endpoint<UploadConnection>("^/upload/([a-z]+)$");

        std::cout << "start server thread..." << std::endl;
        server_thread = std::thread([this](){
            //Start WS-server
            server.start();
        });
    }

private:
    template<typename Handler>
    void make_endpoint(const std::string& path)
    {
        auto& endpoint = server.endpoint[path];

        endpoint.on_open=[this](std::shared_ptr<WsServer::Connection> connection) {
            conns[connection] = std::make_shared<Handler>(server, connection);
        };
        endpoint.on_close=[this](std::shared_ptr<WsServer::Connection> connection, int status, const std::string& reason) {
            conns[connection]->on_close(status, reason);
        };
        endpoint.on_error=[this](std::shared_ptr<WsServer::Connection> connection, const boost::system::error_code& ec) {
            conns[connection]->on_error(ec);
        };
        endpoint.on_message = [this](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::Message> message) {
            try {
                ptree json;
                std::string jsonCode = message->string();
                std::cout << "msg: " << jsonCode << std::endl;
                std::stringstream iss(jsonCode);
                read_json(iss, json);
                conns[connection]->on_message(json);
            } catch (boost::property_tree::json_parser_error& e)
            {
                std::cerr << "Failed to parse json: " << e.what() << std::endl;
            } catch (...)
            {
                std::cerr << "Error while message" << std::endl;
            }
        };
    }

private:
    WsServer server;
    std::thread server_thread;

    std::unordered_map<Conn, std::shared_ptr<Connection>> conns;
};

std::unique_ptr<Server> Server::create()
{
    return std::make_unique<ServerImpl>();
}