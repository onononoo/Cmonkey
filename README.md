# Cmonkey

A tiny open-source userscript executor for Chrome. The core (userscript header parsing and `@match`/`@include`/`@exclude` URL matching) is freestanding C compiled to WebAssembly. JavaScript only calls the browser APIs.

## Build

Requires clang with the wasm32 target (e.g. `winget install LLVM.LLVM`):

```bash
clang --target=wasm32 -O2 -nostdlib -ffreestanding -Wl,--no-entry -o core.wasm core.c
```

Test the C core natively:

```bash
gcc test.c -o test && ./test
```

## Install

1. Open `chrome://extensions`, enable **Developer mode**, click **Load unpacked**, pick this folder.
2. On Cmonkey's details page, turn on **Allow User Scripts** (Chrome 138+).

## Use

Click the toolbar icon, paste a script:

- **Run on this tab** runs it once on the current page.
- **Save** stores it and auto-runs it on pages matching its header:

```js
// ==UserScript==
// @name    Hello
// @match   *://*.example.com/*
// @exclude *://example.com/private/*
// @run-at  document-end
// ==/UserScript==
alert('hi from Cmonkey');
```

Supported: `@name`, `@match`, `@exclude-match`, `@include`/`@exclude` (`*` globs), `@run-at`.
Not supported: `GM_*` APIs, `@require`, regex `@include`, iframes.

Scripts run in the page's main world with full access to it. Only run code you trust.

## Files

| File | Language | Role |
| --- | --- | --- |
| `core.c` | C | Header parser and URL matcher (built to `core.wasm`) |
| `test.c` | C | Native self-check |
| `core.js` | JS | Loads the wasm and passes strings in and out |
| `background.js` | JS | Injects saved scripts on navigation |
| `popup.html/js` | HTML/JS | Script editor, run button, saved list |

License: GPL-3.0
