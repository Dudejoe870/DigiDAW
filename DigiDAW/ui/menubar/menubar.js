export class MenuBar extends Element {
    app;

    this(props, kids) {
        this.app = props.app;
    }

    render() {
        return <ul styleset={__DIR__ + "menubar.css#menubar"}>
            <li>File
                <menu>
                    <li.command name="new-file">New file</li>
                    <li.command name="open-file">Open file</li>
                    <li.command name="save-file">Save file</li>
                    <li.command name="save-file-as">Save file as</li>
                </menu>
            </li>
            <li>Edit
                <menu>
                    <li.command name="edit-settings">Settings</li>
                </menu>
            </li>
        </ul>;
    }

}
