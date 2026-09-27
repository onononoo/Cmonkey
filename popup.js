import { core } from './core.js';

const $ = id => document.getElementById(id);
try { chrome.userScripts.getScripts(); } catch { $('warn').hidden = false; } // throws until the toggle is on

$('run').onclick = async () => {
  const [tab] = await chrome.tabs.query({ active: true, currentWindow: true });
  chrome.userScripts.execute({ target: { tabId: tab.id }, js: [{ code: $('code').value }], world: 'MAIN' })
    .catch(e => alert(e.message));
};

$('save').onclick = async () => {
  const { scripts = [] } = await chrome.storage.local.get('scripts');
  scripts.push($('code').value);
  await chrome.storage.local.set({ scripts });
  render();
};

async function render() {
  const c = await core;
  const { scripts = [] } = await chrome.storage.local.get('scripts');
  $('list').replaceChildren(...scripts.map((code, i) => {
    const li = document.createElement('li'), name = document.createElement('a'), del = document.createElement('button');
    name.textContent = c.name(code) || 'Untitled';
    name.href = '#';
    name.onclick = () => { $('code').value = code; };
    del.textContent = 'Delete';
    del.onclick = async () => {
      scripts.splice(i, 1);
      await chrome.storage.local.set({ scripts });
      render();
    };
    li.append(name, del);
    return li;
  }));
}
render();
