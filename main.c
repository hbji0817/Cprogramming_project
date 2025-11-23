#include <stdio.h>
#include <string.h>

#include "delivery_system.h"

int main(void) {
    char input_code[20];

    // 프로그램 시작 시 데이터 로드
    load_data();

    printf("================================================\n");
    printf("                  주문 시스템\n");
    printf("================================================\n");

    while (1) {
        printf("\n------------------------------------------------\n");
        printf("접속: 고객 코드를 입력하거나, 직원 비밀번호(%s)를 입력하세요.\n", STAFF_PIN);
        printf("종료하려면 'quit'을 입력하세요.\n");
        printf("코드 입력: ");

        if (fgets(input_code, sizeof(input_code), stdin) == NULL)
            continue;
        remove_newline(input_code);

        if (strcmp(input_code, "quit") == 0 || strcmp(input_code, "q") == 0) {
            save_data();
            printf("\n[시스템 종료] 이용해 주셔서 감사합니다. 데이터가 저장되었습니다.\n");
            break;
        }

        if (strcmp(input_code, STAFF_PIN) == 0) {
            staff_menu();
        } else if (strlen(input_code) > 0) {
            customer_menu(input_code);
        } else {
            printf("\n코드를 입력해야 합니다.\n");
        }
    }

    return 0;
}