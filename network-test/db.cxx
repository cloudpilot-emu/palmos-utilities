#define DB_NAME "NetTest config"
#define DB_CREATOR 'NTST'
#define DB_TYPE 'CONF'

#define IDX_DESTINATION_RECEIVE_PB 0
#define IDX_DESTINATION_SEND_PB 1

#include "db.h"

#include "debug.h"

DB db;

bool DB::Initialize() {
    LOG("init DB");

    Err createErr = DmCreateDatabase(0, DB_NAME, DB_CREATOR, DB_TYPE, false);
    if (createErr && createErr != dmErrAlreadyExists) return false;

    ref = DmOpenDatabaseByTypeCreator(DB_TYPE, DB_CREATOR, dmModeReadWrite);
    if (!ref) return false;

    if (createErr) {
        LOG("DB already created");
        return true;
    }

    LOG("DB created");

    UInt16 idx = IDX_DESTINATION_RECEIVE_PB;
    DmNewRecord(ref, &idx, sizeof(Address));
    DmReleaseRecord(ref, idx, false);

    idx = IDX_DESTINATION_SEND_PB;
    DmNewRecord(ref, &idx, sizeof(Address));
    DmReleaseRecord(ref, idx, false);

    Address defaultAddress = {"127.0.0.1", 6666};

    SetDestinationReceivePB(defaultAddress);
    SetDestinationSendPB(defaultAddress);

    return true;
}

void DB::GetDestinationReceivePB(Address& target) {
    GetAddress(IDX_DESTINATION_RECEIVE_PB, target);
}

void DB::SetDestinationReceivePB(const Address& address) {
    SetAddress(IDX_DESTINATION_RECEIVE_PB, address);
}

void DB::GetDestinationSendPB(Address& target) { GetAddress(IDX_DESTINATION_SEND_PB, target); }

void DB::SetDestinationSendPB(const Address& address) {
    SetAddress(IDX_DESTINATION_SEND_PB, address);
}

void DB::GetAddress(UInt16 idx, Address& target) {
    MemHandle handle = DmQueryRecord(ref, idx);
    if (!handle) return;

    MemPtr ptr = MemHandleLock(handle);
    target = *static_cast<Address*>(ptr);
    MemHandleUnlock(handle);
}

void DB::SetAddress(UInt16 idx, const Address& address) {
    MemHandle handle = DmGetRecord(ref, idx);
    if (!handle) {
        LOG("unable to open record %u", idx);
        return;
    }

    MemPtr ptr = MemHandleLock(handle);
    DmWrite(ptr, 0, &address, sizeof(Address));
    MemHandleUnlock(handle);

    DmReleaseRecord(ref, idx, 0);
}
