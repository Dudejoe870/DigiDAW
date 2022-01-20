import { MenuBar } from "components/menubar/menubar.js";
import { Timeline } from "components/timeline/timeline.js"

export class Application extends Element {
    render() {
        return <body>
            <MenuBar app={this} />
            <main>
                <Timeline app={this} />
            </main>
        </body>;
    }
}
