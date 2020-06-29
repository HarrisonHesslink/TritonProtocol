#include <queue>

#include "net/http_client.h"
#include "storages/http_abstract_invoke.h"
#include "delphi_protocol.h"

namespace delphi_protocol {
    Delphi::Delphi(){
        if(!scanTasks()){
            LOG_PRINT_L1("Unable to scan tasks!");
        }
    }

    bool Delphi::scanTasks(){}   

    bool getCoinbasePrice(uint64_t& price, std::pair<std::string, std::string> pair)
    {
        epee::net_utils::http::http_simple_client http_client;
        cryptonote::COMMAND_RPC_COINBASE::request req = AUTO_VAL_INIT(req);
        cryptonote::COMMAND_RPC_COINBASE::response res = AUTO_VAL_INIT(res);

        http_client.set_server("api.pro.coinbase.com:443", boost::none, epee::net_utils::ssl_support_t::e_ssl_support_autodetect);

        std::string url = "/products/" + pair.first + pair.second + "/stats";

        LOG_PRINT_L1("Requesting coinbase pricing");

        bool r = epee::net_utils::invoke_http_json(url, req, res, http_client, std::chrono::seconds(10), "GET");

        if(!r){
            LOG_PRINT_L1("Requesting coinbase pricing failed!");
        }

        uint64_t tmp_price = std::stoi(res.last);

        if(tmp_price == 0){
            LOG_PRINT_L1("Last Price is 0");
            return false;
        }
        else 
        {
            price = tmp_price;
        }

        return true;
    }

    bool getTradeOgrePrice(uint64_t& price, std::pair<std::string, std::string> pair)
    {
        epee::net_utils::http::http_simple_client http_client;
        cryptonote::COMMAND_RPC_TRADEOGRE::request req = AUTO_VAL_INIT(req);
        cryptonote::COMMAND_RPC_TRADEOGRE::response res = AUTO_VAL_INIT(res);

        http_client.set_server("tradeogre.com", "443", boost::none);

        std::string url = "/api/v1/ticker/" + pair.first + "-" + pair.second;

        LOG_PRINT_L1("Requesting tradeogre pricing");

        bool r = epee::net_utils::invoke_http_json(url, req, res, http_client, std::chrono::seconds(10), "GET");

        if(!r){
            LOG_PRINT_L1("Requesting tradeogre pricing failed!");
        }
        std::cout << url << std::endl;
        std::cout << res.price << std::endl;
        uint64_t tmp_price = std::stoi(res.price);

        if(tmp_price == 0){
            LOG_PRINT_L1("Last Price is 0");
            return false;
        }
        else 
        {
            price = tmp_price;
        }

        return true;
    }
    bool getBittrexPrice(uint64_t& price, std::pair<std::string, std::string> pair)
    {

        epee::net_utils::http::http_simple_client http_client;
        cryptonote::COMMAND_RPC_BITTREX::request req = AUTO_VAL_INIT(req);
        cryptonote::COMMAND_RPC_BITTREX::response res = AUTO_VAL_INIT(res);

        http_client.set_server("api.bittrex.com:443", boost::none, epee::net_utils::ssl_support_t::e_ssl_support_autodetect);

        std::string url = "/v3/markets/" + pair.second + pair.first + "/ticker" ;

        LOG_PRINT_L1("Requesting bittrex pricing");

        bool r = epee::net_utils::invoke_http_json(url, req, res, http_client, std::chrono::seconds(10), "GET");

        if(!r){
            LOG_PRINT_L1("Requesting bittrex pricing failed!");
        }

        uint64_t tmp_price = std::stoi(res.lastTradeRate);

        if(tmp_price == 0){
            LOG_PRINT_L1("Last Price is 0");
            return false;
        }
        else 
        {
            price = tmp_price;
        }
        return true;
    }
    bool getNancePrice(uint64_t& price, std::pair<std::string, std::string> pair)
    {
        epee::net_utils::http::http_simple_client http_client;
        cryptonote::COMMAND_RPC_NANCE::request req = AUTO_VAL_INIT(req);
        cryptonote::COMMAND_RPC_NANCE::response res = AUTO_VAL_INIT(res);

        http_client.set_server("api.bittrex.com:443", boost::none, epee::net_utils::ssl_support_t::e_ssl_support_autodetect);

        std::string url = "/api/v3/ticker?symbol=" +  pair.second + pair.first;

        LOG_PRINT_L1("Requesting bittrex pricing");

        bool r = epee::net_utils::invoke_http_json(url, req, res, http_client, std::chrono::seconds(10), "GET");

        if(!r){
            LOG_PRINT_L1("Requesting bittrex pricing failed!");
        }
        uint64_t tmp_price = std::stoi(res.lastPrice);

        if(tmp_price == 0){
            LOG_PRINT_L1("Last Price is 0");
            return false;
        }
        else 
        {
            price = tmp_price;
        }
        return true;
    }
    bool getKucoinPrice(uint64_t& price, std::pair<std::string, std::string> pair)
    {
        epee::net_utils::http::http_simple_client http_client;
        cryptonote::COMMAND_RPC_KUCOIN::request req = AUTO_VAL_INIT(req);
        cryptonote::COMMAND_RPC_KUCOIN::response res = AUTO_VAL_INIT(res);

        http_client.set_server("api.kucoin.com:443", boost::none, epee::net_utils::ssl_support_t::e_ssl_support_autodetect);

        std::string url = "/api/v1/market/orderbook/level1?symbol=" + pair.first + "-" + pair.second;

        LOG_PRINT_L1("Requesting kucoin pricing");

        bool r = epee::net_utils::invoke_http_json(url, req, res, http_client, std::chrono::seconds(10), "GET");

        if(!r){
            LOG_PRINT_L1("Requesting kucoin pricing failed!");
        }
        uint64_t tmp_price = std::stoi(res.last);

        if(tmp_price == 0){
            LOG_PRINT_L1("Last Price is 0");
            return false;
        }
        else 
        {
            price = tmp_price;
        }


        return true;
    }
}