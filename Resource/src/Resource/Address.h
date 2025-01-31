#pragma once
#include <string>
#include <unordered_map>
#include "FromJson.h"
#include "ToJson.h"
namespace Aether::Resource
{
    class Address
    {
    public:
        Address(const std::string_view address)
            :m_Address(address)
        {
        }
        Address(const Address&)=default;
        Address(Address&&)=default;
        Address& operator=(const Address&)=default;
        Address& operator=(Address&&)=default;
        const std::string& GetAddress() const
        {
            return m_Address;
        }
        std::string& GetAddress()
        {
            return m_Address;
        }
        void SetAddress(const std::string_view address)
        {
            m_Address=address;
        }
    private:
        std::string m_Address;
    };
    struct AddressHash
    {
        std::size_t operator()(const Address& address) const
        {
            return std::hash<std::string>{}(address.GetAddress());
        }
    };
    template<>
    struct ToJsonImpl<Address>
    {
        Json operator()(const Address& t)
        {
            return t.GetAddress();
        }
    };
    template<>
    struct FromJsonImpl<Address>
    {
        std::expected<Address,std::string> operator()(const Json& json)
        {
            return Address(json.get<std::string>());
        }
    };
}