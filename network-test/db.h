#ifndef _DB_H_
#define _DB_H_

#include <PalmOS.h>

class DB {
   public:
    struct Address {
        char ip[16];
        UInt16 port;
    };

   public:
    static DB& Get() {
        static DB instance;
        return instance;
    }

    bool Initialize();

    void GetDestinationReceivePB(Address& target);
    void SetDestinationReceivePB(const Address& address);

    void GetDestinationSendPB(Address& target);
    void SetDestinationSendPB(const Address& address);

   private:
    DmOpenRef ref;

   private:
    DB() : ref(0) {}

    void GetAddress(UInt16 idx, Address& target);
    void SetAddress(UInt16 idx, const Address& address);

   private:
    DB(const DB&);
    DB& operator=(const DB&);
};

#endif  // _DB_H_
