import { MenuBar } from "components/menubar/menubar.js";
import { StatusBar } from "components/statusbar/statusbar.js";

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
