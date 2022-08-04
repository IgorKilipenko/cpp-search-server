#include "transport_catalogue.h"
namespace transport_catalogue {
    const std::deque<Stop>& TransportCatalogue::GetStops() const {
        return db_->GetStopsTable();
    }

    const std::shared_ptr<TransportCatalogue::Database> TransportCatalogue::GetDatabaseForWrite() const {
        return db_;
    }

    const std::shared_ptr<const TransportCatalogue::Database> TransportCatalogue::GetDatabaseReadOnly() const {
        return db_;
    }

    const BusInfo TransportCatalogue::GetBusInfo(const Bus* bus) const {
        return db_->GetBusInfo(bus);
    }
}