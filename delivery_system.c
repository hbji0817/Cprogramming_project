#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "delivery_system.h"


Order orders[MAX_ORDERS];
int order_count = 0;
int next_order_id = 1;

Customer customers[MAX_CUSTOMERS];
int active_customer_count = 0;

MenuItem menu_items[MAX_MENU_ITEMS]; // 메뉴 항목 배열
int menu_count = 0;                  // 현재 메뉴 항목 수
int next_menu_id = 1;                // 다음 메뉴 ID

// 보조 함수
void remove_newline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// ----------------------------------------------------
// 영속성 함수 (메뉴 데이터 포함)
// ----------------------------------------------------

void save_menu_data(void) {
    FILE* menu_file = fopen(MENU_FILE_NAME, "wb");
    if (menu_file) {
        fwrite(&menu_count, sizeof(int), 1, menu_file);
        fwrite(&next_menu_id, sizeof(int), 1, menu_file);
        if (menu_count > 0) {
            fwrite(menu_items, sizeof(MenuItem), menu_count, menu_file);
        }
        fclose(menu_file);
    } else {
        printf("메뉴 파일 저장에 실패했습니다.\n");
    }
}

void load_menu_data(void) {
    FILE* menu_file = fopen(MENU_FILE_NAME, "rb");
    if (menu_file) {
        if (fread(&menu_count, sizeof(int), 1, menu_file) != 1 || fread(&next_menu_id, sizeof(int), 1, menu_file) != 1) {
            printf("메뉴 파일이 손상되었거나 형식이 맞지 않습니다. 초기화합니다.\n");
            menu_count   = 0;
            next_menu_id = 1;
        } else if (menu_count > 0) {
            fread(menu_items, sizeof(MenuItem), menu_count, menu_file);
            printf("%d개의 메뉴 데이터를 로드했습니다.\n", menu_count);
        } else {
            printf("저장된 메뉴 데이터가 없습니다.\n");
        }
        fclose(menu_file);
    } else {
        printf("메뉴 파일 (%s)이 없어 초기값으로 시작합니다.\n", MENU_FILE_NAME);
    }
}

void save_data(void) {
    FILE* order_file = fopen(ORDERS_FILE_NAME, "wb");
    if (order_file) {
        fwrite(&order_count,   sizeof(int), 1, order_file);
        fwrite(&next_order_id, sizeof(int), 1, order_file);
        if (order_count > 0) {
            fwrite(orders, sizeof(Order), order_count, order_file);
        }
        fclose(order_file);
    } else {
        printf("주문 파일 저장에 실패했습니다.\n");
    }

    FILE* customer_file = fopen(CUSTOMERS_FILE_NAME, "wb");
    if (customer_file) {
        fwrite(&active_customer_count, sizeof(int), 1, customer_file);
        if (active_customer_count > 0) {
            fwrite(customers, sizeof(Customer), active_customer_count, customer_file);
        }
        fclose(customer_file);
    } else {
        printf("고객 파일 저장에 실패했습니다.\n");
    }
    
    save_menu_data(); // 메뉴 데이터 저장
}

void load_data(void) {
    load_menu_data(); // 메뉴 데이터 로드

    FILE* order_file = fopen(ORDERS_FILE_NAME, "rb");
    if (order_file) {
        if (fread(&order_count, sizeof(int), 1, order_file) != 1 || fread(&next_order_id, sizeof(int), 1, order_file) != 1) {
            printf("주문 파일이 손상되었거나 형식이 맞지 않습니다. 초기화합니다.\n");
            order_count   = 0;
            next_order_id = 1;
        } else if (order_count > 0) {
            fread(orders, sizeof(Order), order_count, order_file);
            printf("%d개의 주문 데이터를 로드했습니다.\n", order_count);
        } else {
            printf("저장된 주문 데이터가 없습니다.\n");
        }
        fclose(order_file);
    } else {
        printf("주문 파일 (%s)이 없어 초기값으로 시작합니다.\n", ORDERS_FILE_NAME);
    }

    FILE* customer_file = fopen(CUSTOMERS_FILE_NAME, "rb");
    if (customer_file) {
        if (fread(&active_customer_count, sizeof(int), 1, customer_file) != 1) {
            printf("고객 파일이 손상되었거나 형식이 맞지 않습니다. 초기화합니다.\n");
            active_customer_count = 0;
        } else if (active_customer_count > 0) {
            fread(customers, sizeof(Customer), active_customer_count, customer_file);
            printf("%d명의 고객 데이터를 로드했습니다.\n", active_customer_count);
        } else {
            printf("저장된 고객 데이터가 없습니다.\n");
        }
        fclose(customer_file);
    } else {
        printf("고객 파일 (%s)이 없어 초기값으로 시작합니다.\n", CUSTOMERS_FILE_NAME);
    }
}

// ----------------------------------------------------
// 고객 시스템 함수
// ----------------------------------------------------

int find_customer_index_by_code(const char* code) {
    for (int i = 0; i < active_customer_count; i++) {
        if (strcmp(customers[i].customer_code, code) == 0) {
            return i;
        }
    }
    return -1;
}

void add_new_customer(const char* code) {
    if (active_customer_count >= MAX_CUSTOMERS) {
        printf("최대 고객 수를 초과하여 새 고객을 등록할 수 없습니다.\n");
        return;
    }
    Customer new_cust;
    strncpy(new_cust.customer_code, code, sizeof(new_cust.customer_code) - 1);
    new_cust.customer_code[sizeof(new_cust.customer_code) - 1] = '\0';
    new_cust.visit_count = 1; // 첫 주문이므로 1로 시작
    new_cust.is_vip = 0;
    customers[active_customer_count++] = new_cust;
    printf("신규 고객 등록: %s\n", code);
}

void check_and_promote_vip(const char* customer_code, int order_idx) {
    int cust_idx = find_customer_index_by_code(customer_code);

    if (cust_idx == -1) {
        add_new_customer(customer_code);
        cust_idx = find_customer_index_by_code(customer_code);
    } else {
        customers[cust_idx].visit_count++;
    }

    // VIP 승격 확인
    if (customers[cust_idx].visit_count >= VIP && !customers[cust_idx].is_vip) {
        customers[cust_idx].is_vip = 1;
        printf("🎉 축하합니다! 고객 코드 %s님이 VIP로 승격되었습니다! 🎉\n", customer_code);
    }

    // 주문에 VIP 여부 표시
    if (customers[cust_idx].is_vip) {
        orders[order_idx].is_vip_order = 1;
    }
}

// ----------------------------------------------------
// 메뉴 관리 함수 (추가됨)
// ----------------------------------------------------

int find_menu_index_by_id(int menu_id) {
    for (int i = 0; i < menu_count; i++) {
        if (menu_items[i].menu_id == menu_id) {
            return i;
        }
    }
    return -1;
}

void print_menu(void) {
    if (menu_count == 0) {
        printf("\n[메뉴 목록] - 메뉴가 등록되지 않았습니다.\n");
        return;
    }
    printf("\n--- [메뉴 목록] ---\n");
    printf("ID   | 메뉴 이름           | 가격\n");
    printf("-----|---------------------|--------\n");
    for (int i = 0; i < menu_count; i++) {
        printf("%-4d | %-19s | %d원\n", menu_items[i].menu_id, menu_items[i].item_name, menu_items[i].price);
    }
    printf("-------------------------\n");
}

void add_menu_item(void) {
    if (menu_count >= MAX_MENU_ITEMS) {
        printf("더 이상 메뉴를 추가할 수 없습니다 (최대 %d개).\n", MAX_MENU_ITEMS);
        return;
    }

    MenuItem new_item;
    new_item.menu_id = next_menu_id;

    printf("추가할 메뉴 이름: ");
    fgets(new_item.item_name, sizeof(new_item.item_name), stdin);
    remove_newline(new_item.item_name);

    printf("메뉴 가격 (원): ");
    if (scanf("%d", &new_item.price) != 1 || new_item.price <= 0) {
        printf("잘못된 가격 입력입니다. 메뉴 추가 취소.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF);
        return;
    }
    getchar();

    menu_items[menu_count++] = new_item;
    next_menu_id++;
    save_menu_data();
    printf("\n메뉴 '%s' (ID: %d)가 성공적으로 추가되었습니다.\n", new_item.item_name, new_item.menu_id);
}

void update_menu_item(void) {
    print_menu();
    if (menu_count == 0) return;

    int menu_id;
    printf("수정할 메뉴 ID 입력: ");
    if (scanf("%d", &menu_id) != 1) {
        printf("잘못된 입력입니다.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF);
        return;
    }
    getchar();

    int idx = find_menu_index_by_id(menu_id);
    if (idx == -1) {
        printf("해당 ID의 메뉴를 찾을 수 없습니다.\n");
        return;
    }

    MenuItem* item = &menu_items[idx];
    printf("현재 메뉴: %s (%d원)\n", item->item_name, item->price);
    
    printf("새 메뉴 이름 (현재: %s, 변경하지 않으려면 Enter): ", item->item_name);
    char new_name[50];
    if (fgets(new_name, sizeof(new_name), stdin) != NULL) {
        remove_newline(new_name);
        if (strlen(new_name) > 0) {
            strncpy(item->item_name, new_name, sizeof(item->item_name) - 1);
            item->item_name[sizeof(item->item_name) - 1] = '\0';
        }
    }

    printf("새 메뉴 가격 (현재: %d원, 변경하지 않으려면 0 입력): ", item->price);
    int new_price;
    if (scanf("%d", &new_price) != 1) {
        printf("잘못된 입력입니다. 가격 변경 취소.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF);
    } else {
        if (new_price > 0) {
            item->price = new_price;
        } else if (new_price != 0) {
            printf("가격은 양수여야 합니다. 가격 변경 취소.\n");
        }
    }
    getchar();

    save_menu_data();
    printf("\n메뉴 ID %d가 성공적으로 수정되었습니다. (새 이름: %s, 새 가격: %d원)\n", menu_id, item->item_name, item->price);
}

void delete_menu_item(void) {
    print_menu();
    if (menu_count == 0) return;

    int menu_id;
    printf("삭제할 메뉴 ID 입력: ");
    if (scanf("%d", &menu_id) != 1) {
        printf("잘못된 입력입니다.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF);
        return;
    }
    getchar();

    int idx = find_menu_index_by_id(menu_id);
    if (idx == -1) {
        printf("해당 ID의 메뉴를 찾을 수 없습니다.\n");
        return;
    }

    // 삭제: 배열 요소 이동
    printf("메뉴 '%s' (ID: %d)를 정말로 삭제하시겠습니까? (yes/no): ", menu_items[idx].item_name, menu_id);
    char confirm[8];
    if (fgets(confirm, sizeof(confirm), stdin) == NULL) {
        printf("입력 오류로 취소합니다.\n");
        return;
    }
    remove_newline(confirm);

    if (strcmp(confirm, "yes") == 0 || strcmp(confirm, "y") == 0) {
        // 배열에서 삭제
        for (int i = idx; i < menu_count - 1; i++) {
            menu_items[i] = menu_items[i + 1];
        }
        menu_count--;
        save_menu_data();
        printf("\n메뉴 ID %d가 성공적으로 삭제되었습니다.\n", menu_id);
    } else {
        printf("메뉴 삭제가 취소되었습니다.\n");
    }
}

void menu_management(void) {
    int choice;
    while (1) {
        print_menu();
        printf("\n[메뉴 관리]\n");
        printf("1. 메뉴 추가\n");
        printf("2. 메뉴 수정\n");
        printf("3. 메뉴 삭제\n");
        printf("4. 뒤로 가기\n");
        printf("선택: ");
        if (scanf("%d", &choice) != 1) {
            printf("잘못된 입력입니다.\n");
            int c; while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        getchar();

        switch (choice) {
            case 1:
                add_menu_item();
                break;
            case 2:
                update_menu_item();
                break;
            case 3:
                delete_menu_item();
                break;
            case 4:
                return;
            default:
                printf("유효하지 않은 선택입니다.\n");
                break;
        }
    }
}


// ----------------------------------------------------
// 주문 시스템 함수
// ----------------------------------------------------

// 메뉴창
void staff_menu(void) {
    int choice;
    while (1) {
        printf("\n[직원 메뉴]\n");
        printf("1. 전체 주문 조회 (VIP 최우선 배치)\n");
        printf("2. 주문 완료 처리\n");
        printf("3. 메뉴 수정하기 (추가/수정/삭제)\n"); // 수정된 항목
        printf("4. 단골 고객 현황 조회\n");
        printf("5. 모든 주문 삭제\n"); 
        printf("6. 뒤로 가기\n"); // 뒤로 가기 위치 변경
        printf("선택: ");
        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("잘못된 입력입니다.\n");
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
                menu_management(); // 메뉴 관리 기능 호출
                break;
            case 4:
                // 단골 고객 현황 출력 함수가 있으면 호출 (예: print_customers())
                printf("단골 고객 현황 기능은 구현되어 있지 않거나 별도 함수가 필요합니다.\n");
                break;
            case 5:
                delete_all_orders();
                break;
            case 6:
                return;
            default:
                printf("유효하지 않은 선택입니다.\n");
                break;
        }
    }
}


void customer_menu(const char* customer_code) {
    int choice;
    int cust_idx = find_customer_index_by_code(customer_code);

    printf("\n[고객 메뉴 - 코드: %s]\n", customer_code);

    if (cust_idx != -1) {
        if (customers[cust_idx].is_vip) {
            printf("✨ VIP 혜택: 주문이 대기열 최우선으로 배치되어 더 빠르게 배달됩니다. ✨\n");
        } else {
            int needed = VIP - customers[cust_idx].visit_count;
            if (needed > 0) {
                printf("💡 %d회 더 주문하시면 단골 혜택이 시작됩니다. 💡\n", needed);
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

// 주문 추가 (메뉴 ID 선택 방식으로 변경)
void add_order(const char* customer_code) {
    if (menu_count == 0) {
        printf("\n현재 주문 가능한 메뉴가 없습니다. 직원에게 문의하세요.\n");
        return;
    }

    print_menu(); // 메뉴 목록 표시

    int selected_menu_id;
    printf("주문할 메뉴의 ID 입력: ");
    if (scanf("%d", &selected_menu_id) != 1) {
        printf("\n잘못된 입력입니다. 주문 취소.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF);
        return;
    }
    getchar();

    int menu_idx = find_menu_index_by_id(selected_menu_id);
    if (menu_idx == -1) {
        printf("\n해당 ID의 메뉴를 찾을 수 없습니다. 주문 취소.\n");
        return;
    }

    if (order_count >= MAX_ORDERS) {
        printf("\n더 이상 주문을 받을 수 없습니다.\n");
        return;
    }

    Order new_order;
    int   order_idx = order_count;

    new_order.id = next_order_id++;
    strncpy(new_order.customer_code, customer_code, sizeof(new_order.customer_code) - 1);
    new_order.customer_code[sizeof(new_order.customer_code) - 1] = '\0';

    new_order.menu_id = selected_menu_id;
    strncpy(new_order.item_name, menu_items[menu_idx].item_name, sizeof(new_order.item_name) - 1);
    new_order.item_name[sizeof(new_order.item_name) - 1] = '\0';
    
    printf("선택 메뉴: %s (%d원)\n", new_order.item_name, menu_items[menu_idx].price);

    printf("수량: ");
    if (scanf("%d", &new_order.quantity) != 1 || new_order.quantity <= 0) {
        printf("\n잘못된 수량 입력입니다. 주문 취소.\n");
        next_order_id--;
        int c; while ((c = getchar()) != '\n' && c != EOF);
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

    // 2. 일반 대기 주문 (시간 순으로 정렬하는 것이 이상적이지만 단순화를 위해 그대로 출력)
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

// 모든 주문 삭제
void delete_all_orders(void) {
    char confirm[8];
    printf("모든 주문을 삭제하면 복구할 수 없습니다. 계속하시겠습니까? (yes/no): ");
    if (fgets(confirm, sizeof(confirm), stdin) == NULL) {
        printf("입력 오류로 취소합니다.\n");
        return;
    }
    remove_newline(confirm);
    if (strcmp(confirm, "yes") == 0 || strcmp(confirm, "y") == 0) {
        // 주문 목록 초기화
        order_count = 0;
        next_order_id = 1;
        memset(orders, 0, sizeof(orders));
        save_data();
        printf("모든 주문이 삭제되었습니다.\n");
    } else {
        printf("삭제가 취소되었습니다.\n");
    }
}

