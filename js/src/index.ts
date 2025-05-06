const isNode = (): boolean => {
    return typeof process !== 'undefined' && process.versions != null && process.versions.node != null;
};

let WorkerThreads: any;
if (isNode()) {
    WorkerThreads = import('worker_threads').then((module) => {
        return module.Worker;
    });
}

const browserWorker = isNode() ? null : new window.Worker(new URL("./worker-browser", import.meta.url), { type: 'module' });
const nodeWorker = isNode() ? new WorkerThreads(new URL("./worker-node", import.meta.url)) : null

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

export const getVideoInfo = async (file: File): Promise<VideoInfo> => {
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
};

export const trimVideo = async (file: File): Promise<VideoInfo> => {
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
};

export const getVideoInfoFromPath = async (filePath: string): Promise<VideoInfo> => {
    if (!nodeWorker) {
        throw new Error('This function can only be used in a Node.js environment.');
    }

    return new Promise((resolve, reject) => {
        nodeWorker.on('message', (data: VideoInfo) => {
            resolve(data);
        });
        nodeWorker.postMessage({ type: 'get_video_info', filePath });
    });
}