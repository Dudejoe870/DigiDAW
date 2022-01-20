export class Settings extends Element {
    app;

    currentPage;

    supportedAPIs = [];

    devices = [];
    supportedSampleRates = [];

    testTone = false;

    Pages = Object.freeze({
        AudioEngine: 0,
        MidiEngine: 1
    });

    this(props, kids) {
        this.app = props.app;
        this.currentPage = -1;
        this.supportedAPIs = AudioEngine.getSupportedAPIs();
    }

    componentWillUnmount() {
        if (this.testTone) {
            AudioEngine.pause();
        }
    }

    getAPIDropdown() {
        var html = "";

        for (const api of this.supportedAPIs) {
            html += `<option id="api-${api}" ${api == AudioEngine.getCurrentAPI() ? "selected" : ""}>${AudioEngine.getAPIDisplayName(api)}</option>`;
        }

        return html;
    }

    getOutputDeviceDropdown() {
        var html = "";

        html += `<option class="device--1 lang-None" ${AudioEngine.outputDevice == -1 ? "selected" : ""}></option>`;

        for (const device of this.devices) {
            if (device.probed === true && device.outputChannels > 0) {
                html += `<option class="device-${device.index}" ${device.index == AudioEngine.outputDevice ? "selected" : ""}>${device.name}</option>`;
            }
        }

        return html;
    }

    getInputDeviceDropdown() {
        var html = "";

        html += `<option class="device--1 lang-None" ${AudioEngine.inputDevice == -1 ? "selected" : ""}></option>`;

        for (const device of this.devices) {
            if (device.probed === true && device.inputChannels > 0) {
                html += `<option class="device-${device.index}" ${device.index == AudioEngine.inputDevice ? "selected" : ""}>${device.name}</option>`;
            }
        }

        return html;
    }

    getSampleRateDropdown() {
        var html = "";

        if (this.supportedSampleRates != undefined) {
            for (const rate of this.supportedSampleRates) {
                html += `<option ${rate == AudioEngine.sampleRate ? "selected" : ""}>${rate}</option>`;
            }
        }

        return html;
    }

    getBufferSizeDropdown() {
        var html = "";

        for (let size = 64; size <= 4096; size *= 2) {
            html += `<option id="buffer-${size}" ${AudioEngine.bufferSize == size ? "selected" : ""}>${size} <span class="lang-SettingsSamples"></span></option>`;
        }

        return html;
    }

    getDeviceInfoForDevice(device) {
        var html = "";

        if (device > 0) {
            html += `<span class="lang-DeviceOutputChannels"></span>: ${this.devices[device].outputChannels}<br />`;
            html += `<span class="lang-DeviceInputChannels"></span>: ${this.devices[device].inputChannels}<br />`;
            html += `<span class="lang-DevicePreferredSampleRate"></span>: ${this.devices[device].preferredSampleRate}<br />`;
        }
        else {
            html += `<span class="lang-SettingsNoDeviceSelected"></span>`;
        }

        return html;
    }

    getDeviceInfoSection() {
        var html = "";

        html += `<h3 class="lang-SettingsOutputDevice"></h3>`;
        html += this.getDeviceInfoForDevice(AudioEngine.outputDevice);
        html += `<h3 class="lang-SettingsInputDevice"></h3>`;
        html += this.getDeviceInfoForDevice(AudioEngine.inputDevice);

        return html;
    }

    getTestOutputButton() {
        var html = `<button id="test-button" style="display: inline-block; position: absolute;" class="lang-SettingsTestButton"></button>`;

        return html;
    }

    getAudioEnginePage() {
        this.devices = AudioEngine.queryDevices();
        this.supportedSampleRates = AudioEngine.getSupportedSampleRates();

        return <section style="flow: vertical; height: *; width: *; margin: 32px; margin-top: 0; margin-bottom: 0; padding: 0; text-align: center; align-items: center; justify-content: center;">
            <h1 class="lang-AudioEngineSettings"></h1>
            <section style="flow: vertical; text-align: center; horizontal-align: center; width: *;">
                <div>
                    <label class="lang-SettingsAPIBackend"></label>
                    <select type="dropdown" id="api-dropdown" state-html={ this.getAPIDropdown() }>
                    </select>
                </div>
                <hr />
                
                <h2 class="lang-DeviceSettings"></h2>
                <div>
                    <label class="lang-SettingsOutputDevice"></label>
                    <select type="dropdown" id="output-dropdown" state-html={ this.getOutputDeviceDropdown() }>
                    </select>
                    <span state-html={this.getTestOutputButton()}></span>
                </div>
                <br />

                <div>
                    <label class="lang-SettingsInputDevice"></label>
                    <select type="dropdown" id="input-dropdown" state-html={ this.getInputDeviceDropdown() }>
                    </select>
                </div>
                <br />

                <div>
                    <label class="lang-SettingsSampleRate"></label>
                    <select type="dropdown" id="samplerate-dropdown" state-html={ this.getSampleRateDropdown() }>
                    </select>
                </div>
                <br />

                <div>
                    <label class="lang-SettingsBufferSize"></label>
                    <select type="dropdown" id="buffersize-dropdown" state-html={ this.getBufferSizeDropdown() }>
                    </select>
                </div>
                <hr />

                <h2 class="lang-DeviceInfo"></h2>
                <section state-html={ this.getDeviceInfoSection() }>
                </section>
            </section>
        </section>;
    }

    midiEnginePage = <section>
        [WIP]
    </section>;

    getCurrentPage() {
        if (this.currentPage == this.Pages.AudioEngine)
            return this.getAudioEnginePage();
        else if (this.currentPage == this.Pages.MidiEngine)
            return this.midiEnginePage;
        else
            return <section style="flow: vertical; height: *; vertical-align: middle; text-align: center; color: var(disabled-text)" class="lang-SettingsSelectOption">
            </section>;
    }

    render() {
        return <body styleset={__DIR__ + "settings.css#settings"}>
            <main>
                <ul>
                    <li id="audio-button" class={ this.currentPage == this.Pages.AudioEngine ? "current-page lang-AudioEngine" : "lang-AudioEngine" }></li>
                    <li id="midi-button" class={ this.currentPage == this.Pages.MidiEngine ? "current-page lang-MidiEngine" : "lang-MidiEngine" }></li>
                </ul>

                <section style="flow: vertical; width: *; height: *; overflow-y: auto; background-color: var(back);">
                    { this.getCurrentPage() }
                </section>
            </main>
        </body>;
    }

    ["on click at #audio-button"](event, button) {
        this.componentUpdate({ currentPage: this.Pages.AudioEngine });
    }

    ["on click at #midi-button"](event, button) {
        this.componentUpdate({ currentPage: this.Pages.MidiEngine });
    }

    ["on click at #test-button"](event, button) {
        AudioEngine.start();

        AudioMixer.startTestTone();
        this.patch(this.render());

        this.testTone = true;
        setTimeout(() => {
            AudioMixer.endTestTone();
            AudioEngine.stop();
            this.patch(this.render());
            this.testTone = false;
        }, 1000);
    }

    ["on change at #api-dropdown"](event, dropdown) {
        var option = dropdown.$("option:current");
        var apiNum = parseInt(option.id.substring(4)); // Get the API number ID from the id of the current selected option.
        AudioEngine.changeBackend(apiNum);
        this.patch(this.render());
    }

    ["on change at #output-dropdown"](event, dropdown) {
        var option = dropdown.$("option:current");
        var deviceNum = parseInt(option.className.substring(7)); // Get the Device number ID from the class of the current selected option.

        AudioEngine.outputDevice = deviceNum;
        this.supportedSampleRates = AudioEngine.getSupportedSampleRates();

        this.patch(this.render());
    }

    ["on change at #input-dropdown"](event, dropdown) {
        var option = dropdown.$("option:current");
        var deviceNum = parseInt(option.className.substring(7));

        AudioEngine.inputDevice = deviceNum;
        this.supportedSampleRates = AudioEngine.getSupportedSampleRates();

        this.patch(this.render());
    }

    ["on change at #samplerate-dropdown"](event, dropdown) {
        var option = dropdown.$("option:current");
        var samplerate = parseInt(option.innerText); // Get the sample rate from the text content.

        AudioEngine.sampleRate = samplerate;

        this.patch(this.render());
    }

    ["on change at #buffersize-dropdown"](event, dropdown) {
        var option = dropdown.$("option:current");
        var bufferSize = parseInt(option.id.substring(7));

        AudioEngine.bufferSize = bufferSize;

        this.patch(this.render());
    }
}
