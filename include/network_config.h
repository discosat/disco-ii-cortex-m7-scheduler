#pragma once

void* vmem_server_task(void* param);
void* router_task(void* param);
static void* onehz_task(void* param);

// static void iface_can_init(void);  // TODO
// static void iface_kiss_init(void);  // TODO

static void csp_init_fun(void);

int setup_network(void);
