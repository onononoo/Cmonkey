import { core } from './core.js';

const $ = id => document.getElementById(id);
// userScripts is an optional permission; the namespace is missing until it's granted from a click.
$('grant').hidden = !!browser.userScripts;
$('grant').onclick = () => browser.permissions.request({ permissions: ['userScripts'] }).then(ok => ok && location.reload());

$('run').onclick = async () => {
  const [tab] = await browser.tabs.query({ active: true, currentWindow: true });
  browser.userScripts.execute({ target: { tabId: tab.id }, js: [{ code: $('code').value }], world: 'MAIN' })
    .catch(e => alert(e.message));
};

$('save').onclick = async () => {
  const { scripts = [] } = await browser.storage.local.get('scripts');
  scripts.push($('code').value);
  await browser.storage.local.set({ scripts });
  render();
};

async function render() {
  const c = await core;
  const { scripts = [] } = await browser.storage.local.get('scripts');
  $('list').replaceChildren(...scripts.map((code, i) => {
    const li = document.createElement('li'), name = document.createElement('a'), del = document.createElement('button');
    name.textContent = c.name(code) || 'Untitled';
    name.href = '#';
    name.onclick = () => { $('code').value = code; };
    del.textContent = 'Delete';
    del.onclick = async () => {
      scripts.splice(i, 1);
      await browser.storage.local.set({ scripts });
      render();
    };
    li.append(name, del);
    return li;
  }));
}
render();
