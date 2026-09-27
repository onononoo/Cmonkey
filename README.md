# Cmonkey

A tiny open-source userscript executor for Firefox. The core (userscript header parsing and `@match`/`@include`/`@exclude` URL matching) is freestanding C compiled to WebAssembly. JavaScript only calls the browser APIs.

## Build

Requires clang with the wasm32 target (e.g. `winget install LLVM.LLVM`):

```bash
sh build.sh
```

This produces:

| Output | For |
| --- | --- |
| `dist/Cmonkey.xpi` | Firefox install file |
| `dist/firefox` | Unpacked folder for development and signing |
| `dist/Cmonkey-source.zip` | Source upload for Mozilla review |

Test the C core natively:

```bash
gcc test.c -o test && ./test
```

## Install (Firefox 153+)

Open the signed [`release/Cmonkey-0.1.1.xpi`](release/Cmonkey-0.1.1.xpi) in Firefox (drag it into a window, or File > Open File), click **Add**, then open the Cmonkey popup and click **Allow Cmonkey to run user scripts**.

To sign a new build yourself (release Firefox only installs **signed** `.xpi` files; bump `version` in `manifest.json` first). Signing is free and needs a Mozilla account:

1. Create API keys at https://addons.mozilla.org/developers/addon/api/key/
2. Run (keys are read from the environment):

   ```bash
   read -p "JWT issuer: " WEB_EXT_API_KEY; read -sp "JWT secret: " WEB_EXT_API_SECRET; echo; export WEB_EXT_API_KEY WEB_EXT_API_SECRET; npx web-ext sign --channel=unlisted -s dist/firefox -a dist --upload-source-code dist/Cmonkey-source.zip
   ```

   The signed `.xpi` lands in `dist/` and installs on any Firefox.

Unsigned builds install on Firefox Developer Edition / Nightly after setting `xpinstall.signatures.required` to `false` in `about:config`, or temporarily on any Firefox via `about:debugging#/runtime/this-firefox` > **Load Temporary Add-on** (removed on restart).

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
| `manifest.json` | JSON | Extension manifest |
| `build.sh` | sh | Compiles the wasm and assembles `dist/` |

License: GPL-3.0
