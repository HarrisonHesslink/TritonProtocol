#include "service_node_deregister.h"
#include "service_node_list.h"
#include "cryptonote_config.h"
#include "cryptonote_core.h"
#include "version.h"
#include "quorum_cop.h"

#undef MONERO_DEFAULT_LOG_CATEGORY
#define MONERO_DEFAULT_LOG_CATEGORY "quorum_cop"

namespace service_nodes
{

    eon_manager(cryptonote::core& core)
    {
        m_core = core;
    }

    bool eon_manager::handle_new_round(uint64_t round_id, uint64_t block_height, std::string feed_address, std::vector<std::string> publishers, std::vector<std::string> raters)
    {
        service_nodes::new_round round;
        round.round_id = round_id;
        round.feed_address = feed_address;
        round.publishers = publishers;
        round.raters = raters;

        //check publishers and raters

        if(m_round_answers[feed_address][round_id] == nullptr)
            m_round_answers[feed_address][round_id] = round;
            m_core.relay_new_round(round);
    }

    static crypto::hash make_hash(crypto::public_key const &pubkey, uint64_t& round_id, uint64_t& block_height, std::string& feed_address, std::string& answer)
	{
		char buf[44] = "SUP"; // Meaningless magic bytes
		crypto::hash result;
		memcpy(buf + 4, reinterpret_cast<const void *>(&pubkey), sizeof(pubkey));
		memcpy(buf + 4 + sizeof(pubkey), reinterpret_cast<const void *>(&round_id), sizeof(round_id));
        memcpy(buf + 4 + sizeof(round_id), reinterpret_cast<const void *>(&block_height), sizeof(block_height));
        memcpy(buf + 4 + sizeof(block_height), reinterpret_cast<const void *>(&feed_address), sizeof(feed_address));
        memcpy(buf + 4 + sizeof(feed_address), reinterpret_cast<const void *>(&answer), sizeof(answer));
		crypto::cn_fast_hash(buf, sizeof(buf), result);
		return result;
	}
    
    bool eon_manager::handle_new_answer(service_nodes::answer& ans, uint64_t& block_height) {

        //check if valid node

        //check if round 
        if(m_round_answers[ans.feed_address][ans.round_id] == nullptr)
            return false;

        service_nodes::new_round r = m_round_answers[ans.feed_address][ans.round_id];

        //round is closed but shouldn't happen
        if(block_height >= r.height + 100)
            return false;

        if(m_round_answers[feed_address][round_id][ans.node_address] == nullptr)
            m_round_answers[feed_address][round_id][ans.node_address] = ans;

        return true;
    }

    service_nodess::answer eon_manager::handle_my_new_answer(uint64_t round_id, uint64_t block_height, std::string feed_address, std::string answer, std:string eth_hash, std::string eth_sig)
    {
		crypto::public_key my_pubkey;
		crypto::secret_key my_seckey;
		if (!m_core.get_service_node_keys(my_pubkey, my_seckey))
			return;

        crypto::hash xeq_hash = make_hash(my_pubkey, round_id, block_height, feed_address, block_height, answer);
        crypto::signature xeq_sig;
        crypto::generate_signature(hash, my_pubkey, my_seckey, xeq_sig);

        service_nodes::answer this_answer;
        this_answer.round_id = round_id;
        this_answer.block_height = block_height;
        this_answer.feed_address = feed_address;
        this_answer.answer = answer;
        this_answer.node_address = my_pubkey;
        this_answer.xeq_hash = epee::string_tools::pod_to_hex(xeq_hash);
        this_answer.eth_hash = eth_hash;
        this_answer.xeq_sig = epee::string_tools::pod_to_hex(xeq_sig);
        this_answer.eth_sig = eth_sig;

        if(m_round_answers[feed_address][round_id][my_pubkey] == nullptr)
            m_round_answers[feed_address][round_id][my_pubkey] = answer;
            m_core.relay_my_eon_answer(answer);

        return true;
    }

}