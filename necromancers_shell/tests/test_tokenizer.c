#include "../src/commands/tokenizer.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void test_simple_tokens(void) {
    Token* tokens = NULL;
    size_t count = 0;

    TokenizeResult result = tokenize("help status quit", &tokens, &count);

    assert(result == TOKENIZE_SUCCESS);
    assert(count == 3);
    assert(strcmp(tokens[0].value, "help") == 0);
    assert(strcmp(tokens[1].value, "status") == 0);
    assert(strcmp(tokens[2].value, "quit") == 0);

    free_tokens(tokens, count);
    printf("[PASS] test_simple_tokens\n");
}

static void test_quoted_strings(void) {
    Token* tokens = NULL;
    size_t count = 0;

    TokenizeResult result = tokenize("echo \"hello world\"", &tokens, &count);

    assert(result == TOKENIZE_SUCCESS);
    assert(count == 2);
    assert(strcmp(tokens[0].value, "echo") == 0);
    assert(strcmp(tokens[1].value, "hello world") == 0);
    assert(tokens[1].is_quoted == true);

    free_tokens(tokens, count);
    printf("[PASS] test_quoted_strings\n");
}

static void test_escape_sequences(void) {
    Token* tokens = NULL;
    size_t count = 0;

    TokenizeResult result = tokenize("echo \"hello\\nworld\"", &tokens, &count);

    assert(result == TOKENIZE_SUCCESS);
    assert(count == 2);
    assert(strcmp(tokens[0].value, "echo") == 0);
    assert(tokens[1].value[5] == '\n');

    free_tokens(tokens, count);
    printf("[PASS] test_escape_sequences\n");
}

static void test_empty_input(void) {
    Token* tokens = NULL;
    size_t count = 0;

    TokenizeResult result = tokenize("", &tokens, &count);

    assert(result == TOKENIZE_SUCCESS);
    assert(count == 0);
    assert(tokens == NULL);

    printf("[PASS] test_empty_input\n");
}

static void test_unclosed_quote(void) {
    Token* tokens = NULL;
    size_t count = 0;

    TokenizeResult result = tokenize("echo \"hello", &tokens, &count);

    assert(result == TOKENIZE_ERROR_UNCLOSED_QUOTE);

    printf("[PASS] test_unclosed_quote\n");
}

int main(void) {
    printf("Running tokenizer tests...\n\n");

    test_simple_tokens();
    test_quoted_strings();
    test_escape_sequences();
    test_empty_input();
    test_unclosed_quote();

    printf("\nAll tokenizer tests passed!\n");
    return 0;
}
