// delivery_system.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "delievery_system.h"


Order orders[MAX_ORDERS];
int order_count = 0;
int next_order_id = 1;

Customer customers[MAX_CUSTOMERS];
int active_customer_count = 0;

// 영속성 함수
void save_data(void) {
    FILE* order_file = fopen(ORDERS_FILE_NAME, "wb");
    if (order_file) {
        fwrite(&order_count,   sizeof(int), 1, order_file);
        fwrite(&next_order_id, sizeof(int), 1, order_file);
        if (order_count > 0) {
            fwrite(orders, sizeof(Order), order_count, order_file);
        }
        fclose(order_file);
    } 
    else {
        printf("주문 파일 저장에 실패했습니다.\n");
    }

    FILE* customer_file = fopen(CUSTOMERS_FILE_NAME, "wb");
    if (customer_file) {
        fwrite(&active_customer_count, sizeof(int), 1, customer_file);
        if (active_customer_count > 0) {
            fwrite(customers, sizeof(Customer), active_customer_count, customer_file);
        }
        fclose(customer_file);
    } 
    else {
        printf("고객 파일 저장에 실패했습니다.\n");
    }
}

void load_data(void) {
    FILE* order_file = fopen(ORDERS_FILE_NAME, "rb");
    if (order_file) {
        if (fread(&order_count, sizeof(int), 1, order_file) != 1 || fread(&next_order_id, sizeof(int), 1, order_file) != 1) {
            printf("주문 파일이 손상되었거나 형식이 맞지 않습니다. 초기화합니다.\n");
            order_count   = 0;
            next_order_id = 1;
        } 
        else if (order_count > 0) {
            fread(orders, sizeof(Order), order_count, order_file);
            printf("%d개의 주문 데이터를 로드했습니다.\n", order_count);
        } 
        else {
            printf("저장된 주문 데이터가 없습니다.\n");
        }
        fclose(order_file);
    } 
    else {
        printf("주문 파일 (%s)이 없어 초기값으로 시작합니다.\n", ORDERS_FILE_NAME);
    }

    FILE* customer_file = fopen(CUSTOMERS_FILE_NAME, "rb");
    if (customer_file) {
        if (fread(&active_customer_count, sizeof(int), 1, customer_file) != 1) {
            printf("고객 파일이 손상되었거나 형식이 맞지 않습니다. 초기화합니다.\n");
            active_customer_count = 0;
        } 
        else if (active_customer_count > 0) {
            fread(customers, sizeof(Customer), active_customer_count, customer_file);
            printf("%d명의 고객 데이터를 로드했습니다.\n", active_customer_count);
        } 
        else {
            printf("저장된 고객 데이터가 없습니다.\n");
        }
        fclose(customer_file);
    } 
    else {
        printf("고객 파일 (%s)이 없어 초기값으로 시작합니다.\n", CUSTOMERS_FILE_NAME);
    }
}

// 보조 함수
void remove_newline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// 메뉴창
void staff_menu(void) {
    int choice;
    while (1) {
        printf("\n[직원 메뉴]\n");
        printf("1. 전체 주문 조회 (VIP 최우선 배치)\n");
        printf("2. 주문 완료 처리\n");
        printf("3. 단골 고객 현황 조회\n");
        printf("4. 뒤로 가기\n");
        printf("선택: ");
        if (scanf("%d", &choice) != 1) {
            printf("\n잘못된 입력입니다.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        getchar();

        switch (choice) {
            case 1:
                view_all_orders();
                break;
            case 2:
                mark_order_complete();
                break;
            case 3:
                printf("\n--- 단골 고객 현황 (%d명) ---\n", active_customer_count);
                printf("코드     | 방문 횟수 | VIP 여부\n");
                printf("---------|-----------|--------\n");
                for (int i = 0; i < active_customer_count; i++) {
                    printf("%-8s | %-8d | %s\n",
                           customers[i].customer_code,
                           customers[i].visit_count,
                           customers[i].is_vip ? "VIP (최우선)" : "일반 단골");
                }
                printf("------------------------\n");
                break;
            case 4:
                return;
            default:
                printf("\n잘못된 선택입니다.\n");
        }
    }
}

void customer_menu(const char* customer_code) {
    int choice;
    int cust_idx = find_customer_index_by_code(customer_code);

    printf("\n[고객 메뉴 - 코드: %s]\n", customer_code);

    if (cust_idx != -1) {
        if (customers[cust_idx].is_vip) {
            printf("주문이 대기열 최우선으로 배치되어 더 빠르게 배달됩니다.\n");
        } else {
            int needed = VIP - customers[cust_idx].visit_count;
            if (needed > 0) {
                printf("%d회 더 주문하시면 단골 혜택이 시작됩니다.\n", needed);
            }
        }
    } else {
        printf("첫 주문을 환영합니다.\n");
    }

    while (1) {
        printf("\n1. 주문 추가\n");
        printf("2. 내 주문 조회\n");
        printf("3. 뒤로 가기\n");
        printf("선택: ");
        if (scanf("%d", &choice) != 1) {
            printf("\n잘못된 입력입니다.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        getchar();

        switch (choice) {
            case 1:
                add_order(customer_code);
                return;
            case 2:
                view_orders(customer_code);
                break;
            case 3:
                return;
            default:
                printf("\n잘못된 선택입니다.\n");
        }
    }
}

// 주문 관련 함수
void add_order(const char* customer_code) {
    if (order_count >= MAX_ORDERS) {
        printf("\n더 이상 주문을 받을 수 없습니다.\n");
        return;
    }

    Order new_order;
    int   order_idx = order_count;

    new_order.id = next_order_id++;
    strncpy(new_order.customer_code, customer_code, sizeof(new_order.customer_code) - 1);
    new_order.customer_code[sizeof(new_order.customer_code) - 1] = '\0';

    printf("메뉴 이름: ");
    fgets(new_order.item_name, sizeof(new_order.item_name), stdin);
    remove_newline(new_order.item_name);

    printf("수량: ");
    if (scanf("%d", &new_order.quantity) != 1 || new_order.quantity <= 0) {
        printf("\n잘못된 수량 입력입니다. 주문 취소.\n");
        next_order_id--;
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return;
    }
    getchar();

    new_order.status       = STATUS_PENDING;
    new_order.order_time   = time(NULL);
    new_order.is_vip_order = 0;

    orders[order_idx] = new_order;
    order_count++;

    check_and_promote_vip(customer_code, order_idx);

    save_data();

    printf("\n주문이 추가되었습니다. 주문 ID: %d\n", orders[order_idx].id);
}

// 내 주문 조회
void view_orders(const char* customer_code) {
    printf("\n[내 주문 조회 - 코드: %s]\n", customer_code);
    printf("--------------------------------------------------------------------\n");
    int found = 0;

    for (int i = 0; i < order_count; i++) {
        if (strcmp(orders[i].customer_code, customer_code) == 0) {
            found = 1;
            char time_buf[20];
            strftime(time_buf, 20, "%H:%M:%S", localtime(&orders[i].order_time));

            const char* vip_tag = orders[i].is_vip_order ? "[VIP]" : "";
            const char* priority_info = (orders[i].is_vip_order && orders[i].status == STATUS_PENDING) ? "(최우선)" : "";

            printf("ID: %d | 메뉴: %s | 수량: %d | 상태: %s %s | 시간: %s %s\n",
                   orders[i].id, orders[i].item_name, orders[i].quantity, orders[i].status == STATUS_PENDING ? "대기" : "완료", vip_tag, time_buf, priority_info);
        }
    }
    if (!found) {
        printf("주문 내역이 없습니다.\n");
    }
    printf("--------------------------------------------------------------------\n");
}

// 전체 주문 조회 (직원용)
void view_all_orders(void) {
    printf("\n[전체 주문 조회]\n");
    if (order_count == 0) {
        printf("전체 주문 내역이 없습니다.\n");
        return;
    }

    printf("--------------------------------------------------------------------\n");
    printf("ID   | 고객코드 | 메뉴 (수량)      | 상태   | 태그   | 주문 시간\n");
    printf("-----|----------|------------------|--------|--------|--------------------\n");

    // 1. VIP 대기 주문
    for (int i = 0; i < order_count; i++) {
        if (orders[i].status == STATUS_PENDING && orders[i].is_vip_order) {
            char time_buf[20];
            strftime(time_buf, 20, "%H:%M:%S",
                     localtime(&orders[i].order_time));
            printf("%-4d | %-8s | %-12s (%d) | 대기   | [VIP]  | %s (최우선 처리)\n",
                   orders[i].id,orders[i].customer_code,orders[i].item_name,orders[i].quantity,time_buf);
        }
    }

    // 2. 일반 대기 주문
    for (int i = 0; i < order_count; i++) {
        if (orders[i].status == STATUS_PENDING && !orders[i].is_vip_order) {
            char time_buf[20];
            strftime(time_buf, 20, "%H:%M:%S",
                     localtime(&orders[i].order_time));
            printf("%-4d | %-8s | %-12s (%d) | 대기   |        | %s\n",
                   orders[i].id,orders[i].customer_code,orders[i].item_name,orders[i].quantity,time_buf);
        }
    }

    // 3. 완료된 주문
    for (int i = 0; i < order_count; i++) {
        if (orders[i].status == STATUS_COMPLETE) {
            char time_buf[20];
            strftime(time_buf, 20, "%H:%M:%S",
                     localtime(&orders[i].order_time));
            const char* vip_tag = orders[i].is_vip_order ? "[VIP]" : "";
            printf("%-4d | %-8s | %-12s (%d) | 완료   | %-6s | %s\n",
                   orders[i].id, orders[i].customer_code, orders[i].item_name, orders[i].quantity, vip_tag, time_buf);
        }
    }
    printf("--------------------------------------------------------------------\n");
}

void mark_order_complete(void) {
    int order_id;
    printf("완료할 주문 ID 입력: ");
    if (scanf("%d", &order_id) != 1) {
        printf("\n잘못된 입력입니다.\n");
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return;
    }
    getchar();

    for (int i = 0; i < order_count; i++) {
        if (orders[i].id == order_id) {
            if (orders[i].status == STATUS_COMPLETE) {
                printf("\n이 주문은 이미 완료되었습니다.\n");
                return;
            }
            orders[i].status = STATUS_COMPLETE;
            save_data();
            printf("\n주문 ID %d가 완료 처리되었습니다.\n", order_id);
            return;
        }
    }
    printf("\n해당 주문 ID를 찾을 수 없습니다.\n");
}
