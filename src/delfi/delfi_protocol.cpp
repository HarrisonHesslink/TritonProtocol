#include <queue>
#include <math.h>

#include "rapidjson/document.h"
#include "net/http_client.h"
#include "delfi_protocol.h"
#include "int-util.h"

namespace delfi_protocol {



    Delfi::Delfi(){
        if(!scanTasks()){
            LOG_PRINT_L1("Unable to scan tasks!");
        }
    }

    bool Delfi::scanTasks(){}   

    uint64_t getCoinbasePrice(std::pair<std::string, std::string> pair)
    {
        std::string url = "api.pro.coinbase.com";
        std::string uri = "/products/" + pair.first + "-" + pair.second + "/stats";

        LOG_PRINT_L1("Requesting coinbase pricing");

        epee::net_utils::http::http_simple_client http_client;
        epee::net_utils::http::http_response_info res;
        const epee::net_utils::http::http_response_info *res_info = &res;
        epee::net_utils::http::fields_list fields;
        std::string body;
        http_client.set_server(url, "443",  boost::none);
        bool r = true;
        r = http_client.invoke_get(uri, std::chrono::milliseconds(1000000), "", &res_info, fields);

        if(res_info){
            MDEBUG("response code: " << res_info->m_response_code);
            MDEBUG("response length: " << res_info->m_header_info.m_content_length);
            MDEBUG("response comment: " << res_info->m_response_comment);
            MDEBUG("response body: " << res_info->m_body);

            rapidjson::Document d;
            d.Parse(res_info->m_body.c_str());
            if(d.Size() < 0)
                return 0;
            for (size_t i = 0; i < d.Size(); i++)
            {  
                return 0;
            }
        }
        else
        {
            return 0;
        }
        
        return 0;
    }

    uint64_t getTradeOgrePrice(std::pair<std::string, std::string> pair)
    {
        std::string url = "tradeogre.com";
        std::string uri = "/api/v1/ticker/" + pair.first + "-" + pair.second;

        epee::net_utils::http::http_simple_client http_client;
        const epee::net_utils::http::http_response_info *res_info = nullptr;
        epee::net_utils::http::fields_list fields;
        std::string body;

        http_client.set_server(url, "443",  boost::none);
        http_client.connect(std::chrono::seconds(10));
        bool r = true;
        r = http_client.invoke_get(uri, std::chrono::seconds(1), "", &res_info, fields);

        if(res_info){
            body = res_info->m_body;
            MDEBUG("response code: " << res_info->m_response_code);
            MDEBUG("response length: " << res_info->m_header_info.m_content_length);
            MDEBUG("response comment: " << res_info->m_response_comment);
            MDEBUG("response body: " << body);

            rapidjson::Document d;
            d.Parse(body.c_str());
            if(d.Size() < 0)
                return 0;
            for (size_t i = 0; i < d.Size(); i++)
            {  
                uint64_t t = static_cast<uint64_t>(std::stod(d["price"].GetString()) * 100000000);
                std::cout << t << std::endl;
                return static_cast<uint64_t>(std::stod(d["price"].GetString()) * 100000000);
            }

        }
        else
        {
            return 0;
        }
        

        return 0;
    }
    uint64_t getBittrexPrice(std::pair<std::string, std::string> pair)
    {

        std::string url = "api.bittrex.com";
        std::string uri = "/v3/markets/" + pair.second + "-" + pair.first + "/ticker" ;

        LOG_PRINT_L1("Requesting bittrex pricing");

        epee::net_utils::http::http_simple_client http_client;
        const epee::net_utils::http::http_response_info *res_info = nullptr;
        epee::net_utils::http::fields_list fields;
        std::string body;

        http_client.set_server(url, "443",  boost::none);
        http_client.connect(std::chrono::seconds(10));
        bool r = true;
        r = http_client.invoke_get(uri, std::chrono::seconds(1), "", &res_info, fields);
        if(res_info){
            MDEBUG("response code: " << res_info->m_response_code);
            MDEBUG("response length: " << res_info->m_header_info.m_content_length);
            MDEBUG("response comment: " << res_info->m_response_comment);
            MDEBUG("response body: " << res_info->m_body);

            rapidjson::Document d;
            d.Parse(res_info->m_body.c_str());
            if(d.Size() < 0)
                return 0;
            for (size_t i = 0; i < d.Size(); i++)
            {  
                return static_cast<uint64_t>(std::stod(d["lastTradeRate"].GetString()) * 100000000);
            }

        }
        else
        {
            return 0;
        }
        

        return 0;
    }
    uint64_t getNancePrice(std::pair<std::string, std::string> pair)
    {
        std::string url = "api.binance.com";
        std::string uri = "/api/v3/ticker/price?symbol=" +  pair.second + pair.first;

        LOG_PRINT_L1("Requesting binance pricing");

        epee::net_utils::http::http_simple_client http_client;
        const epee::net_utils::http::http_response_info *res_info = nullptr;
        epee::net_utils::http::fields_list fields;
        std::string body;

        http_client.set_server(url, "443",  boost::none);
        http_client.connect(std::chrono::seconds(10));
        bool r = true;
        r = http_client.invoke_get(uri, std::chrono::seconds(1), "", &res_info, fields);
        if(res_info){
            MDEBUG("response code: " << res_info->m_response_code);
            MDEBUG("response length: " << res_info->m_header_info.m_content_length);
            MDEBUG("response comment: " << res_info->m_response_comment);
            MDEBUG("response body: " << res_info->m_body);

            rapidjson::Document d;
            d.Parse(res_info->m_body.c_str());
            if(d.Size() < 0)
                return 0;
            for (size_t i = 0; i < d.Size(); i++)
            {  
                return static_cast<uint64_t>(std::stod(d["price"].GetString()) * 100000000);
            }
        }

        return 0;
    }
    uint64_t getKucoinPrice(std::pair<std::string, std::string> pair)
    {
        std::string url = "api.kucoin.com";
        std::string uri = "/api/v1/market/orderbook/level1?symbol=" + pair.first + "-" + pair.second;

        LOG_PRINT_L1("Requesting kucoin pricing");

        epee::net_utils::http::http_simple_client http_client;
        const epee::net_utils::http::http_response_info *res_info = nullptr;
        epee::net_utils::http::fields_list fields;
        std::string body;

        http_client.set_server(url, "443",  boost::none);
        http_client.connect(std::chrono::seconds(10));
        bool r = true;
        r = http_client.invoke_get(uri, std::chrono::seconds(1), "", &res_info, fields);
        if(res_info){
            MDEBUG("response code: " << res_info->m_response_code);
            MDEBUG("response length: " << res_info->m_header_info.m_content_length);
            MDEBUG("response comment: " << res_info->m_response_comment);
            MDEBUG("response body: " << res_info->m_body);

            rapidjson::Document d;
            d.Parse(res_info->m_body.c_str());
        
            if(d.Size() < 0)
                return 0;

            for (size_t i = 0; i < d.Size(); i++)
            {  
                return static_cast<uint64_t>(std::stod(d["data"]["price"].GetString()) * 100000000);
            }

        }

        return 0;
    }
}