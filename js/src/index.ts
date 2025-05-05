const worker = new Worker(new URL("./worker-browser.js", import.meta.url))

type Keyframe = {
    pts_time: number;
}

type VideoInfo = {
    duration: number;
    width: number;
    height: number;
    videoCodec: string;
    audioCodec: string;
    keyframes: Keyframe[];
}

export const getFileInfo = async (file: File): Promise<VideoInfo> => {
    return new Promise((resolve, reject) => {
        worker.onmessage = (e: MessageEvent) => {
            const data = e.data;
            if (data.error) {
                reject(data.error);
            } else {
                resolve(data);
            }
        };
        worker.postMessage(['get_file_info', file]);
    });
};