#include "int-util.h"
#include "rpc/core_rpc_server_commands_defs.h"

namespace delfi_protocol {
    struct task 
    {
        crypto::hash taskHash;
        std::pair<std::string, std::string> pair;
        std::vector<std::string> exchanges;

        crypto::hash reg_tx_hash;

        uint64_t block_rate;
        uint64_t block_height_finished;

        uint64_t completeTask();
    };

    struct task_update 
    {
        crypto::hash taskUpdateHash;
        crypto::hash taskHash;
        uint64_t price;

        crypto::public_key pubkey;
        crypto::signature sig;
    }

    class Delfi {

        public:
            Delfi();
            std::vector<task> getTasks();
            uint64_t getTaskLastHeight(const crypto::hash& task_hash);
        private:
            bool scanTasks();
            bool startTasks();
            std::queue<task> m_tasks;


    };

    uint64_t getCoinbasePrice(std::pair<std::string, std::string> pair /*  BTCUSD  */);
    uint64_t getTradeOgrePrice(std::pair<std::string, std::string> pair /*  BTC-XEQ  */);
    uint64_t getBittrexPrice(std::pair<std::string, std::string> pair /*  XHV-BTC  */);
    uint64_t getNancePrice(std::pair<std::string, std::string> pair /*  BTCUSDT  */);
    uint64_t getKucoinPrice(std::pair<std::string, std::string> pair /*  BTC-XEQ  */);
}