#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 최대 주문 개수, 직원 비밀번호 정의
#define MAX_ORDERS 100
#define STAFF_PIN "0000"

// 주문 상태 정의
#define STATUS_PENDING 0
#define STATUS_COMPLETE 1

// 주문 구조체
typedef struct {
    int id;                // 주문 고유 ID
    char customer_code[20]; // 고객 코드 (테이블 번호 등)
    char item_name[50];    // 메뉴 이름
    int quantity;          // 수량
    int status;            // 주문 상태 (0: 대기, 1: 완료)
} Order;

Order orders[MAX_ORDERS]; // 주문을 저장할 배열
int order_count = 0;      // 현재 주문 개수
int next_order_id = 1;    // 다음 주문에 할당할 ID

// 함수 선언 칸
void staff_menu();
void customer_menu(const char* customer_code);
void add_order(const char* customer_code);
void view_orders(const char* customer_code);
void view_all_orders();
void mark_order_complete();
void remove_newline(char* str);


// 메인 함수
int main() {
    char input_code[20];
    printf("================================================\n");
    printf("                  주문 관리 시스템\n");
    printf("================================================\n");

    while (1) {
        printf("\n------------------------------------------------\n");
        printf("접속: 고객 코드를 입력하거나, 직원 비밀번호(%s)를 입력하세요.\n", STAFF_PIN);
        printf("종료하려면 'q'를입력하세요.\n");
        printf("코드 입력: ");
        
        if (fgets(input_code, sizeof(input_code), stdin) == NULL) continue;
        remove_newline(input_code);

        // 종료 명령 처리
        if (strcmp(input_code, "quit") == 0 || strcmp(input_code, "종료") == 0) {
            printf("\n[시스템 종료] 이용해 주셔서 감사합니다.\n");
            break;
        }
        
        // 직원 비밀번호 확인
        if (strcmp(input_code, STAFF_PIN) == 0) {
            staff_menu();
        } 
        // 고객 코드 확인 (빈 입력은 허용하지 않음)
        else if (strlen(input_code) > 0) {
            customer_menu(input_code);
        } else {
            printf("\n[오류] 코드를 입력해야 합니다.\n");
        }
    }

    return 0;
}

// 함수 내용 구현 칸

void staff_menu() {
    int choice;
    while (1) {
        printf("\n[직원 메뉴]\n");
        printf("1. 전체 주문 조회\n");
        printf("2. 주문 완료 처리\n");
        printf("3. 뒤로 가기\n");
        printf("선택: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                view_all_orders();
                break;
            case 2:
                mark_order_complete();
                break;
            case 3:
                return;
            default:
                printf("\n[오류] 잘못된 선택입니다.\n");
        }
    }
}

void customer_menu(const char* customer_code) {
    int choice;
    while (1) {
        printf("\n[고객 메뉴 - 코드: %s]\n", customer_code);
        printf("1. 주문 추가\n");
        printf("2. 내 주문 조회\n");
        printf("3. 뒤로 가기\n");
        printf("선택: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                add_order(customer_code);
                break;
            case 2:
                view_orders(customer_code);
                break;
            case 3:
                return;
            default:
                printf("\n[오류] 잘못된 선택입니다.\n");
        }
    }
}

void add_order(const char* customer_code) {
    if (order_count >= MAX_ORDERS) {
        printf("\n[오류] 더 이상 주문을 받을 수 없습니다.\n");
        return;
    }

    Order new_order;
    new_order.id = next_order_id++;
    strncpy(new_order.customer_code, customer_code, sizeof(new_order.customer_code) - 1);
    new_order.customer_code[sizeof(new_order.customer_code) - 1] = '\0';

    printf("메뉴 이름: ");
    fgets(new_order.item_name, sizeof(new_order.item_name), stdin);
    remove_newline(new_order.item_name);

    printf("수량: ");
    scanf("%d", &new_order.quantity);
    getchar();

    new_order.status = STATUS_PENDING;

    orders[order_count++] = new_order;
    printf("\n[성공] 주문이 추가되었습니다. 주문 ID: %d\n", new_order.id);
}

void view_orders(const char* customer_code) {
    printf("\n[내 주문 조회 - 코드: %s]\n", customer_code);
    int found = 0;
    for (int i = 0; i < order_count; i++) {
        if (strcmp(orders[i].customer_code, customer_code) == 0) {
            found = 1;
            printf("주문 ID: %d | 메뉴: %s | 수량: %d | 상태: %s\n",
                   orders[i].id,
                   orders[i].item_name,
                   orders[i].quantity,
                   orders[i].status == STATUS_PENDING ? "대기" : "완료");
        }
    }
    if (!found) {
        printf("[정보] 주문 내역이 없습니다.\n");
    }
}

void view_all_orders() {
    printf("\n[전체 주문 조회]\n");
    if (order_count == 0) {
        printf("[정보] 주문 내역이 없습니다.\n");
        return;
    }
    for (int i = 0; i < order_count; i++) {
        printf("주문 ID: %d | 고객 코드: %s | 메뉴: %s | 수량: %d | 상태: %s\n",
               orders[i].id,
               orders[i].customer_code,
               orders[i].item_name,
               orders[i].quantity,
               orders[i].status == STATUS_PENDING ? "대기" : "완료");
    }
}

void mark_order_complete() {
    int order_id;
    printf("완료할 주문 ID 입력: ");
    scanf("%d", &order_id);
    getchar(); 

    for (int i = 0; i < order_count; i++) {
        if (orders[i].id == order_id) {
            if (orders[i].status == STATUS_COMPLETE) {
                printf("\n[정보] 이 주문은 이미 완료되었습니다.\n");
                return;
            }
            orders[i].status = STATUS_COMPLETE;
            printf("\n[성공] 주문 ID %d가 완료 처리되었습니다.\n", order_id);
            return;
        }
    }
    printf("\n[오류] 해당 주문 ID를 찾을 수 없습니다.\n");
}

void remove_newline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

