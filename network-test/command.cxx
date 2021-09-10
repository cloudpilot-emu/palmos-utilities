#include "command.h"

#include <PalmOS.h>

#include "debug.h"
#include "field.h"

static const char* REMOTE_IP = "127.0.0.1";
static const UInt16 REMOTE_PORT = 6666;

void TestSocketOptionGet() { Printf("Testing NetLibSocketOptionGet...\n\n"); }

void TestReceivePB() { Printf("Testing NetLibReceivePB...\n\n"); }

void TestSendPB() {
    NetSocketRef socket = -1;
    Err err;
    static const char* words[] = {"one ", "two ", "three "};
    const UInt8 wordCount = 5;

    Printf("Testing NetLibSendPB...\n\n");

    Printf("opening netlib...\n");

    UInt16 refNum;
    if (SysLibFind("Net.lib", &refNum) != 0) {
        Printf("Net.lib not found\n");
        return;
    }

    UInt16 ifErr;
    if (NetLibOpen(refNum, &ifErr) != 0) {
        Printf("failed to open netlib\n");
        return;
    }

    Printf("connecting to %s:%u\n", REMOTE_IP, REMOTE_PORT);

    socket = NetLibSocketOpen(refNum, netSocketAddrINET, netSocketTypeStream, netSocketProtoIPTCP,
                              -1, &err);
    if (err != 0) {
        Printf("failed to open socket\n");
        goto cleanup;
    }

    NetSocketAddrINType addr;
    addr.family = netSocketAddrINET;
    addr.addr = NetLibAddrAToIN(refNum, REMOTE_IP);
    addr.port = REMOTE_PORT;
    if (NetLibSocketConnect(refNum, socket, reinterpret_cast<NetSocketAddrType*>(&addr),
                            sizeof(addr), -1, &err) != 0) {
        Printf("failed to connect\n");
        goto cleanup;
    }

    NetIOVecType chunks[wordCount + 1];
    for (int i = 0; i < wordCount; i++) {
        UInt8 word = SysRandom(0) % 3;

        chunks[i].bufP = (UInt8*)(words[word]);
        chunks[i].bufLen = StrLen(words[word]) + 1;
    }

    chunks[wordCount].bufP = (UInt8*)"\n";
    chunks[wordCount].bufLen = 1;

    NetIOParamType params;
    params.addrP = 0;
    params.addrLen = 0;
    params.iov = chunks;
    params.iovLen = wordCount + 1;
    params.accessRights = 0;
    params.accessRightsLen = 0;

    Int16 bytesSent;
    bytesSent = NetLibSendPB(refNum, socket, &params, 0, -1, &err);
    if (bytesSent > 0)
        Printf("successfully sent %i bytes\n", bytesSent);
    else
        Printf("failed to send\n");

cleanup:
    if (socket > 0) NetLibSocketClose(refNum, socket, -1, &err);
    NetLibClose(refNum, true);

    Printf("\n");
}

void TestDmSend() { Printf("Testing NetLibDmSend...\n\n"); }
