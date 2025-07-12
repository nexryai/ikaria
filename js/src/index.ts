const isNode = (): boolean => {
    //@ts-ignore
    return typeof process !== 'undefined' && process.versions != null && process.versions.node != null;
};

let WorkerThreads: any;
let nodeWorker: any = null;
let browserWorker: any = null;

if (isNode()) {
    //@ts-ignore
    import('worker_threads').then((module) => {
        WorkerThreads = module.Worker;
        nodeWorker = new WorkerThreads(new URL("./worker-node.js", import.meta.url));
    });
} else {
    browserWorker = new window.Worker(new URL("./worker-browser", import.meta.url), { type: 'module' });
}

type VideoInfo = {
    duration: number;
    width: number;
    height: number;
    videoCodec: string;
    audioCodec: string;
    keyframes: string[];
}

/**
 * Gets video information from a File object.
 * This function is designed to be used in a browser environment.
 *
 * @param file - The video file to get information from.
 * @returns A promise that resolves to the video information.
 */
export function getVideoInfo(file: File): Promise<VideoInfo>;

/**
 * Gets video information from a file path.
 * This function is designed to be used in a Node.js environment.
 *
 * @param filePath - The path to the video file.
 * @returns A promise that resolves to the video information.
 */
export function getVideoInfo(filePath: string): Promise<VideoInfo>;

export async function getVideoInfo(file: File | string): Promise<VideoInfo> {
    if (typeof file === 'string') {
        if (!nodeWorker) {
            throw new Error('This function can only be used in a Node.js environment.');
        }

        return new Promise((resolve, reject) => {
            nodeWorker.on('message', (data: any) => {
                if (data.error) {
                    reject(data.error);
                } else {
                    resolve(data);
                }
            });
            nodeWorker.postMessage({ type: 'get_video_info', filePath: file });
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

            console.log('Worker: Sending file info request', file);
            browserWorker.postMessage(['get_video_info', file]);
        });
    }
}

/**
 * Trims a video file and returns the path to the trimmed video.
 * This function is designed to be used in a browser environment.
 *
 * @param file - The video file to be trimmed.
 * @param startSec - The start time in seconds for trimming.
 * @param endSec - The end time in seconds for trimming.
 * @param tweakTimestamp - Optional parameter to adjust the timestamp of the trimmed video.
 * @returns A promise that resolves to the path of the trimmed video as a blob URL.
 */
export function trimVideo(file: File, startSec: string, endSec: string, tweakTimestamp?: boolean): Promise<string>

/**
 * Trims a video file and returns the resulting video as an ArrayBuffer.
 *
 * @param file - The path to the video file to be trimmed.
 * @param startSec - The start time in seconds for trimming.
 * @param endSec - The end time in seconds for trimming.
 * @param tweakTimestamp - Optional parameter to adjust the timestamp of the trimmed video.
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

export const resizeImageToWebP = async (file: File | string, resizeWidth: number, resizeHeight: number): Promise<string | ArrayBuffer> => {
    if (typeof file === 'string') {
        if (!nodeWorker) {
            throw new Error('This function can only be used in a Node.js environment.');
        }

        return new Promise((resolve, reject) => {
            nodeWorker.on('message', (data: ArrayBuffer) => {
                resolve(data);
            });
            nodeWorker.postMessage({ type: 'reseize_image', filePath: file, resizeWidth, resizeHeight });
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

            browserWorker.postMessage(['reseize_image', file, resizeWidth, resizeHeight]);
        });
    }
}
