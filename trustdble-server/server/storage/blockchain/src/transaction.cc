#include "blockchain/transaction.h"

using namespace trustdble;

auto Transaction::init() -> int{
    return 0;
}
auto Transaction::addTable(const std::string &tablename, std::map<BYTES, BYTES> &table_map) -> int{
    auto [it, result] = table_cache.emplace(tablename, std::move(table_map));
    return result ? 0 : 1;
}
auto Transaction::addWrite(const std::string &tablename, BYTES &key, BYTES &value) -> int{
    if(tablename.empty() || key.size==0)
        return 1;
    STATEMENT statement = {STATEMENT_TYPE::WRITE, tablename, key, value};
    statements.push_back(statement);
    return 0;
}
auto Transaction::addRemove(const std::string &tablename,const BYTES &key) -> int{
    if(tablename.empty() || key.size==0)
        return 1;
    STATEMENT statement = {STATEMENT_TYPE::REMOVE, tablename, key, BYTES(nullptr,0)};
    statements.push_back(statement);
    return 0;
}
