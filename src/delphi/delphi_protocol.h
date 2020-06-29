#include "int-util.h"
#include "rpc/core_rpc_server_commands_defs.h"

namespace delphi_protocol {
    class Delphi {

        struct task {
            std::pair<std::string, std::string> pair;
            std::vector<std::string> exchanges;

            cryptonote::transaction registrationTX;

            uint64_t block_rate;
            uint64_t blocks_til_finished;
        };

        public:
            Delphi();

            std::queue<Delphi::task> m_tasks;

        private:
            bool scanTasks();
            bool startTasks();

    };

    bool getCoinbasePrice(uint64_t& price, std::pair<std::string, std::string> pair /*  BTCUSD  */);
    bool getTradeOgrePrice(uint64_t& price, std::pair<std::string, std::string> pair /*  BTC-XEQ  */);
    bool getBittrexPrice(uint64_t& price, std::pair<std::string, std::string> pair /*  XHV-BTC  */);
    bool getNancePrice(uint64_t& price, std::pair<std::string, std::string> pair /*  BTCUSDT  */);
    bool getKucoinPrice(uint64_t& price, std::pair<std::string, std::string> pair /*  BTC-XEQ  */);
}