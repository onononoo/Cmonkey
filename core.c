// Cmonkey core: userscript metadata parsing and URL matching.
// Freestanding C (no libc) so it compiles straight to WebAssembly.
// JS writes strings into buffer, calls an export, reads the result.

#ifdef __wasm__
#define EXPORT(name) __attribute__((export_name(name)))
#else
#define EXPORT(name)
#endif

#define BUF_SIZE (1 << 20)
static char buffer[BUF_SIZE];
static int out_len;

EXPORT("buf") char *buf(void) { return buffer; }
EXPORT("buf_size") int buf_size(void) { return BUF_SIZE; }
EXPORT("result_len") int result_len(void) { return out_len; }

typedef struct { const char *p; int n; } str;

static str S(const char *c) { int n = 0; while (c[n]) n++; return (str){c, n}; }
static str sub(str s, int from, int to) { return (str){s.p + from, to - from}; }
static int space(char c) { return c == ' ' || c == '\t' || c == '\r'; }

static int same(str a, str b) {
    if (a.n != b.n) return 0;
    for (int i = 0; i < a.n; i++) if (a.p[i] != b.p[i]) return 0;
    return 1;
}
static int starts(str a, str b) { return a.n >= b.n && same(sub(a, 0, b.n), b); }
static int ends(str a, str b) { return a.n >= b.n && same(sub(a, a.n - b.n, a.n), b); }

// Index of b in a, or -1.
static int find(str a, str b) {
    for (int i = 0; i + b.n <= a.n; i++) if (same(sub(a, i, i + b.n), b)) return i;
    return -1;
}

// Index of c in s at or after from, or s.n.
static int idx(str s, char c, int from) {
    while (from < s.n && s.p[from] != c) from++;
    return from;
}

static str trim(str s) {
    while (s.n && space(*s.p)) s.p++, s.n--;
    while (s.n && space(s.p[s.n - 1])) s.n--;
    return s;
}

// '*' matches any run of characters, everything else is literal.
static int glob(str p, str s) {
    int i = 0, j = 0, star = -1, mark = 0;
    while (j < s.n) {
        if (i < p.n && p.p[i] == '*') star = i++, mark = j;
        else if (i < p.n && p.p[i] == s.p[j]) i++, j++;
        else if (star >= 0) i = star + 1, j = ++mark;
        else return 0;
    }
    while (i < p.n && p.p[i] == '*') i++;
    return i == p.n;
}

// Chrome-style match pattern: scheme://host/path, e.g. *://*.example.com/*
static int match_pattern(str pat, str url) {
    if (same(pat, S("<all_urls>"))) return 1;
    int ps = find(pat, S("://")), us = find(url, S("://"));
    if (ps < 0 || us < 0) return 0;

    str pscheme = sub(pat, 0, ps), uscheme = sub(url, 0, us);
    if (same(pscheme, S("*")) ? !same(uscheme, S("http")) && !same(uscheme, S("https"))
                              : !same(pscheme, uscheme)) return 0;

    int pp = idx(pat, '/', ps + 3), up = idx(url, '/', us + 3);
    str phost = sub(pat, ps + 3, pp), uhost = sub(url, us + 3, up);
    int at = uhost.n;
    while (at > 0 && uhost.p[at - 1] != '@') at--;      // drop user:pass@
    uhost = sub(uhost, at, uhost.n);
    uhost = sub(uhost, 0, idx(uhost, ':', 0));          // drop :port

    if (starts(phost, S("*."))) {
        str base = sub(phost, 2, phost.n), dot = sub(phost, 1, phost.n);
        if (!same(uhost, base) && !ends(uhost, dot)) return 0;
    } else if (!same(phost, S("*")) && !same(phost, uhost)) return 0;

    return glob(sub(pat, pp, pat.n), sub(url, up, url.n));
}

// Everything after the "==UserScript==" line (empty if there is none).
static str header(str src) {
    int i = find(src, S("==UserScript=="));
    return i < 0 ? sub(src, 0, 0) : sub(src, idx(src, '\n', i), src.n);
}

// Pops the next "// @key value" line off *src; stops at "==/UserScript==".
static int next_meta(str *src, str *key, str *val) {
    while (src->n > 0) {
        int e = idx(*src, '\n', 0);
        str line = trim(sub(*src, 0, e));
        *src = sub(*src, e < src->n ? e + 1 : e, src->n);
        if (find(line, S("==/UserScript==")) >= 0) break;
        if (!starts(line, S("//"))) continue;
        line = trim(sub(line, 2, line.n));
        if (!starts(line, S("@"))) continue;
        int k = 1;
        while (k < line.n && !space(line.p[k])) k++;
        *key = sub(line, 1, k);
        *val = trim(sub(line, k, line.n));
        return 1;
    }
    src->n = 0;
    return 0;
}

// buffer = url bytes followed by script bytes.
// Returns 0 = don't run, 1 = document-start, 2 = document-end, 3 = document-idle.
EXPORT("run_at") int run_at(int url_len, int src_len) {
    str url = {buffer, url_len}, src = header((str){buffer + url_len, src_len}), k, v;
    int hit = 0, when = 3;
    while (next_meta(&src, &k, &v)) {
        if (same(k, S("match")) && match_pattern(v, url)) hit = 1;
        else if (same(k, S("include")) && glob(v, url)) hit = 1;
        else if (same(k, S("exclude-match")) && match_pattern(v, url)) return 0;
        else if (same(k, S("exclude")) && glob(v, url)) return 0;
        else if (same(k, S("run-at")))
            when = same(v, S("document-start")) ? 1 : same(v, S("document-end")) ? 2 : 3;
    }
    return hit ? when : 0;
}

// buffer = script bytes. Returns offset of @name in buffer, length via result_len().
EXPORT("script_name") int script_name(int src_len) {
    str src = header((str){buffer, src_len}), k, v;
    while (next_meta(&src, &k, &v))
        if (same(k, S("name"))) { out_len = v.n; return (int)(v.p - buffer); }
    out_len = 0;
    return 0;
}
