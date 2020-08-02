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
#include "delfi_protocol.h"
#include "int-util.h"
#include "price_provider.h"

namespace delfi_protocol {

    static crypto::hash make_hash(crypto::public_key const &pubkey, uint64_t timestamp, delfi_protocol::oracle_data od)
	{
		char buf[44] = "SUP"; // Meaningless magic bytes
		crypto::hash result;
		memcpy(buf + 4, reinterpret_cast<const void *>(&pubkey), sizeof(pubkey));
		memcpy(buf + 4 + sizeof(pubkey), reinterpret_cast<const void *>(&timestamp), sizeof(timestamp));
		memcpy(buf + 4 + sizeof(timestamp), reinterpret_cast<const void *>(&tu.price), sizeof(od.price));
		memcpy(buf + 4 + sizeof(tu.price), reinterpret_cast<const void *>(&tu.taskHash), sizeof(tu.task_hash));
		crypto::cn_fast_hash(buf, sizeof(buf), result);

		return result;
	}

	static crypto::hash make_hash(crypto::public_key const &pubkey, uint64_t timestamp)
	{
		char buf[44] = "SUP"; // Meaningless magic bytes
		crypto::hash result;
		memcpy(buf + 4, reinterpret_cast<const void *>(&pubkey), sizeof(pubkey));
		memcpy(buf + 4 + sizeof(pubkey), reinterpret_cast<const void *>(&timestamp), sizeof(timestamp));
		crypto::cn_fast_hash(buf, sizeof(buf), result);

		return result;
	}

    Delfi::Delfi(cryptonote::core& core) : m_core(core){
        if(!scanTasks()){
            LOG_PRINT_L1("Unable to scan tasks!");
        }
    }

    bool Delfi::scanTasks(){}   

    uint64_t Delfi::getTaskLastHeight(const crypto::hash& task_hash){}

    std::vector<task> Delfi::getTasks(){
        return m_tasks;
    }

    uint64_t task::completeTask()
    {
        for (std::string exchange :: exchanges)
        {
            if(exchange == "Coinbase")
            {
                return price_provider::getCoinbasePrice(task.pair);
            }
            else if(exchange == "Kucoin")
            {
                return price_provider::getKucoinPrice(task.pair);

            }
            else if(exchange == "TradeOgre")
            {
                return price_provider::getTradeOgrePrice(task.pair);

            }
            else if(exchange == "Bittrex")
            {
                return price_provider::getBittrexPrice(task.pair);
            }
            else 
            {
                //this should never happen
                MERROR("Exchange give not supported");
                return 0;
            }
        }
    }

    std::tuple<uint64_t,uint64_t,uint64_t> Delfi::census_tasks(const crypto::public_key &my_pubkey, oracle_data od)
	{
        double stdev = 0;
        sizet_t N = 0;
        uint64_t m = 0;

        //Mean
        for (auto t1 : task_updates)
        {
            if(t1.taskHash != task_hash)
                continue;

            m += ti.od.price;
            N++;
        }

        m = m / N;

        //stdev
        for (auto t1 : task_updates)
        {
            if(t1.taskHash != task_hash)
                continue;

            double delta = t1.od.price - m;
            stdev += delta * delta;
        }

        stdev = stdev / N;

        return {stdev, N, m};

	}

    //Process Tasks for next block
	void Delfi::process_tasks(const crypto::public_key &my_pubkey, const crypto::secret_key &my_seckey)
	{
		delfi_protocol::task_update task_update;
		std::vector<delfi_protocol::task> tasks = m_delfi.getTasks();
		for (auto task : tasks)
		{
			delfi_protocol::oracle_data od;
			if (latest_height > task.block_height_finished)
			{
				MERROR("Task is expired");
				continue;
			}

			uint64_t last_task_height = m_delfi.getTaskLastHeight(task.taskHash);

			if(last_task_height + task.block_rate != latest_height)
			{
				MGINFO_GREEN("Task: " << task.taskHash << " at block height: " << latest_height << " does not need to be ran.");
				continue;
			}
	
			od.price = task.completeTask();
			task_update.taskHash = task.taskHash;

            //Generate Oracle Hash
			crypto::hash hash = make_hash(my_pubkey, time(nullptr), od);
			crypto::generate_signature(hash, my_pubkey, my_seckey, od.sig);

            od.oracle_hash = hash;

			//push task update to then be sent to all oracle nodes
			task_update.od.push_back(od);
		}

		if(task_update.od.size() == 0)
			return;

		cryptonote::NOTIFY_TASK_UPDATE::request req;
		generate_task_update(my_pubkey, my_seckey, task_updates, req);

		m_core.submit_task_update(req);
	}

    bool Delfi::handle_task_update(const cryptonote::NOTIFY_TASK_UPDATE::request &taskUpdate)
    {
        uint64_t now = time(nullptr);
			
		uint64_t timestamp = taskUpdate.timestamp;
		const crypto::public_key& pubkey = taskUpdate.pubkey;
		const crypto::signature& sig = taskUpdate.sig;


		if ((timestamp < now - UPTIME_PROOF_BUFFER_IN_SECONDS) || (timestamp > now + UPTIME_PROOF_BUFFER_IN_SECONDS))
			return false;

		if (!m_core.is_service_node(pubkey))
			return false;

		CRITICAL_REGION_LOCAL(m_lock);
		if (m_task_update_seen[pubkey] >= now - (UPTIME_PROOF_FREQUENCY_IN_SECONDS / 2))
			return false; // already received one uptime proof for this node recently.

		crypto::hash hash = make_hash(pubkey, timestamp);
		if (!crypto::check_signature(hash, pubkey, sig))
			return false;

		m_task_update_seen[pubkey] = now;
		return true;
    }

    std::vector<delfi_protocol::task_update> quorum_cop::get_task_updates(const crypto::public_key &pubkey)
	{
		CRITICAL_REGION_LOCAL(m_lock);

		const auto& it = m_ribbon_data_received.find(pair_hash);
		if (it != m_ribbon_data_received.end())
		{
			return m_valid_tasks[pubkey];
		}

		return {};
    }

    void Delfi::generate_task_update(const crypto::public_key& pubkey, const crypto::secret_key& seckey, std::vector<delfi_protocol::task_update>& task_updates, cryptonote::NOTIFY_TASK_UPDATE::request& req)
	{
		req.task_updates = task_updates;
		req.timestamp = time(nullptr);
		req.pubkey = pubkey;

		crypto::hash hash = make_hash(req.pubkey, req.timestamp);
	}


}