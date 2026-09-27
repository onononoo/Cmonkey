import { core } from './core.js';

// phase: 1 = document-start, 2 = document-end, 3 = document-idle (matches run_at in core.c)
async function inject(phase, { tabId, frameId, url }) {
  if (frameId !== 0) return; // ponytail: top frame only, add @noframes handling to run in iframes
  const c = await core;
  const { scripts = [] } = await browser.storage.local.get('scripts');
  for (const code of scripts) {
    if (c.runAt(url, code) !== phase) continue;
    browser.userScripts.execute({
      target: { tabId, frameIds: [0] }, js: [{ code }], world: 'MAIN', injectImmediately: true,
    }).catch(console.error);
  }
}

// ponytail: onCommitted is "close to" document-start, not guaranteed before page scripts.
// Switch to browser.userScripts.register if exact timing matters.
browser.webNavigation.onCommitted.addListener(d => inject(1, d));
browser.webNavigation.onDOMContentLoaded.addListener(d => inject(2, d));
browser.webNavigation.onCompleted.addListener(d => inject(3, d));
