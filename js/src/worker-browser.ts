import Ikaria from './ikaria-wasm.js';

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
                ikaria.remuxToDash(file, `/out/manifest.mpd`)
                console.log("Done!");
                postMessage({ result: "ok" });

            default:
                break;
        }
    } catch (error: any) {
        postMessage({ error: error.message });
    }
}

export default self;
