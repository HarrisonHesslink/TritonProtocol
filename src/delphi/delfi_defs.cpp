#include "serialization/keyvalue_serialization.h"
#include "storages/portable_storage.h"
#include "delphi_defs.h"
namespace delphi_protocol {
    bool tradeogre_def_serialized::_load(epee::serialization::portable_storage& src, epee::serialization::section* hparent)
    {
        tradeogre_def_serialized in{};
        if(in._load(src,hparent))
        {
            success = in.success;
            initialprice = in.initialprice;
            price = in.price;
            high = in.high;
            low = in.low;
            volume = in.volume;
            bid = in.bid;
            ask = in.ask;
            return true;
        }

        return false;
    }

    bool pricing_record::store(epee::serialization::portable_storage& dest, epee::serialization::section* hparent) const
    {
        const pr_serialized out{success,initialprice,price,high,low,volume,bid,ask};
        return out.store(dest, hparent);
    }
}
}