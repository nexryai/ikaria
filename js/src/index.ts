const cdnBase = "https://ikaria-cloud-cdn.nexryai.workers.dev";
const workerUrl = `${cdnBase}/worker-browser.js`;

const workerCode = `import "${workerUrl}";`;
const dataUrl = `data:application/javascript;base64,${btoa(workerCode)}`;

const browserWorker = new Worker(dataUrl, {
  type: 'module',
  credentials: 'omit'
});

type VideoInfo = {
    duration: number;
    width: number;
    height: number;
    videoCodec: string;
    audioCodec: string;
    keyframes: string[];
}

export async function readFile(file: string): Promise<Uint8Array<any>> {
    if (!browserWorker) {
        throw new Error('This function can only be used in a browser environment.');
    }

    return new Promise((resolve, reject) => {
        browserWorker.onmessage = (e: MessageEvent) => {
            const data = e.data;
            if (data.error) {
                reject(data.error);
            } else if (data.file) {
                resolve(data.file);
            }
        };

        console.log('Worker: Sending read request', file);
        browserWorker.postMessage(['read_file', file]);
    });
}

export async function readDir(path: string): Promise<Array<string>> {
    if (!browserWorker) {
        throw new Error('This function can only be used in a browser environment.');
    }

    return new Promise((resolve, reject) => {
        browserWorker.onmessage = (e: MessageEvent) => {
            const data = e.data;
            if (data.error) {
                reject(data.error);
            } else if (data.files) {
                console.log(data)
                resolve(data.files);
            }
        };

        console.log('Worker: Sending read request', path);
        browserWorker.postMessage(['read_dir', path]);
    });
}

export async function remuxToDash(file: string): Promise<void> {
    if (!browserWorker) {
        throw new Error('This function can only be used in a browser environment.');
    }

    return new Promise((resolve, reject) => {
        browserWorker.onmessage = (e: MessageEvent) => {
            const data = e.data;
            if (data.error) {
                reject(data.error);
            } else {
                data?.result === "ok" ? resolve() : reject("Failed");
            }
        };

        console.log('Worker: Sending file info request', file);
        browserWorker.postMessage(['remux_to_dash', file]);
    });
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
