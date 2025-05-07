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


/**
 * Trims a video file and returns the path to the trimmed video.
 * This function is designed to be used in a browser environment.
 *
 * @param file - The video file to be trimmed.
 * @param startSec - The start time in seconds for trimming.
 * @param endSec - The end time in seconds for trimming.
 * @returns A promise that resolves to the path of the trimmed video as a blob URL.
 */
export function trimVideo(file: File, startSec: string, endSec: string, tweakTimestamp?: boolean): Promise<string> 

/**
 * Trims a video file and returns the resulting video as an ArrayBuffer.
 *
 * @param file - The path to the video file to be trimmed.
 * @param startSec - The start time in seconds for trimming.
 * @param endSec - The end time in seconds for trimming.
 * @returns A promise that resolves to an ArrayBuffer containing the trimmed video data.
 */
export function trimVideo(file: string, startSec: string, endSec: string, tweakTimestamp?: boolean): Promise<ArrayBuffer>;

export async function trimVideo(file: File | string, startSec: string, endSec: string, tweakTimestamp: boolean = true): Promise<string | ArrayBuffer> {
    if (typeof file === 'string') {
        if (!nodeWorker) {
            throw new Error('This function can only be used in a Node.js environment.');
        }

        return new Promise((resolve, reject) => {
            nodeWorker.on('message', (data: any) => {
                resolve(data);
            });
            nodeWorker.postMessage({ type: 'trim_video', filePath: file });
        });
    } else {
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
    }

    
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