import React, { useEffect, useState } from 'react';
import { remuxToDash } from '../../js/dist';
import VideoPlayer from './components/VideoPlayer';

const requestPersistentStorage = async (): Promise<boolean> => {
  if (navigator.storage && navigator.storage.persist) {
    const isAlreadyPersisted = await navigator.storage.persisted();
    if (isAlreadyPersisted) {
      return true;
    }
    const isPersisted = await navigator.storage.persist();
    return isPersisted;
  }
  return false;
};

const DashConverterPage: React.FC = () => {
  const [isProcessing, setIsProcessing] = useState(false);
  const [statusMessage, setStatusMessage] = useState<string>('Please upload a file');
  const [playerKey, setPlayerKey] = useState<number>(0);
  const [showPlayer, setShowPlayer] = useState(false);

  useEffect(() => {
    const initSw = async () => {
      if ('serviceWorker' in navigator) {
        try {
          const registration = await navigator.serviceWorker.register('/sw.js');

          if (registration.installing) {
            await new Promise<void>(resolve => {
              registration.installing?.addEventListener('statechange', (e: any) => {
                if (e.target.state === 'activated') resolve();
              });
            });
          }
          await navigator.serviceWorker.ready;
        } catch (error) {
          console.error('SW registration failed:', error);
        }
      }
    };
    initSw();
  }, []);

  const handleFileChange = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const files = event.target.files;
    if (!files || files.length === 0) return;

    const storageGranted = await requestPersistentStorage();
    if (!storageGranted) {
      console.warn("Persistent storage was not allowed.");
    }

    const selectedFile = files[0];
    setIsProcessing(true);
    setStatusMessage('Writing to OPFS...');
    setShowPlayer(false);

    try {
      const opfsRoot = await navigator.storage.getDirectory();
      const fileHandle = await opfsRoot.getFileHandle(selectedFile.name, { create: true });

      const writable = await fileHandle.createWritable();
      await writable.write(selectedFile);
      await writable.close();

      setStatusMessage('Converting (remuxToDash)...');
      await remuxToDash(`/opfs/${selectedFile.name}`);

      setStatusMessage('Conversion complete. Ready to play.');
      setShowPlayer(true);
      setPlayerKey(prev => prev + 1);

    } catch (error) {
      console.error('Processing Error:', error);
      setStatusMessage(`An error occurred: ${error}`);
    } finally {
      setIsProcessing(false);
      event.target.value = '';
    }
  };

  const clearOpfs = async () => {
    if (!window.confirm('Are you sure you want to delete all data in OPFS?')) return;

    try {
      const root = await navigator.storage.getDirectory();
      // @ts-ignore
      for await (const name of root.keys()) {
        await root.removeEntry(name, { recursive: true });
      }

      setStatusMessage('OPFS has been cleared');
      setShowPlayer(false);
    } catch (error) {
      console.error('Clear OPFS Error:', error);
      setStatusMessage('Failed to delete OPFS content');
    }
  };

  return (
    <div style={{ padding: '20px', fontFamily: 'sans-serif' }}>
      <h1>WebM to DASH Converter (OPFS)</h1>

      <div style={{ marginBottom: '20px', padding: '15px', border: '1px solid #ccc', borderRadius: '8px' }}>
        <h3>1. Select File to Convert</h3>
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
          <h3>2. Preview Playback</h3>
          <VideoPlayer key={playerKey} />
        </div>
      )}

      <div style={{ marginTop: '40px', borderTop: '1px solid #eee', paddingTop: '10px' }}>
        <h3>Debug Tools</h3>
        <button
          onClick={clearOpfs}
          style={{
            backgroundColor: '#ff4444',
            color: 'white',
            padding: '8px 16px',
            border: 'none',
            borderRadius: '4px',
            cursor: 'pointer'
          }}
        >
          Clear All OPFS Content
        </button>
      </div>
    </div>
  );
};

export default DashConverterPage;
