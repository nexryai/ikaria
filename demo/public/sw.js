// public/sw.js

const VIRTUAL_PATH = '/virtual-dash/';
const OPFS_DIR = 'out'; // ffmpegの出力先ディレクトリ

self.addEventListener('fetch', (event) => {
  const url = new URL(event.request.url);

  // 指定したパスへのリクエストのみをインターセプト
  if (url.pathname.startsWith(VIRTUAL_PATH)) {
    event.respondWith(handleOpfsRequest(url));
  }
});

async function handleOpfsRequest(url) {
  try {
    // URLからファイル名を抽出 (例: /virtual-dash/manifest.mpd -> manifest.mpd)
    const filename = url.pathname.replace(VIRTUAL_PATH, '');

    // OPFSのルートを取得
    const root = await navigator.storage.getDirectory();

    // /out ディレクトリを取得
    const dirHandle = await root.getDirectoryHandle(OPFS_DIR);

    // ファイルハンドルを取得
    const fileHandle = await dirHandle.getFileHandle(filename);

    // Fileオブジェクト(Blob)を取得
    const file = await fileHandle.getFile();

    // 適切なContent-Typeを設定
    const headers = new Headers();
    if (filename.endsWith('.mpd')) {
      headers.set('Content-Type', 'application/dash+xml');
    } else if (filename.endsWith('.webm')) {
      headers.set('Content-Type', 'video/webm');
    }

    // ファイルをレスポンスとして返す
    return new Response(file, {
      status: 200,
      headers: headers,
    });

  } catch (error) {
    console.error('OPFS Access Error:', error);
    return new Response('File not found in OPFS', { status: 404 });
  }
}
