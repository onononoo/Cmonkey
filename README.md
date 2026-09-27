# Cmonkey

A tiny open-source userscript executor for Chromium browsers (Chrome, Edge, Brave, Opera…) and Firefox. The core (userscript header parsing and `@match`/`@include`/`@exclude` URL matching) is freestanding C compiled to WebAssembly. JavaScript only calls the browser APIs.

## Build

Requires clang with the wasm32 target (e.g. `winget install LLVM.LLVM`):

```bash
sh build.sh
```

This produces `dist/chrome` and `dist/firefox`. Test the C core natively:

```bash
gcc test.c -o test && ./test
```

## Install

**Chromium (135+):** open `chrome://extensions`, enable **Developer mode**, click **Load unpacked**, pick `dist/chrome`. Then on Cmonkey's details page turn on **Allow User Scripts**.

**Firefox (153+):** open `about:debugging#/runtime/this-firefox`, click **Load Temporary Add-on**, pick `dist/firefox/manifest.json`. Open the Cmonkey popup and click **Allow Cmonkey to run user scripts**.

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
| `manifest.chrome.json`, `manifest.firefox.json` | JSON | Per-browser manifests (the only difference between builds) |
| `build.sh` | sh | Compiles the wasm and assembles `dist/` |

License: GPL-3.0
