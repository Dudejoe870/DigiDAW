import { MenuBar } from "components/menubar/menubar.js";

export class Application extends Element {
    render() {
        return <body>
            <MenuBar app={this} />
            <main>
            </main>
        </body>;
    }
}
