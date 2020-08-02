// Copyright (c) 2020 Harrison Hesslink
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers
    
#include <math.h>

#include "rapidjson/document.h"
#include "net/http_client.h"
#include "price_provider.h"
#include "int-util.h"

namespace price_provider {
 
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
        if(res_info)
        {
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
        if(res_info)
        {
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
        if(res_info)
        {
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

    uint64_t getHuobiPrice(std::pair<std::string, std::string> pair)
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
        if(res_info)
        {
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

    uint64_t getFOREX(std::pair<std::string, std::string> pair)
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
        if(res_info)
        {
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

    uint64_t getILIFOREX(std::pair<std::string, std::string> pair)
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
        if(res_info)
        {
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