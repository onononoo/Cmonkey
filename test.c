// Native self-check: gcc test.c -o test && ./test
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "core.c"

static int check(const char *url, const char *src) {
    int u = (int)strlen(url), s = (int)strlen(src);
    memcpy(buffer, url, u);
    memcpy(buffer + u, src, s);
    return run_at(u, s);
}

#define HDR(lines) "// ==UserScript==\n" lines "// ==/UserScript==\nalert(1)\n// @match *://*/*\n"

int main(void) {
    const char *ex = HDR("// @name  Example\r\n// @match *://*.example.com/*\n");
    assert(check("https://example.com/", ex) == 3);
    assert(check("http://a.b.example.com/x?y", ex) == 3);
    assert(check("https://badexample.com/", ex) == 0);
    assert(check("https://example.com.evil.io/", ex) == 0);
    assert(check("https://example.com@evil.io/", ex) == 0);
    assert(check("https://evil.io@example.com:8080/", ex) == 3);
    assert(check("ftp://example.com/", ex) == 0);

    assert(check("https://x.io/a/b", HDR("// @match https://x.io/a/*\n")) == 3);
    assert(check("https://x.io/c", HDR("// @match https://x.io/a/*\n")) == 0);
    assert(check("file:///C:/a.html", HDR("// @match <all_urls>\n")) == 3);

    const char *inc = HDR("// @include *\n// @exclude *secret*\n// @run-at document-start\n");
    assert(check("https://any.where/", inc) == 1);
    assert(check("https://any.where/secret", inc) == 0);
    assert(check("https://x.io/", HDR("// @match *://x.io/*\n// @run-at document-end\n")) == 2);

    assert(check("https://x.io/", "alert(1) // @match *://*/*") == 0);  // no header, no run
    assert(check("https://x.io/", HDR("")) == 0);                        // header without @match

    int n = (int)strlen(ex);
    memcpy(buffer, ex, n);
    int off = script_name(n);
    assert(result_len() == 7 && !memcmp(buffer + off, "Example", 7));

    puts("ok");
    return 0;
}
