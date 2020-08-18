// Copyright (c)      2018, The Loki Project
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

#include "oracle_node_paxos.h"
#include "service_node_list.h"
#include "cryptonote_basic/tx_extra.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "cryptonote_basic/verification_context.h"
#include "cryptonote_basic/connection_context.h"
#include "cryptonote_protocol/cryptonote_protocol_defs.h"
#include "service_node_list.h"
#include "cryptonote_core/blockchain.h"

#include "misc_log_ex.h"
#include "string_tools.h"

#include <random>
#include <string>
#include <vector>

#undef MONERO_DEFAULT_LOG_CATEGORY
#define MONERO_DEFAULT_LOG_CATEGORY "service_nodes"


namespace oracle_node_paxos
{
	static crypto::hash make_hash_from(uint64_t block_height, uint32_t service_node_index)
	{
		const int buf_size = sizeof(block_height) + sizeof(service_node_index);
		char buf[buf_size];

		memcpy(buf, reinterpret_cast<void *>(&block_height), sizeof(block_height));
		memcpy(buf + sizeof(block_height), reinterpret_cast<void *>(&service_node_index), sizeof(service_node_index));

		crypto::hash result;
		crypto::cn_fast_hash(buf, buf_size, result);

		return result;
	}

	crypto::signature oracle_node_paxos::sign_vote(uint64_t block_height, uint32_t service_node_index, const crypto::public_key& pub, const crypto::secret_key& sec)
	{
		crypto::signature result;
		crypto::generate_signature(make_hash_from(block_height, service_node_index), pub, sec, result);
		return result;
	}

	bool oracle_node_paxos::verify_vote_signature(uint64_t block_height, uint32_t service_node_index, crypto::public_key p, crypto::signature s)
	{
		std::vector<std::pair<crypto::public_key, crypto::signature>> keys_and_sigs{ std::make_pair(p, s) };
		return verify_votes_signature(block_height, service_node_index, keys_and_sigs);
	}

	bool oracle_node_paxos::verify_votes_signature(uint64_t block_height, uint32_t service_node_index, const std::vector<std::pair<crypto::public_key, crypto::signature>>& keys_and_sigs)
	{
		crypto::hash hash = make_hash_from(block_height, service_node_index);
		for (auto& key_and_sig : keys_and_sigs)
		{
			if (!crypto::check_signature(hash, key_and_sig.first, key_and_sig.second))
			{
				return false;
			}
		}

		return true;
	}

	static bool verify_votes_helper(cryptonote::network_type nettype, const cryptonote::tx_extra_oracle_node_proposer& proposer,
		cryptonote::vote_verification_context &vvc,
		const service_nodes::quorum_state &quorum_state)
	{
		if (proposer.service_node_index >= quorum_state.nodes_to_test.size())
		{
			vvc.m_service_node_index_out_of_bounds = true;
			LOG_PRINT_L1("Service node index in proposer vote was out of bounds: " << proposer.service_node_index << ", expected to be in range of: [0, " << quorum_state.nodes_to_test.size() << ")");
			return false;
		}

		const std::vector<crypto::public_key>& quorum = quorum_state.quorum_nodes;
		std::vector<int8_t> quorum_set;

		std::vector<std::pair<crypto::public_key, crypto::signature>> keys_and_sigs;
		for (const cryptonote::oracle_node_paxos::vote& vote : proposer.votes)
		{
			if (vote.voters_quorum_index >= quorum.size())
			{
				vvc.m_voters_quorum_index_out_of_bounds = true;
				LOG_PRINT_L1("Voter's index in proposer vote was out of bounds: " << vote.voters_quorum_index << ", expected to be in range of: [0, " << quorum.size() << ")");
				return false;
			}

			quorum_set.resize(quorum.size());
			if (++quorum_set[vote.voters_quorum_index] > 1)
			{
				vvc.m_duplicate_voters = true;
				LOG_PRINT_L1("Voter quorum index is duplicated: " << vote.voters_quorum_index << ", expected to be in range of: [0, " << quorum.size() << ")");
				return false;
			}

			keys_and_sigs.push_back(std::make_pair(quorum[vote.voters_quorum_index], vote.signature));
		}

		bool r = oracle_node_proposer::verify_votes_signature(proposer.block_height, proposer.service_node_index, keys_and_sigs);
		if (!r)
		{
			LOG_PRINT_L1("Invalid signatures for votes: ");
			vvc.m_verification_failed = true;
		}

		return r;
	}


	bool oracle_node_paxos::verify_proposer(cryptonote::network_type nettype, const cryptonote::tx_extra_oracle_node_proposer& proposer,
		cryptonote::vote_verification_context &vvc,
		const service_nodes::quorum_state &quorum_state)
	{
		if (proposer.votes.size() < service_nodes::MIN_VOTES_TO_WIN)
		{
			LOG_PRINT_L1("Not enough votes");
			vvc.m_not_enough_votes = true;
			return false;
		}

		bool result = verify_votes_helper(nettype, proposer, vvc, quorum_state);
		return result;
	}


	bool oracle_node_paxos::verify_vote(cryptonote::network_type nettype, const vote& v, cryptonote::vote_verification_context &vvc,
		const service_nodes::quorum_state &quorum_state)
	{
		cryptonote::tx_extra_delfi_marker proposer;
		proposer.block_height = v.block_height;
		proposer.service_node_index = v.service_node_index;
		proposer.votes.push_back(cryptonote::tx_extra_oracle_node_proposer::vote{ v.signature, v.voters_quorum_index });
		return verify_votes_helper(nettype, proposer, vvc, quorum_state);
	}

	void oracle_node_paxos::set_relayed(const std::vector<oracle_node_proposer::vote>& votes)
	{
		CRITICAL_REGION_LOCAL(m_lock);
		const time_t now = time(NULL);

		for (const oracle_node_proposer::vote &find_vote : votes)
		{
			proposer_group desired_group = {};
			proposer_group.block_height = find_vote.block_height;
			proposer_group.service_node_index = find_vote.service_node_index;

			auto proposer_entry = m_proposers.find(desired_group);
			if (proposer_entry != m_proposers.end())
			{
				std::vector<proposer> &proposer_vector = proposer_entry->second;
				for (auto &proposer : proposer_vector)
				{
					if (proposer.m_vote.voters_quorum_index == find_vote.voters_quorum_index)
					{
						proposer.m_time_last_sent_p2p = now;
						break;
					}
				}
			}
		}
	}

	std::vector<oracle_node_paxos::vote> oracle_node_vote_pool::get_relayable_votes() const
	{
		CRITICAL_REGION_LOCAL(m_lock);
		const cryptonote::cryptonote_connection_context fake_context = AUTO_VAL_INIT(fake_context);

		// TODO(doyle): Rate-limiting: A better threshold value that follows suite with transaction relay time back-off
		const time_t now = time(NULL);
		const time_t THRESHOLD = 60 * 2;

		std::vector<oracle_node_proposer::vote> result;
		for (const auto &proposer_entry : m_proposers)
		{
			const std::vector<proposer>& proposer_vector = proposer_entry.second;
			for (const proposer &entry : proposer_vector)
			{
				const time_t last_sent = now - entry.m_time_last_sent_p2p;
				if (last_sent > THRESHOLD)
				{
					result.push_back(entry.m_vote);
				}
			}
		}
		return result;
	}

	bool oracle_node_vote_pool::add_vote(const oracle_node_paxos::vote& new_vote,
		cryptonote::vote_verification_context& vvc,
		const service_nodes::quorum_state &quorum_state,
		cryptonote::transaction &tx)
	{
		if (!oracle_node_paxos::verify_vote(m_nettype, new_vote, vvc, quorum_state))
		{
			LOG_PRINT_L1("Signature verification failed for proposer vote");
			return false;
		}

		CRITICAL_REGION_LOCAL(m_lock);
		time_t const now = time(NULL);
		std::vector<proposer> *proposer_votes;
		{
			proposer_group desired_group = {};
			proposer_group.block_height = new_vote.block_height;
			proposer_group.service_node_index = new_vote.service_node_index;
			proposer_votes = &m_proposers[desired_group];
		}

		bool new_proposer_is_unique = true;
		for (const auto &entry : *proposer_votes)
		{
			if (entry.m_vote.voters_quorum_index == new_vote.voters_quorum_index)
			{
				new_proposer_is_unique = false;
				break;
			}
		}

		if (new_proposer_is_unique)
		{
			vvc.m_added_to_pool = true;
			proposer_votes->emplace_back(proposer(0 /*time_last_sent_p2p*/, new_vote));

			if (proposer_votes->size() >= service_nodes::MIN_VOTES_TO_WIN)
			{
				cryptonote::tx_extra_oracle_node_proposer proposer;
				proposer.block_height = new_vote.block_height;
				proposer.service_node_index = new_vote.service_node_index;
				proposer.votes.reserve(proposer_votes->size());

				for (const auto& entry : *proposer_votes)
				{
					cryptonote::tx_extra_oracle_node_proposer::vote tx_vote = {};
					tx_vote.signature = entry.m_vote.signature;
					tx_vote.voters_quorum_index = entry.m_vote.voters_quorum_index;
					proposer.votes.push_back(tx_vote);
				}

				vvc.m_full_tx_proposer_made = cryptonote::add_oracle_node_proposer_to_tx_extra(tx.extra, proposer);
				if (vvc.m_full_tx_proposer_made)
				{
					tx.version = cryptonote::transaction::version_4_delfi_marker
					tx.is_delfi_marker = true;
				}
				else
				{
					LOG_PRINT_L1("Could not create version 4 delfi marker transaction from votes");
				}
			}
		}

		return true;
	}

	void oracle_node_vote_pool::remove_used_votes(std::vector<std::pair<cryptonote::transaction, cryptonote::blobdata>> const &txs)
	{
		CRITICAL_REGION_LOCAL(m_lock);
		for (const std::pair<cryptonote::transaction, cryptonote::blobdata> &tx : txs)
		{
			if (!tx.first.is_delfi_marker())
				continue;

			cryptonote::tx_extra_oracle_node_proposer proposer;
			if (!get_service_node_proposer_from_tx_extra(tx.first.extra, proposer))
			{
				LOG_ERROR("Could not get proposer from tx version 4, possibly corrupt tx");
				continue;
			}

			proposer_group desired_group = {};
			proposer_group.block_height = proposer.block_height;
			proposer_group.service_node_index = proposer.service_node_index;
			m_proposers.erase(desired_group);
		}
	}

	void oracle_node_vote_pool::remove_expired_votes(uint64_t height)
	{
		if (height < oracle_node_paxos::VOTE_LIFETIME_BY_HEIGHT)
		{
			return;
		}

		CRITICAL_REGION_LOCAL(m_lock);
		uint64_t minimum_height = height - oracle_node_paxos::VOTE_LIFETIME_BY_HEIGHT;
		for (auto it = m_proposers.begin(); it != m_proposers.end();)
		{
			const proposer_group &proposer_for = it->first;
			if (proposer_for.block_height < minimum_height)
			{
				it = m_proposers.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
}; // namespace triton
