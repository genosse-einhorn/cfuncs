#include "str-list.h"

static inline void
assert_str_list_equals_doublenull(StrList l, const char *dns)
{
    size_t i = 0, k = 0;
    while (dns[i]) {
        assert(k < str_list_length(l));

        assert(!strcmp((char*)l[k], &dns[i]));

        i += strlen(&dns[i]) + 1;
        k++;
    }

    assert(k == str_list_length(l));
}

static inline void
assert_str_list_equals(StrList l, size_t len, ...)
{
    assert(str_list_length(l) == len);

    va_list ap;
    va_start(ap, len);

    for (size_t i = 0; i < len; ++i) {
        const char *a = va_arg(ap, const char *);
        assert(!l[i] == !a);
        assert(str_equal(l[i], a));
    }

    va_end(ap);
}

static void
test_create(void)
{
    StrList l = NULL;

    const char *v[] = { "a", "b", "c", NULL };
    str_list_assign_strv(&l, (char **)v);
    assert_str_list_equals_doublenull(l, "a\0b\0c\0");

    const char dn_s[] = "a\0b\0c\0";
    char *dn_r = str_list_as_doublenull(l);
    assert(!memcmp(dn_r, dn_s, sizeof(dn_s)));
    str_clear(&dn_r);

    str_list_assign_doublenull(&l, "a1\0b1\0c1\0d2\0");
    assert(str_list_length(l) == 4);
    assert_str_list_equals_doublenull(l, "a1\0b1\0c1\0d2\0");

    str_list_assign_lines(&l, "a\r\nb\rc\r\r\nd\n\n\r\nf");
    assert_str_list_equals(l, 6, "a", "b\rc\r", "d", "", "", "f");

    str_list_clear(&l);
}

static void
test_split(void)
{
    StrList l = NULL;

    l = str_split("a,b,c", ",");
    assert(str_list_length(l) == 3);
    assert_str_list_equals_doublenull(l, "a\0b\0c\0");

    str_list_assign_split(&l, "", ",");
    assert(str_list_length(l) == 0);

    str_list_assign_split(&l, "abc", "_");
    assert_str_list_equals_doublenull(l, "abc\0");

    str_list_assign_split(&l, "abc", "");
    assert_str_list_equals_doublenull(l, "a\0b\0c\0");
    str_list_clear(&l);
}

static void
test_join(void)
{
    StrList l = NULL;
    char *s = NULL;

    s = str_joined(l, "hello");
    assert(!strcmp(s, ""));
    str_clear(&s);

    str_list_add(&l, "Hello, World!");
    s = str_joined(l, "hello");
    assert(str_equal(s, "Hello, World!"));
    str_clear(&s);
    str_list_clear(&l);

    str_list_assign_split(&l, "ab,bc,cd", ",");

    str_assign_joined(&s, l, NULL);
    assert(str_equal(s, "abbccd"));
    str_clear(&s);

    s = str_joined(l, ",;,");
    assert(str_equal(s, "ab,;,bc,;,cd"));
    str_clear(&s);

    str_list_clear(&l);
}

static void
test_env(void)
{
    StrList l = NULL;
    char *s = NULL;

    const char *v[] = {"FOO=bar", "fo=", "fi", "fi=baz", NULL};
    str_list_assign_strv(&l, (char **)v);

    s = str_list_env_value(l, "FOO");
    assert(str_equal(s, "bar"));
    str_clear(&s);

    s = str_list_env_value(l, "foo"); // FIXME! do we really want case insensitivity here?
    assert(str_equal(s, "bar"));

    str_assign_list_env_value(&s, l, "fo");
    assert(s && str_equal(s, ""));

    str_assign_list_env_value(&s, l, "fi");
    assert(s && str_equal(s, "baz"));

    str_assign_list_env_value(&s, l, "baz");
    assert(!s);

    str_list_set_env_value(&l, "fo", "bo");
    assert_str_list_equals_doublenull(l, "FOO=bar\0fo=bo\0fi\0fi=baz\0");
    s = str_list_env_value(l, "fo");
    assert(str_equal(s, "bo"));
    str_clear(&s);

    str_list_set_env_value(&l, "xx", "yy");
    assert_str_list_equals_doublenull(l, "FOO=bar\0fo=bo\0fi\0fi=baz\0xx=yy\0");
    s = str_list_env_value(l, "xx");
    assert(str_equal(s, "yy"));
    str_clear(&s);

    str_list_unset_env_value(&l, "fi");
    assert_str_list_equals_doublenull(l, "FOO=bar\0fo=bo\0fi\0xx=yy\0");

    str_list_unset_env_value(&l, "fi");
    assert_str_list_equals_doublenull(l, "FOO=bar\0fo=bo\0fi\0xx=yy\0");

    str_list_clear(&l);
}

int main(void)
{
    test_split();
    test_join();
    test_create();
    test_env();

    return 0;
}

