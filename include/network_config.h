#pragma once

void vmem_server_task(void * pvParameters);
void router_task(void * pvParameters);
void onehz_task(void * pvParameters);

void setup_csp_tasks(void);

int setup_network(void);
