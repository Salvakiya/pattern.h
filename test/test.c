#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#undef __STRICT_ANSI__
#define CTEST_MAIN
#define CTEST_SEGFAULT
#define CTEST_COLOR_OK
#include "ctest.h"
#define PATTERN_IMPLEMENTATION
#include "../pattern.h"

int main(int argc, const char** argv) {
    return ctest_main(argc, argv);
}

static bool capture_eq(Pattern_Substring c, const char* o) {
    ASSERT_TRUE(c.size >= 0);
    return (size_t)c.size == strlen(o) && memcmp(o, c.data, c.size) == 0;
}

CTEST(pattern, star) {
    Pattern_State ps;
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaab", ".*b") && capture_eq(ps.captures[0], "aaab"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaa", ".*a") && capture_eq(ps.captures[0], "aaa"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "b", ".*b") && capture_eq(ps.captures[0], "b"));
}

CTEST(pattern, plus) {
    Pattern_State ps;
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaab", ".+b") && capture_eq(ps.captures[0], "aaab"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaa", ".+a") && capture_eq(ps.captures[0], "aaa"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "b", ".+b") == false);
    ASSERT_TRUE(ps.error == PATTERN_NO_MATCH);
}

CTEST(pattern, question) {
    Pattern_State ps;
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaab", ".?b") && capture_eq(ps.captures[0], "ab"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaa", ".?a") && capture_eq(ps.captures[0], "aa"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "b", ".?b") && capture_eq(ps.captures[0], "b"));
}

CTEST(pattern, captures) {
    Pattern_State ps;
    ASSERT_TRUE(pattern_match_cstr(&ps, "alo xyzK", "(%w+)K") && capture_eq(ps.captures[1], "xyz"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "254 K", "(%d*)K") && capture_eq(ps.captures[1], ""));
    ASSERT_TRUE(pattern_match_cstr(&ps, "alo ", "(%w*)$") && capture_eq(ps.captures[1], ""));
    ASSERT_TRUE(!pattern_match_cstr(&ps, "alo ", "(%w+)$"));
    ASSERT_TRUE(ps.error == PATTERN_NO_MATCH);
    ASSERT_TRUE(pattern_match_cstr(&ps, "testtset", "^(tes(t+)set)$"));
    ASSERT_TRUE(capture_eq(ps.captures[1], "testtset"));  // whole match
    ASSERT_TRUE(capture_eq(ps.captures[2], "tt"));        // inner (t+) group
}

CTEST(pattern, emptymatch) {
    Pattern_State ps;
    ASSERT_TRUE(pattern_match_cstr(&ps, "", "") && capture_eq(ps.captures[0], ""));
    ASSERT_TRUE(pattern_match_cstr(&ps, "alo", "") && capture_eq(ps.captures[0], ""));
}

CTEST(pattern, nul_in_data) {
    Pattern_State ps;
    ASSERT_TRUE(pattern_match(&ps, "a\0o a\0o a\0o", 11, "a") && capture_eq(ps.captures[0], "a"));
    ASSERT_TRUE(ps.data.data == ps.captures[0].data);
    ASSERT_TRUE(pattern_match(&ps, "a\0a\0a\0a\0\0ab", 11, "b") && capture_eq(ps.captures[0], "b"));
    ASSERT_TRUE(pattern_get_capture_pos(&ps, 0) == 10);
}

CTEST(pattern, nul_in_pattern) {
    Pattern_State ps;
    ASSERT_TRUE(!pattern_match(&ps, "a\0\0a\0ab", 7, "b%z"));
    ASSERT_TRUE(ps.error == PATTERN_NO_MATCH);
    ASSERT_TRUE(pattern_match(&ps, "a\0\0a\0ab\0", 8, "b%z") && ps.captures[0].size == 2 &&
                pattern_get_capture_pos(&ps, 0) == 6);
}

CTEST(pattern, start_anchor) {
    Pattern_State ps;
    ASSERT_TRUE(pattern_match_cstr(&ps, "cantami123odiva", "12") && capture_eq(ps.captures[0], "12"));
    ASSERT_TRUE(!pattern_match_cstr(&ps, "cantami123odiva", "^12"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "12cantami123odiva", "^12") && capture_eq(ps.captures[0], "12"));
    ASSERT_TRUE(ps.error == PATTERN_MATCH_OK && capture_eq(ps.captures[0], "12"));
    ASSERT_TRUE(ps.data.data == ps.captures[0].data);
}

CTEST(pattern, matches_and_operators) {
    Pattern_State ps;
    Pattern_Error status;
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "aloALO", "%l*") && capture_eq(ps.captures[0], "alo"));
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "aLo_ALO", "%a*") && capture_eq(ps.captures[0], "aLo"));
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "  \n\r*&\n\r   xuxu  \n\n", "%g%g%g+") && capture_eq(ps.captures[0], "xuxu"));
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaab", "a*") && capture_eq(ps.captures[0], "aaa"));
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaa", "^.*$") && capture_eq(ps.captures[0], "aaa"));
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaa", "b*") && capture_eq(ps.captures[0], ""));
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaa", "b*") && capture_eq(ps.captures[0], ""));
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaa", "ab*a") && capture_eq(ps.captures[0], "aa"));
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "aba", "ab*a") && capture_eq(ps.captures[0], "aba"));
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaab", "a+") && capture_eq(ps.captures[0], "aaa"));
    
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaa", "^.+$") && capture_eq(ps.captures[0], "aaa"));
    ASSERT_TRUE(!pattern_match_cstr(&ps, "aaa", "b+"));
    ASSERT_TRUE(!pattern_match_cstr(&ps, "aaa", "ab+a"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "aba", "ab+a") && capture_eq(ps.captures[0], "aba"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "a$a", ".$") && capture_eq(ps.captures[0], "a"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "a$a", ".%$") && capture_eq(ps.captures[0], "a$"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "a$a", ".$.") && capture_eq(ps.captures[0], "a$a"));
    ASSERT_TRUE(!pattern_match_cstr(&ps, "a$a", "$$"));
    ASSERT_TRUE(!pattern_match_cstr(&ps, "a$b", "a$"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "a$a", "$") && capture_eq(ps.captures[0], ""));
    ASSERT_TRUE(pattern_match_cstr(&ps, "", "b*") && capture_eq(ps.captures[0], ""));
    ASSERT_TRUE(!pattern_match_cstr(&ps, "aaa", "bb*"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaab", "a-") && capture_eq(ps.captures[0], ""));
    ASSERT_TRUE(pattern_match_cstr(&ps, "aaa", "^.-$") && capture_eq(ps.captures[0], "aaa"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "aabaaabaaabaaaba", "b.*b") && capture_eq(ps.captures[0], "baaabaaabaaab"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "aabaaabaaabaaaba", "b.-b") && capture_eq(ps.captures[0], "baaab"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "alo xo", ".o$") && capture_eq(ps.captures[0], "xo"));
    ASSERT_TRUE(pattern_match_cstr(&ps, " \n isto é assim", "%S%S*") && capture_eq(ps.captures[0], "isto"));
    ASSERT_TRUE(pattern_match_cstr(&ps, " \n isto é assim", "%S*$") && capture_eq(ps.captures[0], "assim"));
    ASSERT_TRUE(pattern_match_cstr(&ps, " \n isto e assim", "[a-z]*$") && capture_eq(ps.captures[0], "assim"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "um caracter ? extra", "[^%sa-z]") && capture_eq(ps.captures[0], "?"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "", "a?") && capture_eq(ps.captures[0], ""));
    ASSERT_TRUE(pattern_match_cstr(&ps, "abl", "a?b?l?") && capture_eq(ps.captures[0], "abl"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "aa", "^aa?a?a") && capture_eq(ps.captures[0], "aa"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "0alo alo", "%x*") && capture_eq(ps.captures[0], "0a"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "alo alo", "%C+") && capture_eq(ps.captures[0], "alo alo"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "(álo)", "%(á") && capture_eq(ps.captures[0], "(á"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "(álo)", "%(á") && capture_eq(ps.captures[0], "(á"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "==", "^([=]*)=%1$") == PATTERN_NO_MATCH);
    ASSERT_TRUE(pattern_match_cstr(&ps, "===", "^([=]*)=%1$") && ps.capture_count == 2 &&
                capture_eq(ps.captures[0], "===") && capture_eq(ps.captures[1], "="));
    ASSERT_TRUE(pattern_match_cstr(&ps, "====", "^([=]*)=%1$") == PATTERN_NO_MATCH);
    ASSERT_TRUE(pattern_match_cstr(&ps, "==========", "^([=]*)=%1$") == PATTERN_NO_MATCH);
}

CTEST(pattern, nested_captures) {
    Pattern_State ps;
    ASSERT_TRUE(pattern_match_cstr(&ps, "clo alo", "^(((.).).* (%w*))$") && ps.capture_count == 5);
    ASSERT_TRUE(pattern_get_capture_pos(&ps, 1) == 0);
    ASSERT_TRUE(capture_eq(ps.captures[1], "clo alo"));
    ASSERT_TRUE(capture_eq(ps.captures[2], "cl"));
    ASSERT_TRUE(capture_eq(ps.captures[3], "c"));
    ASSERT_TRUE(capture_eq(ps.captures[4], "alo"));

    ASSERT_TRUE(pattern_match_cstr(&ps, "0123456789", "(.+(.?)())") && ps.capture_count == 4);
    ASSERT_TRUE(pattern_get_capture_pos(&ps, 1) == 0);
    ASSERT_TRUE(capture_eq(ps.captures[1], "0123456789"));
    ASSERT_TRUE(capture_eq(ps.captures[2], ""));
    ASSERT_TRUE(pattern_is_position_capture(&ps, 3) && pattern_get_capture_pos(&ps, 3) == 10);
}

CTEST(pattern, balanced_matches) {
    Pattern_State ps;

    ASSERT_TRUE(pattern_match_cstr(&ps, "(a(b)c)", "%b()") && capture_eq(ps.captures[0], "(a(b)c)"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "[a[b]c]", "%b[]") && capture_eq(ps.captures[0], "[a[b]c]"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "<a<b>c>", "%b<>") && capture_eq(ps.captures[0], "<a<b>c>"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "((()))", "%b()") && capture_eq(ps.captures[0], "((()))"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "(abc)def", "%b()def") && capture_eq(ps.captures[0], "(abc)def"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "x(a)y(b)z", "%b()") && capture_eq(ps.captures[0], "(a)"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "(abc", "%b()") == PATTERN_NO_MATCH);
    ASSERT_TRUE(pattern_match_cstr(&ps, "abc)", "%b()") == PATTERN_NO_MATCH);
    // Note: "(()" contains a valid balanced match "()" starting at position 1
    ASSERT_TRUE(pattern_match_cstr(&ps, "(()", "%b()") && capture_eq(ps.captures[0], "()"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "()", "%b()") && capture_eq(ps.captures[0], "()"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "text(abc)more", "(%b())") && ps.capture_count == 2);
    ASSERT_TRUE(capture_eq(ps.captures[1], "(abc)"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "{a{b}c}", "%b{}") && capture_eq(ps.captures[0], "{a{b}c}"));
}

CTEST(pattern, frontier_patterns) {
    Pattern_State ps;
    ASSERT_TRUE(pattern_match_cstr(&ps, "hello world", "%f[%w]hello") && capture_eq(ps.captures[0], "hello"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "hello world", "%f[%a]hello%f[%A]") && capture_eq(ps.captures[0], "hello"));
    ASSERT_TRUE(!pattern_match_cstr(&ps, "xhello", "%f[%w]hello"));
    ASSERT_TRUE(ps.error == PATTERN_NO_MATCH);
    ASSERT_TRUE(pattern_match_cstr(&ps, "abc123def", "%f[%d]%d+") && capture_eq(ps.captures[0], "123"));
    ASSERT_TRUE(pattern_match_cstr(&ps, " word ", "%f[%S]%w+%f[%s]") && capture_eq(ps.captures[0], "word"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "@word@", "%f[%S]%w+%f[%s]") == PATTERN_NO_MATCH);
    ASSERT_TRUE(pattern_match_cstr(&ps, "abc123", "%f[%d]") && capture_eq(ps.captures[0], ""));
    ASSERT_TRUE(pattern_get_capture_pos(&ps, 0) == 3);
    ASSERT_TRUE(pattern_match_cstr(&ps, "hello", "%f[%a]") && pattern_get_capture_pos(&ps, 0) == 0);
    ASSERT_TRUE(pattern_match_cstr(&ps, "hello", "hello%f[%z]") && capture_eq(ps.captures[0], "hello"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "abc:def", "%f[:]") && pattern_get_capture_pos(&ps, 0) == 3);
    ASSERT_TRUE(pattern_match_cstr(&ps, "hello123abc", "(%a+)") && capture_eq(ps.captures[1], "hello"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "hello123abc", "%f[^%d](%a+)") && capture_eq(ps.captures[1], "abc"));
}

CTEST(pattern, balanced_and_frontier_combined) {
    Pattern_State ps;
    ASSERT_TRUE(pattern_match_cstr(&ps, "text (abc) more", "%f[%(]%b()") && capture_eq(ps.captures[0], "(abc)"));
    ASSERT_TRUE(pattern_match_cstr(&ps, "func(arg1, (arg2))", "(%w+)%b()") && ps.capture_count == 2);
    ASSERT_TRUE(capture_eq(ps.captures[0], "func(arg1, (arg2))"));
    ASSERT_TRUE(capture_eq(ps.captures[1], "func"));
}

CTEST(pattern, errors) {
    Pattern_State ps;

    ASSERT_TRUE(pattern_match_cstr(&ps, "  a", "  (.") && ps.error == PATTERN_ERR_UNCLOSED_CAPTURE);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, " a", " .+)") && ps.error == PATTERN_ERR_UNEXPECTED_CAPTURE_CLOSE);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, "a", "[a") && ps.error == PATTERN_ERR_UNCLOSED_CLASS);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, "a", "[]") && ps.error == PATTERN_ERR_UNCLOSED_CLASS);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, "a", "[^]") && ps.error == PATTERN_ERR_UNCLOSED_CLASS);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, " a", " [a%]") && ps.error == PATTERN_ERR_UNCLOSED_CLASS);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, " a", " [a%") && ps.error == PATTERN_ERR_UNCLOSED_CLASS);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, "a", "%") && ps.error == PATTERN_ERR_INCOMPLETE_ESCAPE);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, "aaa", "(.)%1%2") && ps.error == PATTERN_ERR_INVALID_CAPTURE_IDX);
    pattern_print_error(stderr, &ps);
}


CTEST(pattern, balanced_errors) {
    Pattern_State ps;

    ASSERT_TRUE(pattern_match_cstr(&ps, "(abc)", "%b(") && ps.error == PATTERN_ERR_INVALID_BALANCED_PATTERN);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, "(abc)", "%b") && ps.error == PATTERN_ERR_INVALID_BALANCED_PATTERN);
    pattern_print_error(stderr, &ps);
}

CTEST(pattern, frontier_errors) {
    Pattern_State ps;

    ASSERT_TRUE(pattern_match_cstr(&ps, "hello", "%f") && ps.error == PATTERN_ERR_UNCLOSED_FRONTIER_PATTERN);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, "hello", "%f[%w") && ps.error == PATTERN_ERR_UNCLOSED_FRONTIER_PATTERN);
    pattern_print_error(stderr, &ps);

    ASSERT_TRUE(pattern_match_cstr(&ps, "hello", "%fx") && ps.error == PATTERN_ERR_UNCLOSED_FRONTIER_PATTERN);
    pattern_print_error(stderr, &ps);
}

