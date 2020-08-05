#include <vector>
#include <iostream>
#include <math.h>

#include "net/http_client.h"
#include "int-util.h"
#include "rapidjson/document.h"
#include "cryptonote_core.h"
#include "ribbon.h"

namespace service_nodes {

ribbon_protocol::ribbon_protocol(cryptonote::core& core) : m_core(core){};

std::vector<exchange_trade> ribbon_protocol::trades_during_latest_1_block()
{
  std::vector<exchange_trade> trades = get_recent_trades();
  uint64_t top_block_height = m_core.get_current_blockchain_height() - 2;
  crypto::hash top_block_hash = m_core.get_block_id_by_height(top_block_height);
  cryptonote::block top_blk;
  m_core.get_block_by_hash(top_block_hash, top_blk);
  uint64_t top_block_timestamp = top_blk.timestamp;

  std::vector<exchange_trade> result;
  for (size_t i = 0; i < trades.size(); i++)
  {
    if (trades[i].date >= top_block_timestamp){
      result.push_back(trades[i]);
    }
  }
  return result;
}

bool get_trades_from_ogre(std::vector<exchange_trade> *trades)
{
  std::string url = "tradeogre.com";
  std::string uri = "/api/v1/orders/BTC-XEQ";

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
    rapidjson::Document document;
    document.Parse(res_info->m_body.c_str());
    for (size_t i = 0; i < document.Size(); i++)
    {
      exchange_trade trade;
      trade.date = document[i]["date"].GetUint64();
      trade.type = document[i]["type"].GetString();
      trade.price = std::stod(document[i]["price"].GetString()); // trade ogre gives this info as a string
      trade.quantity = std::stod(document[i]["quantity"].GetString());
      trades->push_back(trade);
    }
  }
  else
  {
    return false;
  }

  return true;
}

bool get_orders_from_ogre(std::vector<exchange_order> *orders)
{
  return false;
}


std::pair<double, double> get_gemini_btc_usd()
{
  return {0,0};
}

std::pair<double, double> get_nance_btc_usd()
{
    std::string url = "api.binance.com";
    std::string uri = "/api/v3/ticker/24hr?symbol=BTCUSDT";

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
      rapidjson::Document document;
      document.Parse(res_info->m_body.c_str());
      double btc_usd = 0;
      double vol_usd = 0;
      for (size_t i = 0; i < document.Size(); i++)
      {
        btc_usd = std::stod(document["price"].GetString()) * 100000000;
        vol_usd = std::stod(document["quoteVolume"].GetString());
      }
      return {btc_usd, vol_usd};
    }
    return {0,0};
}

uint64_t get_bittrex_price(std::pair<std::string, std::string> pair)
{

  std::string url = "api.bittrex.com";
  std::string uri = "/v3/markets/" + pair.second + "-" + pair.first + "/ticker" ;

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


std::pair<double, double> get_stamp_btc_usd()
{
   return {0,0};
}

uint64_t create_bitcoin_a(){
  //std::pair<double, double> gemini_usd = get_gemini_btc_usd();
  //std::pair<double, double> coinbase_pro_usd = get_coinbase_pro_btc_usd();
  std::pair<double, double> nance_usd = get_nance_btc_usd();
  uint64_t bittrex_usd = get_bittrex_price({"BTC", "USD"});
  return static_cast<uint64_t>((bittrex_usd + nance_usd.first) / 2);
  //double t_vol = gemini_usd.second + coinbase_pro_usd.second + bitfinex_usd.second + nance_usd.second + stamp_usd.second;
  //double weighted_values = (gemini_usd.first * gemini_usd.second) + (coinbase_pro_usd.first * coinbase_pro_usd.second) + (bitfinex_usd.first * bitfinex_usd.second) + (nance_usd.first * nance_usd.second) + (stamp_usd.first * stamp_usd.second);
  
  //return static_cast<uint64_t>(weighted_values / t_vol);
}

double get_usd_average(){
  // std::pair<double, double> gemini_usd = get_gemini_btc_usd();
  // std::pair<double, double> coinbase_pro_usd = get_coinbase_pro_btc_usd();
  // std::pair<double, double> bitfinex_usd = get_bitfinex_btc_usd();
  // std::pair<double, double> nance_usd = get_nance_btc_usd();
  // std::pair<double, double> stamp_usd = get_stamp_btc_usd();

  // //Sometimes coinbase pro returns 0? Need to look into this.
  // if(coinbase_pro_usd.first == 0)
  //   return (gemini_usd.first + bitfinex_usd.first + nance_usd.first + stamp_usd.first) / 4;

  // return (gemini_usd.first + coinbase_pro_usd.first + bitfinex_usd.first + nance_usd.first + stamp_usd.first) / 5;
  std::pair<double, double> nance_usd = get_nance_btc_usd();
  uint64_t bittrex_usd = get_bittrex_price({"BTC", "USD"});
  return (bittrex_usd + nance_usd.first) / 2;
}

double ribbon_protocol::get_btc_b(){
  
  crypto::hash block_hash = m_core.get_block_id_by_height(m_core.get_current_blockchain_height() - 1);
  cryptonote::block blk;
  m_core.get_block_by_hash(block_hash, blk);
  return static_cast<double>(blk.btc_b);
}

uint64_t ribbon_protocol::convert_btc_to_usd(double btc)
{
  double usd_average = get_btc_b();

  if(usd_average == 0)
    usd_average = get_usd_average();
  
	double usd = usd_average * btc;
	return static_cast<uint64_t>(usd * 100000); // remove "cents" decimal place and convert to integer
}

uint64_t convert_btc_to_usd(double btc)
{
	double usd = get_usd_average() * btc;
	return static_cast<uint64_t>(usd * 100000); // remove "cents" decimal place and convert to integer
}

uint64_t create_ribbon_blue(std::vector<exchange_trade> trades)
{
  double filtered_mean = filter_trades_by_deviation(trades);
  return convert_btc_to_usd(filtered_mean);
}

//Volume Weighted Average
uint64_t create_ribbon_green(std::vector<exchange_trade> trades)
{
  double weighted_mean = trades_weighted_mean(trades);
  return convert_btc_to_usd(weighted_mean);
}

uint64_t get_volume_for_block(std::vector<exchange_trade> trades)
{
  double volume = 0;
  if(trades.size() == 0)
    return 0;

  for(size_t i = 0; i < trades.size();i++){
    volume += (trades[i].price * trades[i].quantity);
  }
  return convert_btc_to_usd(volume);
}


//Volume Weighted Average with 2 STDEV trades removed
double filter_trades_by_deviation(std::vector<exchange_trade> trades)
{
  double weighted_mean = trades_weighted_mean(trades);
  int n = trades.size();
  double sum = 0;
  
  for (size_t i = 0; i < trades.size(); i++)
  {
    sum += pow((trades[i].price - weighted_mean), 2.0);
  }
  
  double deviation = sqrt(sum / (double)n);
  
  double max = weighted_mean + (2 * deviation);
  double min = weighted_mean - (2 * deviation);
  
  for (size_t i = 0; i < trades.size(); i++)
  {
    if (trades[i].price > max || trades[i].price < min)
      trades.erase(trades.begin() + i);
  }

  return trades_weighted_mean(trades);
}

double trades_weighted_mean(std::vector<exchange_trade> trades)
{
  double XEQ_volume_sum = 0;
  double weighted_sum = 0;
  for (size_t i = 0; i < trades.size(); i++)
  {
    XEQ_volume_sum += trades[i].quantity;
    weighted_sum += (trades[i].price * trades[i].quantity);
  }
  
  return weighted_sum / XEQ_volume_sum;
}

std::vector<exchange_trade> get_recent_trades()
{
  std::vector<service_nodes::exchange_trade> trades;
  if(!service_nodes::get_trades_from_ogre(&trades))
    MERROR("Error getting trades from Ogre");

  return trades;
}


std::vector<adjusted_liquidity> get_recent_liquids(double blue)
{
  std::vector<exchange_order> orders;
  if(!get_orders_from_ogre(&orders))
    MERROR("Error getting orders from TradeOgre");
  //more exchanges below
  std::vector<adjusted_liquidity> adj_liquid = create_adjusted_liqudities(orders, blue);
  return adj_liquid;
}


std::vector<adjusted_liquidity> create_adjusted_liqudities(std::vector<exchange_order> orders, double spot){
  std::vector<adjusted_liquidity> al;

  for(size_t i = 0; i < orders.size();i++){
      adjusted_liquidity this_al;
      if(orders[i].price != spot){
        this_al.price = orders[i].price;
        double denom = (1 - std::abs(this_al.price - spot));
        this_al.liquid = (orders[i].quantity) * (1 / denom);
        al.push_back(this_al);
      }
  }

  return al;
}

double create_mac(std::vector<adjusted_liquidity> adj_liquids){
  double adj_liquid_sum = 0;
  double price_weighted = 0;

  for(size_t i = 0; i < adj_liquids.size(); i++){
    adj_liquid_sum += adj_liquids[i].liquid;
    price_weighted += (adj_liquids[i].liquid * adj_liquids[i].price);
  }
  return price_weighted * adj_liquid_sum;
}

}
