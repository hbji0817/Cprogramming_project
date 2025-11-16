// customer_system.c
#include <stdio.h>
#include <string.h>
#include "delievery_system.h"

// 고객 코드로 customers 배열 인덱스 찾기
int find_customer_index_by_code(const char* code) {
    for (int i = 0; i < active_customer_count; i++) {
        if (strcmp(customers[i].customer_code, code) == 0) {
            return i;
        }
    }
    return -1;
}

// 신규 고객 등록
void add_new_customer(const char* code) {
    if (active_customer_count >= MAX_CUSTOMERS) {
        return;
    }

    if (find_customer_index_by_code(code) == -1) {
        strncpy(customers[active_customer_count].customer_code, code,
                sizeof(customers[active_customer_count].customer_code) - 1);
        customers[active_customer_count]
            .customer_code[sizeof(customers[active_customer_count].customer_code) - 1] = '\0';

        customers[active_customer_count].visit_count = 0;
        customers[active_customer_count].is_vip = 0;
        active_customer_count++;

        save_data();
    }
}

// VIP 승급 체크 및 혜택 적용
void check_and_promote_vip(const char* customer_code, int order_idx) {
    int cust_idx = find_customer_index_by_code(customer_code);

    if (cust_idx == -1) {
        add_new_customer(customer_code);
        cust_idx = find_customer_index_by_code(customer_code);
    }

    if (cust_idx != -1) {
        int was_vip = customers[cust_idx].is_vip;

        customers[cust_idx].visit_count++;

        if (customers[cust_idx].visit_count >= VIP) {
            orders[order_idx].is_vip_order = 1;

            if (was_vip == 0) {
                customers[cust_idx].is_vip = 1;
                printf("\n********************************************************\n");
                printf("  [단골 손님으로 승급되셨습니다!]\n");
                printf("  이 주문부터 VIP 혜택이 적용되어 빠르게 처리됩니다.\n");
                printf("********************************************************\n");
            } else {
                printf("단골 고객의 주문입니다. 대기열 최우선 배치가 적용됩니다.\n");
            }
        }

        if (customers[cust_idx].is_vip != was_vip ||
            customers[cust_idx].visit_count > 0) {
            save_data();
        }
    }
}
