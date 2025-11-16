// delivery_system.h
#ifndef DELIEVERY_SYSTEM_H
#define DELIEVERY_SYSTEM_H

#include <time.h>

#define MAX_ORDERS 100
#define STAFF_PIN "0000"

// VIP 및 고객 관련 상수
#define VIP_PROMOTION_THRESHOLD 5 // 자동 VIP 승급 기준 횟수
#define MAX_CUSTOMERS 50          // 최대 고객 수

// 파일 영속성 관련 상수
#define ORDERS_FILE_NAME "orders.dat"
#define CUSTOMERS_FILE_NAME "customers.dat"

// 주문 상태 정의
#define STATUS_PENDING  0
#define STATUS_COMPLETE 1

// 고객 정보 구조체
typedef struct {
    char customer_code[20];
    int  visit_count; // 방문 횟수
    int  is_vip;      // 1: VIP, 0: 일반 고객
} Customer;

// 주문 구조체 (VIP/시간 정보 포함)
typedef struct {
    int    id;                 // 주문 고유 ID
    char   customer_code[20];  // 고객 코드
    char   item_name[50];      // 메뉴 이름
    int    quantity;           // 수량
    int    status;             // 0: 대기, 1: 완료
    int    is_vip_order;       // VIP 주문 여부
    time_t order_time;         // 주문 시간
} Order;

extern Order    orders[MAX_ORDERS];
extern int      order_count;
extern int      next_order_id;

extern Customer customers[MAX_CUSTOMERS];
extern int      active_customer_count;


// 배달 시스템
void staff_menu(void);
void customer_menu(const char* customer_code);
void add_order(const char* customer_code);
void view_orders(const char* customer_code);
void view_all_orders(void);
void mark_order_complete(void);
void remove_newline(char* str);

// 고객 시스템
int  find_customer_index_by_code(const char* code);
void add_new_customer(const char* code);
void check_and_promote_vip(const char* customer_code, int order_idx);

// 영속성 함수
void save_data(void);
void load_data(void);

#endif
