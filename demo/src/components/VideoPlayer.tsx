import React, { useEffect, useRef } from 'react';
import { MediaPlayer } from 'dashjs';

const VideoPlayer: React.FC = () => {
  const videoRef = useRef<HTMLVideoElement>(null);
  const playerRef = useRef<dashjs.MediaPlayerClass | null>(null);


  useEffect(() => {
    if (!videoRef.current) return;

    const url = '/virtual-dash/manifest.mpd';

    const player = MediaPlayer().create();
    player.initialize(videoRef.current, url, true);
    playerRef.current = player;

    player.on(MediaPlayer.events.ERROR, (e: any) => {
       console.error('Dash Playback Error:', e);
    });

    return () => {
      if (playerRef.current) {
        playerRef.current.destroy();
        playerRef.current = null;
      }
    };
  }, []);

  return (
    <div>
      <video
        ref={videoRef}
        controls
        playsInline
        style={{ width: '100%', maxWidth: '800px', backgroundColor: '#000' }}
      />
    </div>
  );
};

export default VideoPlayer;
