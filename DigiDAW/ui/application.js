import { MenuBar } from "menubar/menubar.js";
import { StatusBar } from "statusbar/statusbar.js";

export class Application extends Element {
    render() {
        return <body>
            <MenuBar app={this} />
            <main>
            </main>
            <StatusBar app={this} />
        </body>;
    }
}
