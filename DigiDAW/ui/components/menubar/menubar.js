export class MenuBar extends Element {
    app;

    this(props, kids) {
        this.app = props.app;
    }

    render() {
        return <ul styleset={__DIR__ + "menubar.css#menubar"}>
            <li><span class="lang-MenuFile"></span>
                <menu>
                    <li class="lang-MenuFileNewFile"></li>
                    <li class="lang-MenuFileOpenFile"></li>
                    <li class="lang-MenuFileSaveFile"></li>
                    <li class="lang-MenuFileSaveFileAs"></li>
                </menu>
            </li>
            <li><span class="lang-MenuEdit"></span>
                <menu>
                    <li id="settings-button" class="lang-MenuEditSettings"></li>
                </menu>
            </li>
            <li><span class="lang-MenuHelp"></span>
                <menu>
                    <li id="about-button" class="lang-MenuHelpAbout"></li>
                </menu>
            </li>
        </ul>;
    }

    ["on click at #about-button"](event, button) {
        new Window({ url: "../../windows/about/about.htm", type: Window.TOOL_WINDOW, width: 620, height: 512, parent: Window.this });
    }

    ["on click at #settings-button"](event, button) {
        new Window({ url: "../../windows/settings/settings.htm", type: Window.FRAME_WINDOW, parent: Window.this });
    }
}
