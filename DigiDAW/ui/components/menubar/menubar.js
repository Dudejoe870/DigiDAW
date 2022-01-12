export class MenuBar extends Element {
    app;

    this(props, kids) {
        this.app = props.app;
    }

    render() {
        return <ul styleset={__DIR__ + "menubar.css#menubar"}>
            <li>File
                <menu>
                    <li>New file</li>
                    <li>Open file</li>
                    <li>Save file</li>
                    <li>Save file as</li>
                </menu>
            </li>
            <li>Edit
                <menu>
                    <li id="settings-button">Settings</li>
                </menu>
            </li>
            <li>Help
                <menu>
                    <li id="about-button">About</li>
                </menu>
            </li>
        </ul>;
    }

    ["on click at #about-button"](event, button) {
        new Window({ url: "../../windows/about/about.htm", type: Window.TOOL_WINDOW, width: 620, height: 512 });
    }

    ["on click at #settings-button"](event, button) {
        new Window({ url: "../../windows/settings/settings.htm", type: Window.FRAME_WINDOW });
    }
}
