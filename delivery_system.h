#ifndef DELIVERY_SYSTEM_H
#define DELIVERY_SYSTEM_H

#include <time.h>

#define MAX_ORDERS 100
#define MAX_MENU_ITEMS 30 // 최대 메뉴 항목 수
#define STAFF_PIN "0000"

// VIP 및 고객 관련 상수
#define VIP 5                     // 자동 VIP 승급 기준 횟수
#define MAX_CUSTOMERS 50          // 최대 고객 수

// 파일 영속성 관련 상수
#define ORDERS_FILE_NAME "orders.dat"
#define CUSTOMERS_FILE_NAME "customers.dat"
#define MENU_FILE_NAME "menu.dat" // 메뉴 파일 이름 추가

// 주문 상태 정의
#define STATUS_PENDING  0
#define STATUS_COMPLETE 1
#define STATUS_PREPARING 2
#define PREP_DELAY_SECONDS 30

// 메뉴 정보 구조체 추가
typedef struct {
    int  menu_id;        // 메뉴 고유 ID (1부터 시작)
    char item_name[50];  // 메뉴 이름
    int  price;          // 가격 (추가됨: 재고 관리는 단순화를 위해 생략)
} MenuItem;

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
    int    menu_id;            // 메뉴 ID로 변경
    char   item_name[50];      // 메뉴 이름 (저장을 위해 유지)
    int    quantity;           // 수량
    int    status;             // 0: 대기, 1: 완료
    int    is_vip_order;       // VIP 주문 여부
    time_t order_time;         // 주문 시간
} Order;

// 전역 변수 선언
extern Order    orders[MAX_ORDERS];
extern int      order_count;
extern int      next_order_id;

extern Customer customers[MAX_CUSTOMERS];
extern int      active_customer_count;

extern MenuItem menu_items[MAX_MENU_ITEMS]; // 메뉴 항목 배열
extern int      menu_count;                 // 현재 메뉴 항목 수
extern int      next_menu_id;               // 다음 메뉴 ID

// 배달 시스템
void staff_menu(void);
void customer_menu(const char* customer_code);
void add_order(const char* customer_code);
void view_orders(const char* customer_code);
void view_all_orders(void);
void mark_order_complete(void);
void remove_newline(char* str);
void delete_all_orders(void);
void check_and_update_status(void);
const char* get_status_string(int status);

// 고객 시스템
int  find_customer_index_by_code(const char* code);
void add_new_customer(const char* code);
void check_and_promote_vip(const char* customer_code, int order_idx);

// 메뉴 관리 시스템 (추가됨)
void menu_management(void);
void print_menu(void);
void add_menu_item(void);
void update_menu_item(void);
void delete_menu_item(void);
int  find_menu_index_by_id(int menu_id);

// 영속성 함수
void save_data(void);
void load_data(void);
void save_menu_data(void); // 메뉴 데이터 저장 추가
void load_menu_data(void); // 메뉴 데이터 로드 추가

#endif