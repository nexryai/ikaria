import React, { useState } from 'react';
import { remuxToDash } from '../../js/src';
import VideoPlayer from './components/VideoPlayer';
const DashConverterPage: React.FC = () => {
  const [isProcessing, setIsProcessing] = useState(false);
  const [statusMessage, setStatusMessage] = useState<string>('ファイルをアップロードしてください');
  const [playerKey, setPlayerKey] = useState<number>(0);
  const [showPlayer, setShowPlayer] = useState(false);

  const handleFileChange = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const files = event.target.files;
    if (!files || files.length === 0) return;

    const selectedFile = files[0];
    setIsProcessing(true);
    setStatusMessage('OPFSへ書き込み中...');
    setShowPlayer(false);

    try {
      const opfsRoot = await navigator.storage.getDirectory();

      const fileHandle = await opfsRoot.getFileHandle(selectedFile.name, { create: true });

      const writable = await fileHandle.createWritable();
      await writable.write(selectedFile);
      await writable.close();

      console.log("Written to OPFS via JS");

      setStatusMessage('変換中 (remuxToDash)...');

      await remuxToDash(`/opfs/${selectedFile.name}`);

      setStatusMessage('変換完了。再生準備OK');
      setShowPlayer(true);
      setPlayerKey(prev => prev + 1);

    } catch (error) {
      console.error('Processing Error:', error);
      setStatusMessage(`エラーが発生しました: ${error}`);
    } finally {
      setIsProcessing(false);
      event.target.value = '';
    }
  };

  const clearOpfs = async () => {
    if (!window.confirm('OPFS内の全データを削除しますか？')) return;

    try {
      const root = await navigator.storage.getDirectory();
      // @ts-ignore
      for await (const name of root.keys()) {
        await root.removeEntry(name, { recursive: true });
      }

      console.log('OPFS Cleared');
      setStatusMessage('OPFSを空にしました');
      setShowPlayer(false);
    } catch (error) {
      console.error('Clear OPFS Error:', error);
      setStatusMessage('削除に失敗しました');
    }
  };

  return (
    <div style={{ padding: '20px', fontFamily: 'sans-serif' }}>
      <h1>WebM to DASH Converter (OPFS)</h1>

      <div style={{ marginBottom: '20px', padding: '15px', border: '1px solid #ccc', borderRadius: '8px' }}>
        <h3>1. ファイルを選択して変換</h3>
        <input
          type="file"
          accept="video/webm,video/mp4"
          onChange={handleFileChange}
          disabled={isProcessing}
        />
        <p style={{ color: isProcessing ? 'blue' : 'black' }}>
          <strong>Status:</strong> {statusMessage}
        </p>
      </div>

      {showPlayer && (
        <div style={{ marginBottom: '20px' }}>
          <h3>2. プレビュー再生</h3>
          <VideoPlayer key={playerKey} />
        </div>
      )}

      <div style={{ marginTop: '40px', borderTop: '1px solid #eee', paddingTop: '10px' }}>
        <h3>Debug Tools</h3>
        <button
          onClick={clearOpfs}
          style={{ backgroundColor: '#ff4444', color: 'white', padding: '8px 16px', border: 'none', borderRadius: '4px', cursor: 'pointer' }}
        >
          OPFSの中身をすべて削除
        </button>
      </div>
    </div>
  );
};

export default DashConverterPage;
