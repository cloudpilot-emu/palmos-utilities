#include "command.h"

#include <PalmOS.h>

#include "db.h"
#include "debug.h"
#include "field.h"

void TestSocketOptionGet() {
#define ASSERT(message, expected, actual)                                      \
    if (expected != actual) {                                                  \
        Printf("assertion failed: " message ": %u != %u\n", expected, actual); \
        goto cleanup;                                                          \
    }

#define REFRESH_SOCKET()                                                                           \
    if (socket > 0) NetLibSocketClose(refNum, socket, -1, &err);                                   \
    socket = NetLibSocketOpen(refNum, netSocketAddrINET, netSocketTypeStream, netSocketProtoIPTCP, \
                              -1, &err);                                                           \
    if (err != 0) {                                                                                \
        Printf("failed to open socket\n");                                                         \
        goto cleanup;                                                                              \
    }

    NetSocketRef socket = -1;
    Err err;

    UInt8 u8val;
    UInt16 u16val;
    UInt32 u32val;
    UInt16 len;
    NetSocketLingerType linger;
    const char* ipOptions = "\x9f\x0c\x00\x01\x00\x00RTGAME";
    char optionsBuffer[40];

    Printf("Testing NetLibSocketOptionGet/Set...\n\n");
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

    socket = NetLibSocketOpen(refNum, netSocketAddrINET, netSocketTypeStream, netSocketProtoIPTCP,
                              -1, &err);
    if (err != 0) {
        Printf("failed to open socket\n");
        goto cleanup;
    }

    Printf("netSocketOptSockNonBlocking...\n");

    len = sizeof(u8val);
    NetLibSocketOptionGet(refNum, socket, netSocketOptLevelSocket, netSocketOptSockNonBlocking,
                          &u8val, &len, 0, &err);
    ASSERT("NetLibSocketOptionGet failed", 0, err);
    ASSERT("initial value", 0, u8val);

    u8val = 1;
    NetLibSocketOptionSet(refNum, socket, netSocketOptLevelSocket, netSocketOptSockNonBlocking,
                          &u8val, sizeof(u8val), 0, &err);
    ASSERT("NetLibSocketOptionSet failed", 0, err);

    len = sizeof(u8val);
    u8val = 0;
    NetLibSocketOptionGet(refNum, socket, netSocketOptLevelSocket, netSocketOptSockNonBlocking,
                          &u8val, &len, 0, &err);
    ASSERT("NetLibSocketOptionGet failed", 0, err);
    ASSERT("true (u8)", 1, u8val);

    u16val = 1;
    NetLibSocketOptionSet(refNum, socket, netSocketOptLevelSocket, netSocketOptSockNonBlocking,
                          &u16val, sizeof(u16val), 0, &err);
    ASSERT("NetLibSocketOptionSet failed", 0, err);

    len = sizeof(u16val);
    u16val = 0;
    NetLibSocketOptionGet(refNum, socket, netSocketOptLevelSocket, netSocketOptSockNonBlocking,
                          &u16val, &len, 0, &err);
    ASSERT("NetLibSocketOptionGet failed", 0, err);
    ASSERT("true (u16)", 1, u16val);

    u32val = 1;
    NetLibSocketOptionSet(refNum, socket, netSocketOptLevelSocket, netSocketOptSockNonBlocking,
                          &u32val, sizeof(u32val), 0, &err);
    ASSERT("NetLibSocketOptionSet failed", 0, err);

    len = sizeof(u32val);
    u32val = 0;
    NetLibSocketOptionGet(refNum, socket, netSocketOptLevelSocket, netSocketOptSockNonBlocking,
                          &u32val, &len, 0, &err);
    ASSERT("NetLibSocketOptionGet failed", 0, err);
    ASSERT("true (u32)", 1, u32val);

    Printf("netSocketOptTCPMaxSeg...\n");

    u16val = 256;
    NetLibSocketOptionSet(refNum, socket, netSocketOptLevelTCP, netSocketOptTCPMaxSeg, &u16val,
                          sizeof(u16val), 0, &err);
    ASSERT("NetLibSocketOptionSet failed", 0, err);

    len = sizeof(u16val);
    u16val = 0;
    NetLibSocketOptionGet(refNum, socket, netSocketOptLevelTCP, netSocketOptTCPMaxSeg, &u16val,
                          &len, 0, &err);
    ASSERT("NetLibSocketOptionGet failed", 0, err);
    ASSERT("256 (u16)", 256, u16val);

    REFRESH_SOCKET();

    u32val = 512;
    NetLibSocketOptionSet(refNum, socket, netSocketOptLevelTCP, netSocketOptTCPMaxSeg, &u32val,
                          sizeof(u32val), 0, &err);
    ASSERT("NetLibSocketOptionSet failed", 0, err);

    len = sizeof(u32val);
    u32val = 0;
    NetLibSocketOptionGet(refNum, socket, netSocketOptLevelTCP, netSocketOptTCPMaxSeg, &u32val,
                          &len, 0, &err);
    ASSERT("NetLibSocketOptionGet failed", 0, err);
    ASSERT("512 (u32)", 512, u32val);

    Printf("netSocketOptSockLinger...\n");

    linger.onOff = 1;
    linger.time = 10;
    NetLibSocketOptionSet(refNum, socket, netSocketOptLevelSocket, netSocketOptSockLinger, &linger,
                          sizeof(linger), 0, &err);
    ASSERT("NetLibSocketOptionSet failed", 0, err);

    len = sizeof(linger);
    linger.onOff = 0;
    linger.time = 0;
    NetLibSocketOptionGet(refNum, socket, netSocketOptLevelSocket, netSocketOptSockLinger, &linger,
                          &len, 0, &err);

    ASSERT("NetLibSocketOptionGet failed", 0, err);
    ASSERT("linger.onOff", 1, linger.onOff);
    ASSERT("linger.time", 10, linger.time);

    Printf("netSocketOptLevelIP...\n");

    NetLibSocketOptionSet(refNum, socket, netSocketOptLevelIP, netSocketOptIPOptions,
                          (void*)ipOptions, 12, 0, &err);
    ASSERT("NetLibSocketOptionGet failed", 0, err);

    len = 40;
    NetLibSocketOptionGet(refNum, socket, netSocketOptLevelIP, netSocketOptIPOptions,
                          &optionsBuffer, &len, 0, &err);
    ASSERT("NetLibSocketOptionGet failed", 0, err);
    if (len < 12 || MemCmp(ipOptions, optionsBuffer + len - 12, 12) != 0) {
        Printf("assertion failed: options mismatch");
        goto cleanup;
    }

    Printf("\n...all passed");

cleanup:
    if (socket > 0) NetLibSocketClose(refNum, socket, -1, &err);
    NetLibClose(refNum, true);

    Printf("\n");

#undef ASSERT
#undef REFRESH_SOCKET
}

void TestReceivePB() {
    NetSocketRef socket = -1;
    Err err;

    Printf("Testing NetLibReceivePB...\n\n");

    DB::Address destination;
    db.GetDestinationReceivePB(destination);

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

    NetIPAddr ipAddr = NetLibAddrAToIN(refNum, destination.ip);
    if (static_cast<Int32>(ipAddr) < 0) {
        Printf("invalid IP address %s\n", destination.ip);
        goto cleanup;
    }

    Printf("connecting to %s:%u\n", destination.ip, destination.port);

    socket = NetLibSocketOpen(refNum, netSocketAddrINET, netSocketTypeStream, netSocketProtoIPTCP,
                              -1, &err);
    if (err != 0) {
        Printf("failed to open socket: %i\n", err);
        goto cleanup;
    }

    NetSocketAddrINType addrConnect;
    addrConnect.family = netSocketAddrINET;
    addrConnect.addr = ipAddr;
    addrConnect.port = destination.port;
    if (NetLibSocketConnect(refNum, socket, reinterpret_cast<NetSocketAddrType*>(&addrConnect),
                            sizeof(addrConnect), -1, &err) != 0) {
        Printf("failed to connect: %i\n", err);
        goto cleanup;
    }

    NetIOVecType chunks[4];
    UInt8 buffer[256];
    MemSet(buffer, 256, 0);

    for (int i = 0; i < 4; i++) {
        chunks[i].bufLen = 63;
        chunks[i].bufP = buffer + i * 64;
    }

    NetIOParamType params;
    params.addrP = 0;
    params.addrLen = 0;
    params.iov = chunks;
    params.iovLen = 4;
    params.accessRights = 0;
    params.accessRightsLen = 0;

    Printf("receiving...\n");

    Int16 bytesReceived;
    bytesReceived = NetLibReceivePB(refNum, socket, &params, 0, -1, &err);
    if (bytesReceived < 0) {
        Printf("failed to receive: %i\n", err);
        goto cleanup;
    }

    Printf("received %i bytes\n", bytesReceived);

    for (int i = 0; i < 4; i++) {
        Printf("chunk %i: %s\n", i, params.iov[i].bufP);
    }

cleanup:
    if (socket > 0) NetLibSocketClose(refNum, socket, -1, &err);
    NetLibClose(refNum, true);

    Printf("\n");
}

void TestSendPB() {
    NetSocketRef socket = -1;
    Err err;
    static const char* words[] = {"one ", "two ", "three "};
    const UInt8 wordCount = 5;

    DB::Address destination;
    db.GetDestinationSendPB(destination);

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

    NetIPAddr ipAddr = NetLibAddrAToIN(refNum, destination.ip);
    if (static_cast<Int32>(ipAddr) < 0) {
        Printf("invalid IP address %s\n", destination.ip);
        goto cleanup;
    }

    Printf("connecting to %s:%u\n", destination.ip, destination.port);

    socket = NetLibSocketOpen(refNum, netSocketAddrINET, netSocketTypeStream, netSocketProtoIPTCP,
                              -1, &err);
    if (err != 0) {
        Printf("failed to open socket: %i\n", err);
        goto cleanup;
    }

    NetSocketAddrINType addr;
    addr.family = netSocketAddrINET;
    addr.addr = ipAddr;
    addr.port = destination.port;
    if (NetLibSocketConnect(refNum, socket, reinterpret_cast<NetSocketAddrType*>(&addr),
                            sizeof(addr), -1, &err) != 0) {
        Printf("failed to connect: %i\n", err);
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
        Printf("failed to send: %i\n", err);

cleanup:
    if (socket > 0) NetLibSocketClose(refNum, socket, -1, &err);
    NetLibClose(refNum, true);

    Printf("\n");
}

void ShowNameserver() {
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

    UInt32 dns;
    UInt16 len = sizeof(dns);
    char buffer[16];

    if (NetLibSettingGet(refNum, netSettingPrimaryDNS, &dns, &len) == 0) {
        NetLibAddrINToA(refNum, dns, buffer);

        Printf("Primary DNS: %s\n", buffer);
    } else {
        Printf("failed to get primary DNS\n");
    }

    NetLibClose(refNum, true);
}

void GetHostByName() {
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

    Printf("resolving www.google.de...\n");

    NetHostInfoBufType buf;
    Err err;

    NetHostInfoPtr infobuf = NetLibGetHostByName(refNum, "www.google.de", &buf, -1, &err);
    if (!infobuf) {
        Printf("lookup failed: %i\n", err);
    } else {
        Printf("reported name: %s\n", infobuf->nameP);

        for (int i = 0; infobuf->addrListP[i] != NULL; i++) {
            Printf("lookup result %i: %i.%i.%i.%i\n", i, infobuf->addrListP[i][0],
                   infobuf->addrListP[i][1], infobuf->addrListP[i][2], infobuf->addrListP[i][3]);
        }

        for (int i = 0; infobuf->nameAliasesP[i] != NULL; i++) {
            Printf("alias %i: %s\n", i, infobuf->nameAliasesP[i]);
        }
    }

    NetLibClose(refNum, true);
}

void GetServByName() {
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

    Printf("Resolving SMTP / TCP\n");

    NetServInfoBufType buf;
    Err err;

    NetServInfoType* info = NetLibGetServByName(refNum, "smtp", "tcp", &buf, 0, &err);

    if (!info) {
        Printf("lookup failed with %i\n", err);
    } else {
        Printf("lookup returned port %i\n", info->port);
    }

    NetLibClose(refNum, true);
}