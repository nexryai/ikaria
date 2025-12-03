const browserWorker = new window.Worker(new URL("./worker-browser", import.meta.url), { type: 'module' });

type VideoInfo = {
    duration: number;
    width: number;
    height: number;
    videoCodec: string;
    audioCodec: string;
    keyframes: string[];
}

export async function getVideoInfo(file: File | string): Promise<VideoInfo> {
    if (!browserWorker) {
        throw new Error('This function can only be used in a browser environment.');
    }

    return new Promise((resolve, reject) => {
        browserWorker.onmessage = (e: MessageEvent) => {
            const data = e.data;
            if (data.error) {
                reject(data.error);
            } else {
                resolve(data);
            }
        };

        console.log('Worker: Sending file info request', file);
        browserWorker.postMessage(['get_video_info', file]);
    });
}

export async function trimVideo(file: File | string, startSec: string, endSec: string, tweakTimestamp: boolean = true): Promise<string | ArrayBuffer> {
    if (!browserWorker) {
        throw new Error('This function can only be used in a browser environment.');
    }

    return new Promise((resolve, reject) => {
        browserWorker.onmessage = (e: MessageEvent) => {
            const data = e.data;
            if (data.error) {
                reject(data.error);
            } else {
                resolve(data);
            }
        };

        browserWorker.postMessage(['trim_video', file, startSec, endSec, tweakTimestamp]);
    });
};
