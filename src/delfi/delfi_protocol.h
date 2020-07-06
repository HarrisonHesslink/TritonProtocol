#include "int-util.h"
#include "rpc/core_rpc_server_commands_defs.h"

namespace delfi_protocol {
    class Delfi {

        struct task 
        {
            crypto::hash taskHash;
            std::pair<std::string, std::string> pair;
            std::vector<std::string> exchanges;

            crypto::hash reg_tx_hash;

            uint64_t block_rate;
            uint64_t blocks_til_finished;
        };

        public:
            Delfi();

            std::queue<Delfi::task> m_tasks;

        private:
            bool scanTasks();
            bool startTasks();

    };

    uint64_t getCoinbasePrice(std::pair<std::string, std::string> pair /*  BTCUSD  */);
    uint64_t getTradeOgrePrice(std::pair<std::string, std::string> pair /*  BTC-XEQ  */);
    uint64_t getBittrexPrice(std::pair<std::string, std::string> pair /*  XHV-BTC  */);
    uint64_t getNancePrice(std::pair<std::string, std::string> pair /*  BTCUSDT  */);
    uint64_t getKucoinPrice(std::pair<std::string, std::string> pair /*  BTC-XEQ  */);
}