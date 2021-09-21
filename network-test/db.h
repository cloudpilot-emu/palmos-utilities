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
    DB() : ref(0) {}

    bool Initialize();

    void GetDestinationReceivePB(Address& target);
    void SetDestinationReceivePB(const Address& address);

    void GetDestinationSendPB(Address& target);
    void SetDestinationSendPB(const Address& address);

   private:
    DmOpenRef ref;

   private:
    void GetAddress(UInt16 idx, Address& target);
    void SetAddress(UInt16 idx, const Address& address);

   private:
    DB(const DB&);
    DB& operator=(const DB&);
};

extern DB db;

#endif  // _DB_H_
