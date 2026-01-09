import Ikaria from './ikaria.js';

console.log('Worker: Starting worker');

const ikariaPromise = Ikaria();

self.onmessage = async (e: MessageEvent) => {
    try {
        console.log('Worker: Received message', e.data);
        const type = e.data[0];
        const file = e.data[1];

        const ikaria = await ikariaPromise;

        switch (type) {
            case 'read_file':

            case 'read_dir':

            case 'remux_to_dash':
                console.log("Starting....")

                try {
                    // @ts-ignore
                    const result = await ikaria.remuxToDash(file, `/out/manifest.mpd`);
                    if (result !== 0) {
                        postMessage({ error: "processor returned non-0 code" });
                    } else {
                        console.log("Done!");
                        postMessage({ result: "ok" });
                    }
                }

            default:
                break;
        }
    } catch (error: any) {
        console.error(error);
        postMessage({ error: error.message });
    }
}

export default self;
