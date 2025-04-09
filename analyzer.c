#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

int if_count = 0;

void analyze_function(json_object *func) {
    json_object *decl, *type, *rettype, *params, *body;

    // 함수 선언 정보
    json_object_object_get_ex(func, "decl", &decl);
    json_object_object_get_ex(decl, "type", &type);

    // 리턴 타입
    json_object *retType = type;
    while (json_object_object_get_ex(retType, "type", &rettype)) {
        retType = rettype;
    }

    json_object *ret_names;
    if (json_object_object_get_ex(retType, "names", &ret_names)) {
        printf("리턴 타입: ");
        for (int i = 0; i < json_object_array_length(ret_names); i++) {
            printf("%s ", json_object_get_string(json_object_array_get_idx(ret_names, i)));
        }
        printf("\n");
    }

    // 함수 이름
    json_object *declname;
    if (json_object_object_get_ex(type, "type", &type)) {
        json_object_object_get_ex(type, "declname", &declname);
        printf("함수 이름: %s\n", json_object_get_string(declname));
    }

    // 파라미터
    json_object *args;
    if (json_object_object_get_ex(type, "args", &args)) {
        json_object *params_array;
        if (json_object_object_get_ex(args, "params", &params_array)) {
            printf("파라미터:\n");
            for (int i = 0; i < json_object_array_length(params_array); i++) {
                json_object *param = json_object_array_get_idx(params_array, i);
                json_object *param_type = param;
                while (json_object_object_get_ex(param_type, "type", &param_type)) {}
                json_object *names;
                if (json_object_object_get_ex(param_type, "names", &names)) {
                    printf("  타입: ");
                    for (int j = 0; j < json_object_array_length(names); j++) {
                        printf("%s ", json_object_get_string(json_object_array_get_idx(names, j)));
                    }
                }
                json_object *name;
                if (json_object_object_get_ex(param, "name", &name)) {
                    printf("  이름: %s\n", json_object_get_string(name));
                } else {
                    printf("  이름: (없음)\n");
                }
            }
        }
    }

    // if 조건문 개수 카운트
    if_count = 0;
    if (json_object_object_get_ex(func, "body", &body)) {
        // 재귀 탐색
        struct json_object_iterator it = json_object_iter_begin(body);
        struct json_object_iterator end = json_object_iter_end(body);
        while (!json_object_iter_equal(&it, &end)) {
            json_object *value = json_object_iter_peek_value(&it);
            if (json_object_is_type(value, json_type_array)) {
                for (int i = 0; i < json_object_array_length(value); i++) {
                    json_object *stmt = json_object_array_get_idx(value, i);
                    json_object *nodetype;
                    if (json_object_object_get_ex(stmt, "_nodetype", &nodetype)) {
                        const char *type_str = json_object_get_string(nodetype);
                        if (strcmp(type_str, "If") == 0) {
                            if_count++;
                        }
                    }
                }
            }
            json_object_iter_next(&it);
        }
    }
    printf("if 조건문 개수: %d\n", if_count);
    printf("=================================\n");
}

int main() {
    const char *filename = "ast.json";
    json_object *root = json_object_from_file(filename);

    if (!root) {
        fprintf(stderr, "파일 %s을 불러올 수 없습니다.\n", filename);
        return 1;
    }

    json_object *ext_array;
    if (!json_object_object_get_ex(root, "ext", &ext_array)) {
        fprintf(stderr, "AST 구조에 'ext' 배열이 없습니다.\n");
        return 1;
    }

    int func_count = 0;

    printf("===== 함수 목록 및 정보 =====\n");

    for (int i = 0; i < json_object_array_length(ext_array); i++) {
        json_object *item = json_object_array_get_idx(ext_array, i);

        json_object *nodetype;
        if (json_object_object_get_ex(item, "_nodetype", &nodetype)) {
            const char *type_str = json_object_get_string(nodetype);

            if (strcmp(type_str, "FuncDef") == 0) {
                func_count++;
                analyze_function(item);
            }
        }
    }

    printf("전체 함수 개수: %d\n", func_count);

    json_object_put(root); // 메모리 해제
    return 0;
}
