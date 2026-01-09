import Ikaria from './ikaria.js';

console.log('Worker: Starting worker');

self.onmessage = async (e: MessageEvent) => {
    try {
        console.log('Worker: Received message', e.data);
        const type = e.data[0];
        const file = e.data[1];

        switch (type) {
            case 'read_file':

            case 'read_dir':

            case 'remux_to_dash':
                console.log("Starting....")

                const result = await new Promise<number>((resolve, reject) => {
                    Ikaria({
                        arguments: ['remuxToDash', file, '/out/manifest.mpd'],
                        onExit: (code: number) => {
                            resolve(code);
                        }
                    }).catch(reject);
                });

                if (result !== 0) {
                    postMessage({ error: "processor returned non-0 code" });
                } else {
                    console.log("Done!");
                    postMessage({ result: "ok" });
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
