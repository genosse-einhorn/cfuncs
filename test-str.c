#include "str.h"

#include <assert.h>
#include <string.h>
#include <limits.h>

static void
test_create(void)
{
    char *a = NULL;
    char *b = NULL;

    assert(str_length(a) == 0);

    str_assign(&a, "Hello, World!");
    assert(str_length(a) == strlen("Hello, World!"));
    assert(!strcmp(a, "Hello, World!"));

    str_assign(&b, a);
    assert(a != b);

    str_clear(&a);
    str_clear(&b);

    a = str_dup("Goodbye World");

    str_assign(&a, NULL);
    assert(!strcmp(a, ""));
    str_clear(&a);

    a = str_dup(NULL);
    assert(!strcmp(a, ""));
    str_clear(&a);
}

static void
test_append(void)
{
    char *a = NULL;
    char *b = NULL;
    char *c = NULL;

    str_append(&a, "Hello, ");
    assert(str_length(a) == 7);
    str_append(&a, "World!");

    str_assign(&b, "Hello, World!");

    assert(str_cmp(a, b) == 0);

    str_append(&a, b);
    assert(str_cmp(a, b) > 0);

    str_assign(&c, b);
    str_append(&b, "bla");
    assert(!strcmp(c, "Hello, World!"));
    assert(!strcmp(b, "Hello, World!bla"));

    str_clear(&a);
    str_clear(&b);
    str_clear(&c);
}

static void
test_compare(void)
{
    const char *a = "Hello";
    const char *b = "Hello World!";
    const char *c = "Goodbye World";

    assert(strcmp(a, b) < 0);
    assert(str_cmp(a, b) < 0);
    assert(strcmp(b, a) > 0);
    assert(str_cmp(b, a) > 0);

    assert(memcmp(a, b, 5) == 0);
    assert(str_cmp_buf(a, 5, b, 5) == 0);

    assert(str_cmp_buf(a, str_length(a), b, str_length(b)) < 0);

    assert(str_cmp(a, NULL) > 0);
    assert(str_cmp(NULL, NULL) == 0);
    assert(str_cmp(NULL, a) < 0);

    assert(str_cmp_buf(NULL, 0, NULL, 0) == 0);
    assert(str_cmp_buf(a, 1, NULL, 0) > 0);
    assert(str_cmp_buf(NULL, 0, a, 5) < 0);

    assert(str_equal_buf(a, 5, b, 5));
    assert(str_equal_buf(NULL, 0, NULL, 0));
    assert(!str_equal_buf(a, str_length(a), b, str_length(b)));
    assert(!str_equal_buf(a, 5, c, 5));
    assert(!str_equal_buf(NULL, 0, a, 5));
    assert(!str_equal_buf(b, 3, NULL, 0));
    assert(!str_equal(a, b));
    assert(str_equal(a, a));
    assert(str_equal(c, c));


    assert(str_natcmp("string", "sTrInG") == 0);
    assert(str_natcmp("  string", "sTrInG") == 0);
    assert(str_natcmp(" s000t\t  r", "s0t\vr  ") == 0);
    assert(str_natcmp(" st  r", "s0tr  ") != 0);
    assert(str_natcmp("2string", "03string") < 0);
    assert(str_natcmp("03string", "2string") > 0);
    assert(str_natcmp("str3ing", "str30ing") < 0);
    assert(str_natcmp("str30ing", "str3ing") > 0);
    assert(str_natcmp("string20", "string2") > 0);
    assert(str_natcmp("string2", "string20") < 0);
    assert(str_natcmp("string2", "string02") == 0);
    assert(str_natcmp("string", "string    \t\n") == 0);
    assert(str_natcmp("Vincent van Gogh", "Vincent vangogh") < 0);
    assert(str_natcmp("Vincent vangogh", "Vincent van Gogh") > 0);
    assert(str_natcmp("  ", NULL) == 0);
    assert(str_natcmp(NULL, "  ") == 0);
}

static void
test_printf(void)
{
    char *a = NULL;

    str_assign_printf(&a, "Hello, %s! %d", "World", 42);

    assert(str_length(a) == (int)strlen("Hello, World! 42"));
    assert(!strcmp(a, "Hello, World! 42"));

    str_assign_printf(&a, "%sHo ho h%c", a, (int)'i');
    assert(!strcmp(a, "Hello, World! 42Ho ho hi"));

    str_clear(&a);
    a = str_printf("Hello, %s", "World");
    assert(!strcmp(a, "Hello, World"));

    str_clear(&a);
}

static void
test_substr(void)
{
    char *a = str_dup("Hello, World!");
    char *b = NULL;

    // normal substr
    b = str_substr(a, 2, 5);
    assert(!str_cmp(b, "llo"));
    str_clear(&b);

    b = str_substr(a, -2, INT_MAX);
    assert(!strcmp(b, "d!"));

    str_assign_substr(&b, a, 0, 0);
    assert(!strcmp(b, ""));

    str_assign_substr(&b, a, 35235, 42);
    assert(!strcmp(b, ""));

    str_assign_substr(&b, a, -15, 1);
    assert(!strcmp(b, "H"));

    str_assign_substr(&b, NULL, 1, 2);
    assert(!strcmp(b, ""));

    str_assign_substr(&b, a, 1, 0);
    assert(!strcmp(b, ""));

    str_assign_substr(&b, a, -15, 4);
    assert(!strcmp(b, "Hell"));

    // inplace substr
    {
        char c[] = "Hello, World!";
        str_substr_inplace(c, 2, 5);
        assert(!strcmp(c, "llo"));
    }

    {
        char c[] = "Hello, World!";
        str_substr_inplace(c, -2, INT_MAX);
        assert(!strcmp(c, "d!"));
    }

    {
        char c[] = "Hello, World!";
        str_substr_inplace(c, 0, 0);
        assert(!strcmp(c, ""));
    }

    {
        char c[] = "Hello, World!";
        str_substr_inplace(c, 35235, 42);
        assert(!strcmp(c, ""));
    }

    {
        char c[] = "Hello, World!";
        str_substr_inplace(c, -15, 1);
        assert(!strcmp(c, "H"));
    }

    {
        char c[] = "Hello, World!";
        str_substr_inplace(c, 1, 0);
        assert(!strcmp(c, ""));
    }

    {
        char c[] = "Hello, World!";
        str_substr_inplace(c, -15, 4);
        assert(!strcmp(c, "Hell"));
    }


    assert(str_starts_with(a, "Hello"));
    assert(str_starts_with(a, NULL));
    assert(str_starts_with(a, ""));
    assert(str_starts_with(NULL, NULL));
    assert(str_starts_with(NULL, ""));
    assert(!str_starts_with(NULL, "Hello"));
    assert(str_starts_with(a, a));

    assert(str_ends_with(a, "World!"));
    assert(str_ends_with(a, NULL));
    assert(str_ends_with(a, ""));
    assert(str_ends_with(NULL, NULL));
    assert(str_ends_with(NULL, ""));
    assert(!str_ends_with(NULL, "Hello"));
    assert(str_ends_with(a, a));

    str_clear(&a);
    str_clear(&b);
}

static void
test_index_of(void)
{
    assert(str_index_of("Hello, World!", "lo, W") == 3);
    assert(str_index_of("Hello, World!", "gagawgkmag") == -1);
    assert(str_index_of("Hello, World!", NULL) == 0);
    assert(str_index_of(NULL, NULL) == 0);
    assert(str_index_of(NULL, "blabla") == -1);
    assert(str_index_of("Hello, World!", "World!OMG") == -1);
    assert(str_index_of("Hello", "llo") == 2);
    assert(str_index_of("Bla", "akfnalkfnvfaifoinaasavaeiven") == -1);

    const char *h = "Hello, World! World! World! Hello";
    assert(str_last_index_of(h, "Hello") == 28);
    assert(str_last_index_of(h, "Hello, ") == 0);
    assert(str_last_index_of(h, "ld!") == 24);
    assert(str_last_index_of(h, "BLA") == -1);
    assert(str_last_index_of(h, NULL) == 33);
    assert(str_last_index_of(NULL, NULL) == 0);
    assert(str_last_index_of(NULL, "Hello") == -1);
    assert(str_last_index_of("Hello", "Hello World!") == -1);
}

static void
test_modified(void)
{
    char *a = str_reversed("Hello");
    assert(!strcmp(a, "olleH"));
    str_assign_reversed(&a, "World");
    assert(!strcmp(a, "dlroW"));

    char b[] = "Goodbye";
    str_reverse_inplace(b);
    assert(!strcmp(b, "eybdooG"));

    char c[] = "abcd";
    str_reverse_inplace(c);
    assert(!strcmp(c, "dcba"));

    str_assign(&a, "Hello");

    char *r = str_replaced(a, "llo", "XXomg");
    assert(!strcmp(r, "HeXXomg"));

    str_assign(&a, "Hello Hello World!");
    str_assign_replaced(&r, a, "Hello", "Goodbye");
    assert(!strcmp(r, "Goodbye Goodbye World!"));

    str_assign_replaced(&r, a, "e", "ee");
    assert(!strcmp(r, "Heello Heello World!"));

    str_assign_replaced(&r, a, "wfakmawflawf", "ee");
    assert(!strcmp(r, "Hello Hello World!"));

    str_assign_replaced(&r, NULL, "e", "ee");
    assert(!strcmp(r, ""));

    str_assign_replaced(&r, "abc", NULL, "x");
    assert(!strcmp(r, "xaxbxcx"));

    str_assign_replaced(&r, NULL, NULL, "abc");
    assert(!strcmp(r, "abc"));

    str_assign_replaced(&r, "abc", "", "");
    assert(!strcmp(r, "abc"));

    str_assign_replaced(&r, "", "", "");
    assert(!strcmp(r, ""));

    str_clear(&a);
    str_clear(&r);
}

static void
test_left_pad(void)
{
    char *r = NULL;

    str_assign_left_padded(&r, "Hello", 2, ' ');
    assert(!strcmp(r, "Hello"));

    str_assign_left_padded(&r, "Hello", 10, ' ');
    assert(!strcmp(r, "     Hello"));

    str_assign_right_padded(&r, "Hello", 2, ' ');
    assert(!strcmp(r, "Hello"));

    str_assign_right_padded(&r, "Hello", 10, ' ');
    assert(!strcmp(r, "Hello     "));

    str_clear(&r);


    char buf[20] = "Hello";
    str_left_pad_inplace(buf, 2, ' ');
    assert(!strcmp(buf, "Hello"));

    str_left_pad_inplace(buf, 10, ' ');
    assert(!strcmp(buf, "     Hello"));

    strcpy(buf, "Hello");
    str_right_pad_inplace(buf, 2, ' ');
    assert(!strcmp(buf, "Hello"));

    str_right_pad_inplace(buf, 10, ' ');
    assert(!strcmp(buf, "Hello     "));
}

static void
test_trim(void)
{

    char *r = str_trimmed("   bla\t\f\v\n");
    assert(!strcmp(r, "bla"));

    str_assign_trimmed(&r, "xd");
    assert(!str_cmp(r, "xd"));

    str_assign_trimmed(&r, "   \r\n");
    assert(str_equal(r, ""));

    str_assign_trimmed(&r, "   x");
    assert(str_equal(r, "x"));

    str_assign_trimmed(&r, "x   ");
    assert(str_equal(r, "x"));

    str_assign_trimmed(&r, "");
    assert(str_equal(r, ""));

    str_assign_trimmed(&r, NULL);
    assert(str_equal(r, ""));

    str_clear(&r);


    char buf[20];
    strcpy(buf, "   bla\t\f\v\n");
    str_trim_inplace(buf);
    assert(!strcmp(buf, "bla"));

    strcpy(buf, "xd");
    str_trim_inplace(buf);
    assert(!str_cmp(buf, "xd"));

    strcpy(buf, "   \r\n");
    str_trim_inplace(buf);
    assert(str_equal(buf, ""));

    strcpy(buf, "   x");
    str_trim_inplace(buf);
    assert(str_equal(buf, "x"));

    strcpy(buf, "x   ");
    str_trim_inplace(buf);
    assert(str_equal(buf, "x"));

    strcpy(buf, "");
    str_trim_inplace(buf);
    assert(str_equal(buf, ""));
}

static void
test_case(void)
{
    char *r = str_uppercased("heLlO!- ");
    assert(str_equal(r, "HELLO!- "));

    str_assign_uppercased(&r, "HELLO");
    assert(str_equal(r, "HELLO"));

    char buf[] = "hEllxa€é’Ø";
    str_uppercase_inplace(buf);
    assert(str_equal(buf, "HELLXA€é’Ø"));

    str_clear(&r);

    r = str_lowercased("heLlO!- ");
    assert(str_equal(r, "hello!- "));

    str_assign_lowercased(&r, "hello");
    assert(str_equal(r, "hello"));

    strcpy(buf, "hEllxa€é’Ø");
    str_lowercase_inplace(buf);
    assert(str_equal(buf, "hellxa€é’Ø"));

    str_clear(&r);
}

int main(void)
{
    test_create();
    test_append();
    test_compare();
    test_printf();
    test_substr();
    test_index_of();
    test_modified();
    test_left_pad();
    test_trim();
    test_case();

    return 0;
}
