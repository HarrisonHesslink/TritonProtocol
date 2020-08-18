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
#include <queue>
#include "int-util.h"
#include "cryptonote_core/cryptonote_core.h"
#include "cryptonote_protocol/cryptonote_protocol_handler.h"
#include "rpc/core_rpc_server_commands_defs.h"

namespace delfi_protocol {

    /*
    
    Consensus PAXOS-DELFI

    Client - Users sending in a task

    Voters - Oracle Nodes

    Proposer - Random 1 Oracle Node

    Learner - All nodes

    Leader - Oracle Node chosen to get reward + make delfi block

    Client (task maker) want to receive XEQ price x 100 blocks -->
    Voters  
    
    
    */
    struct task 
    {
        crypto::hash task_hash;
        std::pair<std::string, std::string> pair;
        std::vector<std::string> exchanges;

        crypto::hash reg_tx_hash;

        uint64_t block_rate;
        uint64_t block_height_finished;

        uint64_t completeTask();
    };


    /*
    Struct Oracle Data contains a nodes data for each task. 
    */

    struct oracle_data 
    {
        crypto::hash oracle_hash;


        crypto::hash task_hash;
        uint64_t price;

        crypto::signature sig;
    };

    /*
    Struct Task Update contains the full list of tasks a node has completed.

    Task->Oracle Data->Task Update

    */
    struct task_update 
    {
        crypto::hash task_update_hash;
        std::vector<oracle_data> od;

        crypto::public_key pubkey;
    };

    struct ballot 
    {
        std::vector<task_update> valid_proposals;

        crypto::signature sig;
        crypto::public_key pubkey;
    };



    class Delfi {
		explicit Delfi(cryptonote::core& core);

        public:
            std::vector<task> getTasks();
            uint64_t getTaskLastHeight(const crypto::hash& task_hash);
            std::vector<delfi_protocol::task_update> get_task_updates(const crypto::public_key &pubkey);

        private:
            void census_tasks(const crypto::public_key &my_pubkey, oracle_data od);
            void process_tasks(const crypto::public_key &my_pubkey, const crypto::secret_key &my_seckey, uint64_t height);
            bool handle_task_update(const cryptonote::NOTIFY_TASK_UPDATE::request &task_update);

            bool scanTasks();
            bool startTasks();


		    cryptonote::core& m_core;
            std::queue<task> m_tasks;
		    std::unordered_map<crypto::public_key, std::vector<delfi_protocol::task_update>> m_valid_tasks;
		    std::unordered_map<crypto::public_key, uint64_t> m_task_update_seen;
    };
    void generate_task_update(const crypto::public_key& pubkey, const crypto::secret_key& seckey, std::vector<delfi_protocol::task_update>& task_updates, cryptonote::NOTIFY_TASK_UPDATE::request& req);

}