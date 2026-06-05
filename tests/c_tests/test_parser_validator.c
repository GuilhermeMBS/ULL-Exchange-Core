/*
 * test_parser_validator.c — ULL Exchange Core
 * Suite de testes dos módulos Parser e Validator
 *
 * Responsável: Gabriel
 *
 * Cobre todos os casos descritos nas Tabelas 4.3, 4.4 e os cenários
 * relevantes da Tabela 4.8 do documento de especificação do projeto.
 *
 * Compilar:
 *   gcc -Wall -Wextra -o test_pv test_parser_validator.c \
 *       parser.c validator.c errorlib.c
 *
 * Executar:
 *   ./test_pv
 *
 * Cada teste imprime [PASS] ou [FAIL] com descrição.
 * O programa retorna 0 se todos passarem, 1 caso contrário.
 *
 * ─── Rastreabilidade com o documento ────────────────────────────────────
 *
 *  GRUPO | ID   | Descrição do caso
 *  ------+------+----------------------------------------------------------
 *  PRS   | P-01 | CSV com 100 ordens válidas carregadas corretamente
 *  PRS   | P-02 | count reflete exatamente o número de linhas de dados
 *  PRS   | P-03 | Todos os campos de uma ordem são convertidos corretamente
 *  PRS   | P-04 | Cabeçalho do CSV é ignorado
 *  PRS   | P-05 | Linhas em branco no meio do CSV são ignoradas
 *  PRS   | P-06 | Linha malformada (campos faltando) → is_valid=false
 *  PRS   | P-07 | CSV com path inválido → retorna NULL
 *  PRS   | P-08 | Ponteiros NULL em csv_path ou total_count → retorna NULL
 *  PRS   | P-09 | prs_free_buffer com ponteiro válido retorna 0
 *  PRS   | P-10 | prs_free_buffer(NULL) retorna -1 sem travar
 *  PRS   | P-11 | Buffer contíguo: ordens acessíveis por índice aritmético
 *  PRS   | P-12 | CSV com 1 milhão de ordens (stress): carrega sem falha
 *  VLD   | V-01 | Ordem válida com side='B' → is_valid=true
 *  VLD   | V-02 | Ordem válida com side='A' → is_valid=true
 *  VLD   | V-03 | price == 0.0 → is_valid=false, order_id=(uint32_t)-1
 *  VLD   | V-04 | price < 0   → is_valid=false, order_id=(uint32_t)-1
 *  VLD   | V-05 | quantity == 0 → is_valid=false, order_id=(uint32_t)-1
 *  VLD   | V-06 | side ∉ {'A','B'} → is_valid=false, order_id=(uint32_t)-1
 *  VLD   | V-07 | side minúsculo ('a','b') → inválida
 *  VLD   | V-08 | Ordem com múltiplos campos inválidos → basta 1 falhar
 *  VLD   | V-09 | vld_validate_order(NULL, n) → retorna sem travar
 *  VLD   | V-10 | vld_validate_order(buf, 0) → retorna sem travar
 *  VLD   | V-11 | buffer com 100 ordens: apenas as inválidas são marcadas
 *  INT   | I-01 | Fluxo completo: CSV misto → válidas ok, inválidas marcadas
 *  INT   | I-02 | Ordens inválidas têm order_id=(uint32_t)-1 após pipeline
 *  INT   | I-03 | Ordens válidas mantêm todos os campos intactos após pipeline
 *  INT   | I-04 | CSV fora de ordem temporal: parser carrega sem reordenar
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>   /* ptrdiff_t */

#include "parser.h"      /* prs_create_orders, prs_free_buffer */
#include "validator.h"   /* vld_validate_order                 */
#include "book.h"        /* obk_order_t, tm_stmp_t             */

/* ═══════════════════════════════════════════════════════════════════════
 * Infraestrutura de testes
 * ═══════════════════════════════════════════════════════════════════════ */

static int g_total  = 0;
static int g_passed = 0;

/* Macro de assert: imprime resultado e acumula placar */
#define CHECK(desc, expr)                                               \
    do {                                                                \
        g_total++;                                                      \
        if (expr) {                                                     \
            printf("  [PASS] %s\n", desc);                             \
            g_passed++;                                                 \
        } else {                                                        \
            printf("  [FAIL] %s  (linha %d)\n", desc, __LINE__);       \
        }                                                               \
    } while (0)

/* Macro de seção: imprime título e ID do caso */
#define SECTION(id, desc)  printf("\n[%s] %s\n", id, desc)

/* ─── Utilitários ──────────────────────────────────────────────────────── */

/*
 * write_csv — grava um arquivo CSV no caminho indicado.
 * Retorna 0 em sucesso, -1 em falha de I/O.
 */
static int write_csv(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fputs(content, f);
    fclose(f);
    return 0;
}

/*
 * write_csv_n_orders — gera um CSV com n ordens todas válidas.
 * side alterna B/A para simular mercado real.
 * Retorna 0 em sucesso, -1 em falha.
 */
static int write_csv_n_orders(const char *path, int n) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;

    fprintf(f, "timestamp,order_id,client_id,quantity,price,symbol,side\n");
    for (int i = 1; i <= n; i++) {
        char side = (i % 2 == 0) ? 'B' : 'A';
        fprintf(f,
                "%u,%d,%d,%d,%.2f,PETR4,%c\n",
                (unsigned)(1748000000 + i),
                i,
                1000 + i,
                10 + (i % 5),
                100.0 + (i % 50) * 0.5,
                side);
    }
    fclose(f);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════
 * GRUPO PRS — Testes do Parser  (Tabela 4.3)
 * ═══════════════════════════════════════════════════════════════════════ */

/* ── P-01 / P-02: CSV com 100 ordens válidas ───────────────────────────── */
static void test_P01_P02_100_ordens_validas(void) {
    SECTION("P-01/P-02", "CSV com 100 ordens validas: carregamento e count");

    const char *path = "/tmp/pv_p01.csv";
    if (write_csv_n_orders(path, 100) != 0) {
        printf("  [SKIP] Falha ao gerar CSV\n");
        return;
    }

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    CHECK("P-01: buffer != NULL",            buf != NULL);
    CHECK("P-02: count == 100",              count == 100);

    if (buf) {
        int all_valid = 1;
        for (int32_t i = 0; i < count; i++)
            if (!buf[i].is_valid) { all_valid = 0; break; }
        CHECK("P-01: todas as ordens sao validas", all_valid);
    }

    prs_free_buffer(buf);
}

/* ── P-03: Conversão correta de todos os campos ─────────────────────────── */
static void test_P03_conversao_de_campos(void) {
    SECTION("P-03", "Conversao correta de todos os campos de uma ordem");

    const char *path = "/tmp/pv_p03.csv";
    write_csv(path,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1748000042,7,999,250,123.75,VALE3,B\n"
    );

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    CHECK("P-03: buffer != NULL",             buf != NULL);
    CHECK("P-03: count == 1",                 count == 1);

    if (buf && count == 1) {
        CHECK("P-03: timestamp == 1748000042", buf[0].timestamp  == 1748000042u);
        CHECK("P-03: order_id  == 7",          buf[0].order_id   == 7u);
        CHECK("P-03: client_id == 999",        buf[0].client_id  == 999u);
        CHECK("P-03: quantity  == 250",        buf[0].quantity   == 250u);
        CHECK("P-03: price     == 123.75",     buf[0].price      == 123.75);
        CHECK("P-03: symbol    == VALE3",      strcmp(buf[0].symbol, "VALE3") == 0);
        CHECK("P-03: side      == 'B'",        buf[0].side       == 'B');
        CHECK("P-03: is_valid  == true",       buf[0].is_valid   == true);
    }

    prs_free_buffer(buf);
}

/* ── P-04: Cabeçalho ignorado ───────────────────────────────────────────── */
static void test_P04_cabecalho_ignorado(void) {
    SECTION("P-04", "Cabecalho do CSV e ignorado");

    const char *path = "/tmp/pv_p04.csv";
    write_csv(path,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1748000001,1,10,50,100.00,PETR4,A\n"
    );

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    CHECK("P-04: count == 1 (cabecalho nao contado)", count == 1);
    if (buf && count == 1)
        CHECK("P-04: order_id == 1 (nao leu cabecalho como ordem)",
              buf[0].order_id == 1u);

    prs_free_buffer(buf);
}

/* ── P-05: Linhas em branco no meio do CSV ──────────────────────────────── */
static void test_P05_linhas_em_branco(void) {
    SECTION("P-05", "Linhas em branco no meio do CSV sao ignoradas");

    const char *path = "/tmp/pv_p05.csv";
    write_csv(path,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1748000001,1,10,50,100.00,PETR4,B\n"
        "\n"
        "1748000002,2,11,30,99.00,PETR4,A\n"
        "\n"
    );

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    CHECK("P-05: count == 2 (linhas em branco excluidas)", count == 2);

    prs_free_buffer(buf);
}

/* ── P-06: Linha malformada → marcada inválida ──────────────────────────── */
static void test_P06_linha_malformada(void) {
    SECTION("P-06", "Linha malformada e marcada como invalida");

    const char *path = "/tmp/pv_p06.csv";
    write_csv(path,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1748000001,1,10,50\n"                          /* campos faltando */
        "1748000002,2,11,30,99.00,PETR4,A\n"           /* válida          */
    );

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    CHECK("P-06: count == 2 (malformada conta no buffer)", count == 2);
    if (buf && count == 2) {
        CHECK("P-06: buf[0] malformada: is_valid == false",
              buf[0].is_valid == false);
        CHECK("P-06: buf[0] malformada: order_id == (uint32_t)-1",
              buf[0].order_id == (uint32_t)-1);
        CHECK("P-06: buf[1] valida apos malformada: is_valid == true",
              buf[1].is_valid == true);
        CHECK("P-06: buf[1] valida: order_id == 2",
              buf[1].order_id == 2u);
    }

    prs_free_buffer(buf);
}

/* ── P-07: Path inválido → NULL ─────────────────────────────────────────── */
static void test_P07_path_invalido(void) {
    SECTION("P-07", "CSV com path invalido retorna NULL");

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders("/caminho/que/nao/existe.csv", &count);

    CHECK("P-07: retorna NULL",   buf == NULL);
    /* count não é verificado pois é indefinido quando retorna NULL */

    prs_free_buffer(buf);   /* deve ser seguro mesmo com NULL */
}

/* ── P-08: Ponteiros NULL nos parâmetros ────────────────────────────────── */
static void test_P08_parametros_null(void) {
    SECTION("P-08", "Ponteiros NULL em csv_path ou total_count retornam NULL");

    int32_t count = 0;

    obk_order_t *b1 = prs_create_orders(NULL, &count);
    CHECK("P-08: csv_path=NULL retorna NULL",   b1 == NULL);

    obk_order_t *b2 = prs_create_orders("/tmp/qualquer.csv", NULL);
    CHECK("P-08: total_count=NULL retorna NULL", b2 == NULL);

    prs_free_buffer(b1);
    prs_free_buffer(b2);
}

/* ── P-09: prs_free_buffer com ponteiro válido ──────────────────────────── */
static void test_P09_free_buffer_valido(void) {
    SECTION("P-09", "prs_free_buffer com ponteiro valido retorna 0");

    const char *path = "/tmp/pv_p09.csv";
    write_csv(path,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1748000001,1,10,50,100.00,PETR4,B\n"
    );

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    int32_t ret = prs_free_buffer(buf);
    CHECK("P-09: prs_free_buffer retorna 0", ret == 0);
}

/* ── P-10: prs_free_buffer(NULL) → -1, sem crash ───────────────────────── */
static void test_P10_free_buffer_null(void) {
    SECTION("P-10", "prs_free_buffer(NULL) retorna -1 sem travar");

    int32_t ret = prs_free_buffer(NULL);
    CHECK("P-10: retorna -1", ret == -1);
}

/* ── P-11: Buffer contíguo — acesso por índice aritmético ──────────────── */
static void test_P11_buffer_contiguo(void) {
    SECTION("P-11", "Buffer contiguo: ordens acessiveis por aritmetica de ponteiros");

    const char *path = "/tmp/pv_p11.csv";
    write_csv(path,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1748000001,1,10,50,100.00,PETR4,B\n"
        "1748000002,2,11,30,99.00,PETR4,A\n"
        "1748000003,3,12,20,101.00,PETR4,B\n"
    );

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    CHECK("P-11: count == 3", count == 3);

    if (buf && count == 3) {
        /* Acessa pelo ponteiro base + offset, não pelo índice do array */
        obk_order_t *p0 = buf + 0;
        obk_order_t *p1 = buf + 1;
        obk_order_t *p2 = buf + 2;

        CHECK("P-11: buf+0 order_id==1", p0->order_id == 1u);
        CHECK("P-11: buf+1 order_id==2", p1->order_id == 2u);
        CHECK("P-11: buf+2 order_id==3", p2->order_id == 3u);

        /* Verifica que a distância em bytes é exatamente sizeof(obk_order_t) */
        ptrdiff_t diff     = (char *)p1 - (char *)p0;
        ptrdiff_t expected = (ptrdiff_t)sizeof(obk_order_t);
        CHECK("P-11: distancia entre ordens == sizeof(obk_order_t)",
              diff == expected);
    }

    prs_free_buffer(buf);
}

/* ── P-12: Stress test — 1 000 000 de ordens ────────────────────────────── */
static void test_P12_stress_1_milhao(void) {
    SECTION("P-12", "Stress: 1.000.000 de ordens carregadas sem falha (Tabela 4.8)");

    const char *path = "/tmp/pv_p12.csv";
    const int N = 1000000;

    if (write_csv_n_orders(path, N) != 0) {
        printf("  [SKIP] Falha ao gerar CSV de stress\n");
        return;
    }

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    CHECK("P-12: buffer != NULL",         buf != NULL);
    CHECK("P-12: count == 1.000.000",     count == N);

    if (buf) {
        /* Verifica primeira, do meio e última para não percorrer tudo */
        CHECK("P-12: buf[0] valida",      buf[0].is_valid == true);
        CHECK("P-12: buf[N/2] valida",    buf[N/2].is_valid == true);
        CHECK("P-12: buf[N-1] valida",    buf[N-1].is_valid == true);
    }

    prs_free_buffer(buf);
}

/* ═══════════════════════════════════════════════════════════════════════
 * GRUPO VLD — Testes do Validator  (Tabela 4.4)
 * ═══════════════════════════════════════════════════════════════════════ */

/* Utilitário: monta uma obk_order_t mínima com is_valid=true */
static obk_order_t make_order(uint32_t id, double price,
                               uint32_t qty, char side) {
    obk_order_t o;
    memset(&o, 0, sizeof(o));
    o.order_id  = id;
    o.client_id = 100 + id;
    o.timestamp = 1748000000u + id;
    o.price     = price;
    o.quantity  = qty;
    o.side      = side;
    o.is_valid  = true;
    strncpy(o.symbol, "PETR4", sizeof(o.symbol) - 1);
    return o;
}

/* ── V-01: Ordem válida Bid ─────────────────────────────────────────────── */
static void test_V01_ordem_valida_bid(void) {
    SECTION("V-01", "Ordem valida com side='B' deve permanecer valida");

    obk_order_t buf[1] = { make_order(1, 100.0, 50, 'B') };
    vld_validate_order(buf, 1);

    CHECK("V-01: is_valid == true",      buf[0].is_valid == true);
    CHECK("V-01: order_id intacto == 1", buf[0].order_id == 1u);
}

/* ── V-02: Ordem válida Ask ─────────────────────────────────────────────── */
static void test_V02_ordem_valida_ask(void) {
    SECTION("V-02", "Ordem valida com side='A' deve permanecer valida");

    obk_order_t buf[1] = { make_order(2, 99.5, 30, 'A') };
    vld_validate_order(buf, 1);

    CHECK("V-02: is_valid == true",      buf[0].is_valid == true);
    CHECK("V-02: order_id intacto == 2", buf[0].order_id == 2u);
}

/* ── V-03: price == 0 ───────────────────────────────────────────────────── */
static void test_V03_price_zero(void) {
    SECTION("V-03", "price == 0.0 invalida a ordem (RF 1.1.3)");

    obk_order_t buf[1] = { make_order(3, 0.0, 10, 'B') };
    vld_validate_order(buf, 1);

    CHECK("V-03: is_valid == false",           buf[0].is_valid == false);
    CHECK("V-03: order_id == (uint32_t)-1",    buf[0].order_id == (uint32_t)-1);
}

/* ── V-04: price < 0 ────────────────────────────────────────────────────── */
static void test_V04_price_negativo(void) {
    SECTION("V-04", "price < 0 invalida a ordem (RF 1.1.3)");

    obk_order_t buf[1] = { make_order(4, -50.0, 10, 'A') };
    vld_validate_order(buf, 1);

    CHECK("V-04: is_valid == false",        buf[0].is_valid == false);
    CHECK("V-04: order_id == (uint32_t)-1", buf[0].order_id == (uint32_t)-1);
}

/* ── V-05: quantity == 0 ────────────────────────────────────────────────── */
static void test_V05_quantity_zero(void) {
    SECTION("V-05", "quantity == 0 invalida a ordem (RF 1.1.3)");

    obk_order_t buf[1] = { make_order(5, 100.0, 0, 'B') };
    vld_validate_order(buf, 1);

    CHECK("V-05: is_valid == false",        buf[0].is_valid == false);
    CHECK("V-05: order_id == (uint32_t)-1", buf[0].order_id == (uint32_t)-1);
}

/* ── V-06: side inválido (letra maiúscula fora do conjunto) ─────────────── */
static void test_V06_side_invalido(void) {
    SECTION("V-06", "side fora de {'A','B'} invalida a ordem (RF 1.1.3)");

    const char sides[] = { 'C', 'V', 'X', 'Z', '?', ' ', '0' };
    int n = (int)(sizeof(sides) / sizeof(sides[0]));

    for (int i = 0; i < n; i++) {
        obk_order_t o = make_order((uint32_t)(10 + i), 100.0, 10, sides[i]);
        vld_validate_order(&o, 1);

        char desc[64];
        snprintf(desc, sizeof(desc),
                 "V-06: side='%c' -> is_valid==false", sides[i]);
        CHECK(desc, o.is_valid == false);

        snprintf(desc, sizeof(desc),
                 "V-06: side='%c' -> order_id==(uint32_t)-1", sides[i]);
        CHECK(desc, o.order_id == (uint32_t)-1);
    }
}

/* ── V-07: side minúsculo ('a', 'b') ───────────────────────────────────── */
static void test_V07_side_minusculo(void) {
    SECTION("V-07", "side minusculo ('a','b') e invalido — apenas maiusculas aceitas");

    obk_order_t ba = make_order(20, 100.0, 10, 'a');
    obk_order_t bb = make_order(21, 100.0, 10, 'b');
    vld_validate_order(&ba, 1);
    vld_validate_order(&bb, 1);

    CHECK("V-07: 'a' -> is_valid==false",   ba.is_valid == false);
    CHECK("V-07: 'b' -> is_valid==false",   bb.is_valid == false);
}

/* ── V-08: múltiplos campos inválidos — basta o primeiro falhar ─────────── */
static void test_V08_multiplos_campos_invalidos(void) {
    SECTION("V-08", "Ordem com price=0 e quantity=0: invalida (fail-fast)");

    obk_order_t buf[1] = { make_order(30, 0.0, 0, 'X') };
    vld_validate_order(buf, 1);

    CHECK("V-08: is_valid == false",        buf[0].is_valid == false);
    CHECK("V-08: order_id == (uint32_t)-1", buf[0].order_id == (uint32_t)-1);
}

/* ── V-09: buffer NULL → retorna sem travar ─────────────────────────────── */
static void test_V09_buffer_null(void) {
    SECTION("V-09", "vld_validate_order(NULL, n) retorna sem travar");

    vld_validate_order(NULL, 5);   /* não deve causar crash */
    CHECK("V-09: execucao sem crash", 1);
}

/* ── V-10: count == 0 → retorna sem travar ──────────────────────────────── */
static void test_V10_count_zero(void) {
    SECTION("V-10", "vld_validate_order(buf, 0) retorna sem travar");

    obk_order_t buf[1] = { make_order(40, 100.0, 10, 'B') };
    vld_validate_order(buf, 0);   /* não deve processar nada */

    CHECK("V-10: is_valid nao alterada (era true)", buf[0].is_valid == true);
    CHECK("V-10: order_id nao alterado",            buf[0].order_id == 40u);
}

/* ── V-11: buffer com 100 ordens — só inválidas são marcadas ───────────── */
static void test_V11_100_ordens_mistas(void) {
    SECTION("V-11", "100 ordens: apenas as invalidas sao marcadas (Tabela 4.4)");

    const int N = 100;
    obk_order_t buf[100];

    /* Monta 100 ordens: índices pares válidos, ímpares inválidos (price=0) */
    for (int i = 0; i < N; i++) {
        double p = (i % 2 == 0) ? 100.0 + i : 0.0;
        buf[i] = make_order((uint32_t)(i + 1), p, 10, 'B');
    }

    vld_validate_order(buf, N);

    int valid_ok = 1, invalid_ok = 1;
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {  /* deve permanecer válida */
            if (!buf[i].is_valid || buf[i].order_id == (uint32_t)-1)
                valid_ok = 0;
        } else {            /* deve ser marcada inválida */
            if (buf[i].is_valid || buf[i].order_id != (uint32_t)-1)
                invalid_ok = 0;
        }
    }

    CHECK("V-11: 50 ordens validas permanecem validas",   valid_ok);
    CHECK("V-11: 50 ordens invalidas sao todas marcadas", invalid_ok);
}

/* ═══════════════════════════════════════════════════════════════════════
 * GRUPO INT — Testes integrados Parser + Validator  (Tabela 4.8)
 * ═══════════════════════════════════════════════════════════════════════ */

/* ── I-01: Fluxo completo CSV misto ─────────────────────────────────────── */
static void test_I01_fluxo_completo_misto(void) {
    SECTION("I-01", "Fluxo completo: CSV misto -> validas ok, invalidas marcadas (Tabela 4.8)");

    const char *path = "/tmp/pv_i01.csv";
    write_csv(path,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1748000001,1,10,100,150.00,PETR4,B\n"  /* valida  */
        "1748000002,2,11,200,149.00,PETR4,A\n"  /* valida  */
        "1748000003,3,12,50,0.00,PETR4,B\n"     /* price=0 */
        "1748000004,4,13,0,120.00,PETR4,A\n"    /* qty=0   */
        "1748000005,5,14,30,99.00,PETR4,X\n"    /* side=X  */
        "1748000006,6,15,80,200.00,PETR4,B\n"   /* valida  */
    );

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    CHECK("I-01: buffer != NULL",  buf != NULL);
    CHECK("I-01: count == 6",      count == 6);

    if (buf && count == 6) {
        CHECK("I-01: buf[0] valida  (price>0, qty>0, side=B)", buf[0].is_valid == true);
        CHECK("I-01: buf[1] valida  (price>0, qty>0, side=A)", buf[1].is_valid == true);
        CHECK("I-01: buf[2] invalida (price=0)",               buf[2].is_valid == false);
        CHECK("I-01: buf[3] invalida (qty=0)",                 buf[3].is_valid == false);
        CHECK("I-01: buf[4] invalida (side=X)",                buf[4].is_valid == false);
        CHECK("I-01: buf[5] valida  (price>0, qty>0, side=B)", buf[5].is_valid == true);
    }

    prs_free_buffer(buf);
}

/* ── I-02: Ordens inválidas têm order_id=(uint32_t)-1 após pipeline ─────── */
static void test_I02_invalidas_order_id(void) {
    SECTION("I-02", "Ordens invalidas: order_id==(uint32_t)-1 apos pipeline completo");

    const char *path = "/tmp/pv_i02.csv";
    write_csv(path,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1748000001,77,10,100,0.00,PETR4,B\n"   /* price=0 → inválida */
        "1748000002,88,11,0,100.00,PETR4,A\n"   /* qty=0   → inválida */
    );

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    if (buf && count == 2) {
        CHECK("I-02: buf[0] order_id==(uint32_t)-1", buf[0].order_id == (uint32_t)-1);
        CHECK("I-02: buf[1] order_id==(uint32_t)-1", buf[1].order_id == (uint32_t)-1);
    }

    prs_free_buffer(buf);
}

/* ── I-03: Ordens válidas mantêm campos intactos ───────────────────────── */
static void test_I03_validas_campos_intactos(void) {
    SECTION("I-03", "Ordens validas: todos os campos intactos apos pipeline");

    const char *path = "/tmp/pv_i03.csv";
    write_csv(path,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1748000010,55,200,150,275.50,VALE3,A\n"
    );

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    if (buf && count == 1) {
        CHECK("I-03: is_valid  == true",          buf[0].is_valid   == true);
        CHECK("I-03: order_id  == 55",            buf[0].order_id   == 55u);
        CHECK("I-03: client_id == 200",           buf[0].client_id  == 200u);
        CHECK("I-03: quantity  == 150",           buf[0].quantity   == 150u);
        CHECK("I-03: price     == 275.50",        buf[0].price      == 275.50);
        CHECK("I-03: side      == 'A'",           buf[0].side       == 'A');
        CHECK("I-03: symbol    == VALE3",         strcmp(buf[0].symbol, "VALE3") == 0);
        CHECK("I-03: timestamp == 1748000010",    buf[0].timestamp  == 1748000010u);
    }

    prs_free_buffer(buf);
}

/* ── I-04: CSV fora de ordem temporal — parser carrega sem reordenar ───── */
static void test_I04_fora_de_ordem_temporal(void) {
    SECTION("I-04", "CSV fora de ordem temporal: parser carrega na ordem do arquivo (Tabela 4.8)");

    const char *path = "/tmp/pv_i04.csv";
    /* timestamps fora de ordem crescente */
    write_csv(path,
        "timestamp,order_id,client_id,quantity,price,symbol,side\n"
        "1748000005,3,30,10,100.00,PETR4,B\n"
        "1748000001,1,10,10,100.00,PETR4,A\n"
        "1748000003,2,20,10,100.00,PETR4,B\n"
    );

    int32_t count = 0;
    obk_order_t *buf = prs_create_orders(path, &count);

    CHECK("I-04: count == 3",  count == 3);

    if (buf && count == 3) {
        /* O parser NÃO reordena — mantém a sequência do arquivo */
        CHECK("I-04: buf[0].order_id == 3 (primeira linha do arquivo)",
              buf[0].order_id == 3u);
        CHECK("I-04: buf[1].order_id == 1 (segunda linha do arquivo)",
              buf[1].order_id == 1u);
        CHECK("I-04: buf[2].order_id == 2 (terceira linha do arquivo)",
              buf[2].order_id == 2u);
    }

    prs_free_buffer(buf);
}

/* ═══════════════════════════════════════════════════════════════════════
 * main — executa todos os grupos em ordem
 * ═══════════════════════════════════════════════════════════════════════ */

int main(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   ULL Exchange Core — Suite de Testes: Parser + Validator   ║\n");
    printf("║   Cobertura: Tabelas 4.3, 4.4 e 4.8 da especificacao        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    /* ── Parser (Tabela 4.3) ── */
    printf("\n━━━ PARSER (Tabela 4.3) ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    test_P01_P02_100_ordens_validas();
    test_P03_conversao_de_campos();
    test_P04_cabecalho_ignorado();
    test_P05_linhas_em_branco();
    test_P06_linha_malformada();
    test_P07_path_invalido();
    test_P08_parametros_null();
    test_P09_free_buffer_valido();
    test_P10_free_buffer_null();
    test_P11_buffer_contiguo();
    test_P12_stress_1_milhao();

    /* ── Validator (Tabela 4.4) ── */
    printf("\n━━━ VALIDATOR (Tabela 4.4) ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    test_V01_ordem_valida_bid();
    test_V02_ordem_valida_ask();
    test_V03_price_zero();
    test_V04_price_negativo();
    test_V05_quantity_zero();
    test_V06_side_invalido();
    test_V07_side_minusculo();
    test_V08_multiplos_campos_invalidos();
    test_V09_buffer_null();
    test_V10_count_zero();
    test_V11_100_ordens_mistas();

    /* ── Integrados (Tabela 4.8) ── */
    printf("\n━━━ INTEGRADOS — Parser + Validator (Tabela 4.8) ━━━━━━━━━━━━━\n");
    test_I01_fluxo_completo_misto();
    test_I02_invalidas_order_id();
    test_I03_validas_campos_intactos();
    test_I04_fora_de_ordem_temporal();

    /* ── Placar final ── */
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  Resultado: %3d / %3d testes passaram",
           g_passed, g_total);
    int espacos = 30 - /* largura do trecho numérico */ 0;
    (void)espacos;
    printf("                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    return (g_passed == g_total) ? 0 : 1;
}