#pragma once

#include "blockchain.h"
#include "cryptonote_protocol/cryptonote_protocol_handler_common.h"


namespace cryptonote
{
	class core;
};

namespace service_nodes
{

    struct answer{
        uint64_t round_id;
        uint64_t block_height

        std::string feed_address;

        std::string answer;
        std::string xeq_hash;

        crypto::public_key node_address;
        std::string eth_hash;

        std::string xeq_sig;
        std::string eth_sig;

        bool isSigned(){return xeq_sig != null};
    };


    struct new_round{
        uint64_t round_id;
        std::string feed_address;
        std::vector<std::string> publishers;
        std::vector<std::string> raters;

        crypto::public_key target_winner;
        uint64_t height;
    };

    struct prepare_answer {
        crypto::public_key target_winner;
        uint16_t round_id;
        uint64_t block_height;
        std::string feed_address;

        std::vector<answer> answers;
        uint64_t height_winner;
        std::string xeq_hash;
        std::string eth_hash;
    }

    class eon_manager {
        public: 
            explicit eon_manager(cryptonote::core& core);

            bool handle_new_round(uint64_t round_id, uint64_t block_height, std::string feed_address, std::vector<std::string> publishers, std::vector<std::string> raters);
            bool handle_my_new_answer(uint64_t round_id, uint64_t block_height, std::string feed_address, std::string answer, std:string eth_hash, std::string eth_sig);
            bool handle_new_answer(service_nodes::answer& ans, uint64_t& block_height);
            // bool handle_chain_transaction_reporting();

            service_nodes::answer generate_answer();
            service_nodes::prepare_answer prepare_flow_answer();

        private:
            cryptonote::core& m_core;

            //feed_address->round_id->struct
            std::unordered_map<std::string, std::unordered_map<uint64_t, new_round>> m_new_rounds;
            //feed_address->round_id->node address->answer
            std::unordered_map<std::string, std::unordered_map<uint64_t, std::unordered_map<crypto::public_key, answer>>> m_round_answers;
            //feed_address->round_id->node address->answer
            std::unordered_map<std::string, std::unordered_map<uint64_t, std::unordered_map<crypto::public_key, answer>>> m_round_signed_answers;
    }


}