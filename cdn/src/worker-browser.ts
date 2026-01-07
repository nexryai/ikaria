import Ikaria from './ikaria.js';

console.log('Worker: Starting worker');

self.onmessage = async (e: MessageEvent) => {
    try {
        console.log('Worker: Received message', e.data);
        const type = e.data[0];
        const file = e.data[1];

        const ikaria = await Ikaria();

        switch (type) {
            case 'read_file':

            case 'read_dir':

            case 'remux_to_dash':
                console.log("Starting....")
                // @ts-ignore
                const remuxer = new ikaria.DashRemuxer();

                try {
                    // @ts-ignore
                    const result = remuxer.process(file, `/out/manifest.mpd`, true)
                    if (result !== 0) {
                        postMessage({ error: "processor returned non-0 code" });
                    } else {
                        console.log("Done!");
                        postMessage({ result: "ok" });
                    }
                } finally {
                    remuxer.delete();
                }

            default:
                break;
        }
    } catch (error: any) {
        postMessage({ error: error.message });
    }
}

export default self;
