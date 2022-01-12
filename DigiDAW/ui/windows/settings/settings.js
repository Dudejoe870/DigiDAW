export class Settings extends Element {
    app;

    currentPage;

    supportedAPIs = [];

    Pages = Object.freeze({
        AudioEngine: 0,
        MidiEngine: 1
    });

    this(props, kids) {
        this.app = props.app;
        this.currentPage = -1;
        this.supportedAPIs = AudioEngine.getSupportedAPIs();
    }

    getAPIDropdown() {
        var ret = "";

        for (const api of this.supportedAPIs) {
            ret += `<option id="api-${api}" ${api == AudioEngine.getCurrentAPI() ? "selected" : ""}>${AudioEngine.getAPIDisplayName(api)}</option>`;
        }

        return ret;
    }

    getAudioEnginePage() {
        return <section style="flow: vertical; height: *; width: *; margin: 32px; margin-top: 0; margin-bottom: 0; padding: 0; text-align: center;">
            <h1 class="lang-AudioEngineSettings"></h1>
            <section style="flow: vertical; text-align: center; horizontal-align: center; width: *;">
                <div>
                    <label class="lang-SettingsAPIBackend"></label>
                    <select type="dropdown" id="api-dropdown" state-html={ this.getAPIDropdown() }>
                    </select>
                </div>
                <hr />

                <h2 class="lang-Devices"></h2>

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

    ["on change at #api-dropdown"](event, dropdown) {
        var option = dropdown.$("option:current");
        var apiNum = parseInt(option.id.charAt(option.id.length - 1)); // Get the API number ID from the id of the current selected option.
        AudioEngine.changeBackend(apiNum);
        this.patch(this.render());
    }
}
