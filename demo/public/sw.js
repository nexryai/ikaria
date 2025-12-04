const VIRTUAL_PATH = '/virtual-dash/';
const OPFS_DIR = 'out';

self.addEventListener('fetch', (event) => {
  const url = new URL(event.request.url);

  if (url.pathname.startsWith(VIRTUAL_PATH)) {
    event.respondWith(handleOpfsRequest(url));
  }
});

async function handleOpfsRequest(url) {
  try {
    const filename = url.pathname.replace(VIRTUAL_PATH, '');

    const root = await navigator.storage.getDirectory();
    const dirHandle = await root.getDirectoryHandle(OPFS_DIR);
    const fileHandle = await dirHandle.getFileHandle(filename);

    const file = await fileHandle.getFile();

    const headers = new Headers();
    if (filename.endsWith('.mpd')) {
      headers.set('Content-Type', 'application/dash+xml');
    } else if (filename.endsWith('.webm')) {
      headers.set('Content-Type', 'video/webm');
    }

    return new Response(file, {
      status: 200,
      headers: headers,
    });

  } catch (error) {
    console.error('OPFS Access Error:', error);
    return new Response('File not found in OPFS', { status: 404 });
  }
}
