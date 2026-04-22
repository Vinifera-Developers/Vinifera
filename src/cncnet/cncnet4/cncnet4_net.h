/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Network utility functions.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <stdint.h>
#include <winsock2.h>
#include <wsipx.h>


#define NET_BUF_SIZE 2048

typedef int socklen_t;

enum {
    CMD_TUNNEL,
    CMD_P2P,
    CMD_DISCONNECT,
    CMD_PING,
    CMD_QUERY,
    CMD_TESTP2P
};


extern int net_socket;

void ipx2in(struct sockaddr_ipx *from, struct sockaddr_in *to);
void in2ipx(struct sockaddr_in *from, struct sockaddr_ipx *to);
bool is_ipx_broadcast(struct sockaddr_ipx *addr);

int net_opt_reuse();
int net_opt_broadcast();

int net_address(struct sockaddr_in *addr, const char *host, uint16_t port);
void net_address_ex(struct sockaddr_in *addr, uint32_t ip, uint16_t port);

int net_init();
void net_free();

int net_bind(const char *ip, int port);

uint32_t net_read_size();
int8_t net_read_int8();
int16_t net_read_int16();
int32_t net_read_int32();
int net_read_data(void *, size_t);
int net_read_string(char *str, size_t len);

int net_write_int8(int8_t);
int net_write_int16(int16_t);
int net_write_int32(int32_t);
int net_write_data(void *, size_t);
int net_write_string(char *str);
int net_write_string_int32(int32_t);

int net_recv(struct sockaddr_in *);
int net_send(struct sockaddr_in *);
int net_send_noflush(struct sockaddr_in *dst);
void net_send_discard();
